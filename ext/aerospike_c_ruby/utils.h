#ifndef AEROSPIKE_C_RUBY_UTILS_H
#define AEROSPIKE_C_RUBY_UTILS_H


void start_timing(struct timeval * tm);

aerospike *  get_client_struct(VALUE client);
as_key *     get_key_struct(VALUE key);
as_record *  get_record_struct(VALUE rec);
as_geojson * get_geo_json_struct(VALUE rb_geo);

void raise_as_err(as_error err);

VALUE record2hash(as_record * rec);
void  hash2record(VALUE hash, as_record * rec);

as_arraylist * array2as_list(VALUE ary);
VALUE          as_list2array(as_arraylist * list);

char ** rb_array2inputArray(VALUE ary);
void    inputArray_destroy(char ** inputArray);

char ** rb_array2bin_names(VALUE ary);
void    bin_names_destroy(char ** bin_names, long len);

VALUE  value_to_s(VALUE val);
char * value_to_s_cstr(VALUE val);
char * val_inspect(VALUE val);

VALUE bool2rb_bool(bool val);
bool  rb_bool2bool(VALUE val);

VALUE    as_val2rb_val(as_val * value);
as_val * rb_val2as_val(VALUE value);

void as_val_free(as_val * value);

const char * as_val_type_as_str(as_val * val);
const char * rb_val_type_as_str(VALUE value);

as_query * query_obj2as_query(VALUE query_obj);

as_hashmap * hash2as_hashmap(VALUE hash);
VALUE        as_hashmap2hash(as_hashmap * map);

void * rb_policy2as_policy(VALUE rb_policy);
void * get_policy(VALUE options);

VALUE enable_rb_GC();
VALUE disable_rb_GC();

VALUE as_geojson_2_val(as_geojson * geo);

as_integer * rb_copy_as_integer(as_integer * value);
as_integer * rb_copy_as_integer_from_val(as_val * value);
as_double * rb_copy_as_double(as_double * value);
as_double * rb_copy_as_double_from_val(as_val * value);
as_string * rb_copy_as_string(as_string * value);
as_string * rb_copy_as_string_from_val(as_val * value);
as_geojson * rb_copy_as_geojson(as_geojson * value);
as_geojson * rb_copy_as_geojson_from_val(as_val * value);
as_arraylist * rb_copy_as_arraylist(as_arraylist * value);
as_arraylist * rb_copy_as_arraylist_from_val(as_val * value);
as_hashmap * rb_copy_as_hashmap(as_hashmap * value);
as_hashmap * rb_copy_as_hashmap_from_val(as_val * value);
as_val * rb_copy_as_val(as_val * value);

as_record * rb_copy_as_record(as_record * record);

as_operations * rb_operations2as_operations(VALUE operations);
as_bytes * rb_obj_to_as_bytes(VALUE obj);
VALUE as_bytes_to_rb_obj(as_bytes * bytes);

VALUE check_and_force_encoding(VALUE str);


#endif // AEROSPIKE_C_RUBY_UTILS_H