#include "as_predexp_array.h"

void init_as_predexp_array(as_predexp_array *a, int initialSize){
  a->array = (as_predexp_base **) cf_calloc(initialSize, sizeof(as_bin_name));
  a->capacity = 0;
  a->size = initialSize;
}

void insert_as_predexp_array(as_predexp_array *a, as_predexp_base * element) {
  if (a->capacity == a->size) {
    a->size *= 2;
    a->array = (as_predexp_base **) cf_realloc(a->array, a->size * sizeof(as_bin_name));
  }
  a->array[a->capacity++] = element;
}

void free_as_predexp_array(as_predexp_array *a) {
  free(a->array);
  a->array = NULL;
  a->capacity = a->size = 0;
}

void predexp_2_as_predexp(VALUE self, as_predexp_array *a) {
  VALUE predexp_node = rb_iv_get(self, "@predexp");

  if (predexp_node == Qnil) {
    return;
  } else {
    switch(TYPE(predexp_node)) {
      case T_HASH:
        predexp_node_2_as_predexp(a, predexp_node);
        break;
      default:
        raise_parse_error();
        break;
    }
  }
}

void predexp_node_2_as_predexp(as_predexp_array *a, VALUE node) {
  check_hash(node);

  // node = {
  //   bin: bin,
  //   true: true,
  //   filters: {
  //     filter: :eq,
  //     value: 1
  //   },
  //   and: [],
  //   or: []
  // }

  VALUE node_bin = rb_hash_aref(node, bin_sym);

  VALUE node_true = rb_hash_aref(node, predexp_true_sym);
  VALUE node_filters = rb_hash_aref(node, predexp_filters_sym);
  push_2_as_predexp(a, node_bin, node_true, node_filters);

  VALUE node_and = rb_hash_aref(node, predexp_and_sym);
  check_array(node_and);
  int length = rb_ary_len_int(node_and);
  if (length > 0) {
    for(int i = 0; i < length; i++){
      VALUE node_and_obj = rb_ary_entry(node_and, i);
      predexp_node_2_as_predexp(a, node_and_obj);
    }
    insert_as_predexp_array(a, as_predexp_and(length + 1));
  }

  VALUE node_or = rb_hash_aref(node, predexp_or_sym);
  check_array(node_or);
  length = rb_ary_len_int(node_or);
  if (length > 0) {
    for(int i = 0; i < length; i++){
      VALUE node_or_obj = rb_ary_entry(node_or, i);
      predexp_node_2_as_predexp(a, node_or_obj);
    }
    insert_as_predexp_array(a, as_predexp_or(length + 1));
  }
}

void push_2_as_predexp(as_predexp_array *a, VALUE node_bin, VALUE node_true, VALUE node_filters){
  check_hash(node_filters);
  VALUE bin;

  switch(TYPE(node_bin)){
    case T_STRING:
      bin = StringValueCStr(node_bin);
      break;
    case T_SYMBOL:
      if(node_bin != predexp_record_sym) {
        node_bin = rb_funcall(node_bin, rb_intern("to_s"), 0);
        bin      = StringValueCStr(node_bin);
      }
      break;
    default:
      raise_parse_error();
      break;
  }

  VALUE node_filter = rb_hash_aref(node_filters, predexp_filter_sym);
  VALUE node_collection = rb_hash_aref(node_filters, predexp_collection_sym);
  VALUE node_value = rb_hash_aref(node_filters, value_sym);
  VALUE node_value_type = TYPE(node_value);

  if (node_bin == predexp_record_sym){
    VALUE record_filter = rb_hash_aref(node_filters, predexp_record_filter_sym);
    if(record_filter == predexp_record_expiration_time_sym ) {
      insert_as_predexp_array(a, as_predexp_rec_last_update());
    } else if (record_filter == predexp_record_last_update_sym) {
      insert_as_predexp_array(a, as_predexp_rec_void_time());
    } else {
      raise_parse_error();
    }
    node_value = NUM2ULONG(node_value);
    insert_integer_predicate(a, node_filter);

  } else if(node_collection != Qnil) {
    VALUE node_collection_pred = rb_hash_aref(node_filters, predexp_collection_pred_sym);
    VALUE node_collection_var = rb_hash_aref(node_filters, predexp_collection_var_sym);
    if (node_collection_pred == Qnil || node_collection_var == Qnil) {
      raise_parse_error();
    } else if(node_value_type == T_FIXNUM) {
      node_value = NUM2UINT(node_value);
      insert_as_predexp_array(a, as_predexp_integer_var(node_collection_var));
      insert_as_predexp_array(a, as_predexp_integer_value(node_value));
      insert_integer_predicate(a, node_filter);
      insert_collection_predicate(a, bin, node_collection, node_collection_pred, node_collection_var);
    } else if(node_value_type == T_STRING) {
      node_value = StringValueCStr(node_value);
      insert_as_predexp_array(a, as_predexp_string_var(node_collection_var));
      insert_as_predexp_array(a, as_predexp_string_value(node_value));
      if (node_filter == predexp_equal_sym) {
        insert_as_predexp_array(a, as_predexp_string_equal());
      } else if(node_filter == predexp_unequal_sym) {
        insert_as_predexp_array(a, as_predexp_string_unequal());
      } else if(node_filter == predexp_regexp_sym) {
        insert_as_predexp_array(a, as_predexp_string_regex(1));
      } else {
        raise_parse_error();
      }
    } else if(node_value_type == T_DATA) {
      if ( rb_funcall(node_value, rb_intern("is_a?"), 1, rb_aero_GeoJson) == Qtrue ) {
        as_geojson * geo = get_geo_json_struct(node_value);
        char * buffer = (char *) malloc ( strlen(geo->value) + 1 );
        strcpy(buffer, geo->value);
        insert_as_predexp_array(a, as_predexp_geojson_var(node_collection_var));
        insert_as_predexp_array(a, as_predexp_geojson_value(buffer));
        if (node_filter == predexp_within_sym) {
          insert_as_predexp_array(a, as_predexp_geojson_within());
        } else if(node_filter == predexp_contains_sym) {
          insert_as_predexp_array(a, as_predexp_geojson_contains());
        } else {
          raise_parse_error();
        }
      } else {
        raise_parse_error();
      }
    } else {
      raise_parse_error();
    }
    insert_collection_predicate(a, bin, node_collection, node_collection_pred, node_collection_var);

  } else if(node_value_type == T_FIXNUM){
    node_value = NUM2UINT(node_value);
    insert_as_predexp_array(a, as_predexp_integer_bin(bin));
    insert_as_predexp_array(a, as_predexp_integer_value(node_value));
    insert_integer_predicate(a, node_filter);

  } else if(node_value_type == T_STRING){
    node_value = StringValueCStr(node_value);
    insert_as_predexp_array(a, as_predexp_string_bin(bin));
    insert_as_predexp_array(a, as_predexp_string_value(node_value));
    if (node_filter == predexp_equal_sym) {
      insert_as_predexp_array(a, as_predexp_string_equal());
    } else if(node_filter == predexp_unequal_sym) {
      insert_as_predexp_array(a, as_predexp_string_unequal());
    } else if(node_filter == predexp_regexp_sym) {
      insert_as_predexp_array(a, as_predexp_string_regex(1));
    } else {
      raise_parse_error();
    }
  } else if(node_value_type == T_DATA){
    if ( rb_funcall(node_value, rb_intern("is_a?"), 1, rb_aero_GeoJson) == Qtrue ) {
      as_geojson * geo = get_geo_json_struct(node_value);
      char * buffer = (char *) malloc ( strlen(geo->value) + 1 );
      strcpy(buffer, geo->value);
      insert_as_predexp_array(a, as_predexp_geojson_bin(bin));
      insert_as_predexp_array(a, as_predexp_geojson_value(buffer));
      if (node_filter == predexp_within_sym) {
        insert_as_predexp_array(a, as_predexp_geojson_within());
      } else if(node_filter == predexp_contains_sym) {
        insert_as_predexp_array(a, as_predexp_geojson_contains());
      } else {
        raise_parse_error();
      }
    } else {
      raise_parse_error();
    }
  } else {
    raise_parse_error();
  }

  if(node_true == Qfalse) {
    insert_as_predexp_array(a, as_predexp_not(1));
  }
}

void insert_integer_predicate(as_predexp_array *a, VALUE val) {
  if (val == predexp_equal_sym) {
    insert_as_predexp_array(a, as_predexp_integer_equal());
  } else if(val == predexp_unequal_sym) {
    insert_as_predexp_array(a, as_predexp_integer_unequal());
  } else if(val == predexp_greater_sym) {
    insert_as_predexp_array(a, as_predexp_integer_greater());
  } else if(val == predexp_greatereq_sym) {
    insert_as_predexp_array(a, as_predexp_integer_greatereq());
  } else if(val == predexp_less_sym) {
    insert_as_predexp_array(a, as_predexp_integer_less());
  } else if(val == predexp_lesseq_sym) {
    insert_as_predexp_array(a, as_predexp_integer_lesseq());
  } else {
    raise_parse_error();
  }
}

void insert_collection_predicate(as_predexp_array *a, VALUE bin, VALUE collection, VALUE collection_pred, VALUE node_collection_var) {
  if(collection == predexp_array_sym) {
    insert_as_predexp_array(a, as_predexp_list_bin(bin));
    if (collection_pred == predexp_or_sym) {
      insert_as_predexp_array(a, as_predexp_list_iterate_or(node_collection_var));
    } else if (collection_pred == predexp_and_sym) {
      insert_as_predexp_array(a, as_predexp_list_iterate_and(node_collection_var));
    } else {
      raise_parse_error();
    }
  } else if(collection == predexp_map_key_sym) {
    insert_as_predexp_array(a, as_predexp_map_bin(bin));
    if (collection_pred == predexp_or_sym) {
      insert_as_predexp_array(a, as_predexp_mapkey_iterate_or(node_collection_var));
    } else if (collection_pred == predexp_and_sym) {
      insert_as_predexp_array(a, as_predexp_mapkey_iterate_and(node_collection_var));
    } else {
      raise_parse_error();
    }
  } else if(collection == predexp_map_val_sym) {
    insert_as_predexp_array(a, as_predexp_map_bin(bin));
    if (collection_pred == predexp_or_sym) {
      insert_as_predexp_array(a, as_predexp_mapval_iterate_or(node_collection_var));
    } else if (collection_pred == predexp_and_sym) {
      insert_as_predexp_array(a, as_predexp_mapval_iterate_and(node_collection_var));
    } else {
      raise_parse_error();
    }
  } else {
    raise_parse_error();
  }
}

void check_hash(VALUE hash){
  if( TYPE(hash) != T_HASH ) {
    raise_parse_error();
  }
}

void check_array(VALUE array){
  if( TYPE(array) != T_ARRAY ) {
    raise_parse_error();
  }
}

void raise_parse_error(){
  rb_raise(rb_aero_ParseError, "[AerospikeC::PredExp] Invalid parameters");
}
