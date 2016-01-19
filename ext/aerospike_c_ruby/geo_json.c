#include <aerospike_c_ruby.h>

VALUE GeoJson;

#define NEW_JSON(json) rb_funcall(self, rb_intern("new"), 1, (json));
#define geo_json_get_coordinates(point) rb_funcall((point), rb_intern("coordinates"), 0)
#define geo_json_get_json() rb_funcall(self, rb_intern("json"), 0)
#define geo_json_get_type() rb_funcall(self, rb_intern("type"), 0)
;

// ----------------------------------------------------------------------------------
//
// free memory method
//
static void geo_json_deallocate(as_geojson * key) {
  as_geojson_destroy(key);
  xfree(key);
}

static VALUE geo_json_allocate(VALUE self) {
  as_geojson * geo = (as_geojson *) ruby_xmalloc ( sizeof(as_geojson) );
  if (! geo) rb_raise(MemoryError, "[AerospikeC::GeoJson][initialize] Error while allocating memory for aerospike geo_json");

  return Data_Wrap_Struct(self, NULL, geo_json_deallocate, geo);
}

// ----------------------------------------------------------------------------------
//
// def initialize
//
static void geo_json_initialize(VALUE self, VALUE json) {
  VALUE rb_json_str;

  if ( TYPE(json) == T_HASH ) {
    rb_json_str = rb_funcall(json, rb_intern("to_json"), 0);
  }
  else {
    rb_json_str = value_to_s(json);
  }

  as_geojson * geo;
  Data_Get_Struct(self, as_geojson, geo);

  as_geojson_init(geo, StringValueCStr(rb_json_str), false);
}

// ----------------------------------------------------------------------------------
//
// def json
//
static VALUE geo_json_json(VALUE self) {
  as_geojson * geo;
  Data_Get_Struct(self, as_geojson, geo);

  char * json = as_geojson_getorelse(geo, DEFAULT_GEO_JSON_ELSE);

  return rb_str_new2(json);
}

// ----------------------------------------------------------------------------------
//
// def coordinates
//
static VALUE geo_json_coordinates(VALUE self) {
  VALUE m_JSON = rb_mJSON();

  VALUE self_json = geo_json_get_json();
  VALUE json_hash = rb_funcall(m_JSON, rb_intern("parse"), 1, self_json);

  return rb_hash_aref(json_hash, RB_COORDINATES_STR);
}

// ----------------------------------------------------------------------------------
//
// def inspect
//
static VALUE geo_json_inspect(VALUE self) {
  VALUE m_JSON = rb_mJSON();

  VALUE json = geo_json_get_json();
  VALUE json_hash = rb_funcall(m_JSON, rb_intern("parse"), 1, json);
  VALUE json_str = rb_inspect(json_hash);

  VALUE str = rb_str_new2("#<AerospikeC::GeoJson ");
  rb_funcall(str, rb_intern("<<"), 1, json_str);
  rb_funcall(str, rb_intern("<<"), 1, rb_str_new2(">"));

  return str;
}

// ----------------------------------------------------------------------------------
//
// def type
//
static VALUE geo_json_type(VALUE self) {
  VALUE m_JSON = rb_mJSON();

  VALUE json = geo_json_get_json();
  VALUE json_hash = rb_funcall(m_JSON, rb_intern("parse"), 1, json);
  VALUE type = rb_hash_aref(json_hash, rb_str_new2("type"));

  return type;
}

// ----------------------------------------------------------------------------------
//
// def polygon?
//
static VALUE geo_json_is_polygon(VALUE self) {
  return rb_funcall(geo_json_get_type(), rb_intern("=="), 1, RB_POLYGON_STR);
}

// ----------------------------------------------------------------------------------
//
// def point?
//
static VALUE geo_json_is_point(VALUE self) {
  return rb_funcall(geo_json_get_type(), rb_intern("=="), 1, RB_POINT_STR);
}

// ----------------------------------------------------------------------------------
//
// def circle?
//
static VALUE geo_json_is_circle(VALUE self) {
  return rb_funcall(geo_json_get_type(), rb_intern("=="), 1, RB_CIRCLE_STR);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.point
//
static VALUE geo_json_point(VALUE self, VALUE x, VALUE y) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_POINT_STR);

  VALUE point_ary = rb_ary_new();
  rb_ary_push(point_ary, x);
  rb_ary_push(point_ary, y);

  rb_hash_aset(json, RB_COORDINATES_STR, point_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.polygon
//
static VALUE geo_json_polygon(VALUE self, VALUE cords) {
  if ( TYPE(cords) != T_ARRAY )
    rb_raise(OptionError, "[AerospikeC::GeoJson][polygon] cords must be an Array");

  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_POLYGON_STR);

  VALUE point_ary = rb_ary_new();
  VALUE polygon_point_ary = rb_ary_new();

  rb_foreach_ary_int(cords) {
    VALUE point = rb_ary_entry(cords, i);

    if ( TYPE(point) == T_ARRAY  ) {
      rb_ary_push(polygon_point_ary, point);
    }
    else if ( TYPE(point) == T_DATA ) {
      if ( rb_funcall(point, rb_intern("is_a?"), 1, GeoJson) != Qtrue )
        rb_raise(OptionError, "[AerospikeC::GeoJson][polygon] point in Polygon must be Array or AerospikeC::GeoJson object");

      VALUE point_cords = geo_json_get_coordinates(point);

      VALUE point_point_ary = rb_ary_new();
      rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 0));
      rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 1));

      rb_ary_push(polygon_point_ary, point_point_ary);
    }
  }

  rb_ary_push(point_ary, polygon_point_ary);
  rb_hash_aset(json, RB_COORDINATES_STR, point_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.polygon_array
//
static VALUE geo_json_polygon_array(VALUE self, VALUE cords) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_POLYGON_STR);

  VALUE point_ary = rb_ary_new();
  VALUE polygon_point_ary = rb_ary_new();

  rb_foreach_ary_int(cords) {
    VALUE point = rb_ary_entry(cords, i);
    rb_ary_push(polygon_point_ary, point);
  }

  rb_ary_push(point_ary, polygon_point_ary);
  rb_hash_aset(json, RB_COORDINATES_STR, point_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.polygon_obj
//
static VALUE geo_json_polygon_obj(VALUE self, VALUE cords) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_POLYGON_STR);

  VALUE point_ary = rb_ary_new();
  VALUE polygon_point_ary = rb_ary_new();

  rb_foreach_ary_int(cords) {
    VALUE point = rb_ary_entry(cords, i);
    VALUE point_cords = geo_json_get_coordinates(point);

    VALUE point_point_ary = rb_ary_new();
    rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 0));
    rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 1));

    rb_ary_push(polygon_point_ary, point_point_ary);
  }

  rb_ary_push(point_ary, polygon_point_ary);
  rb_hash_aset(json, RB_COORDINATES_STR, point_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.circle
//
static VALUE geo_json_circle(VALUE self, VALUE point, VALUE radius) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_CIRCLE_STR);

  VALUE circle_ary = rb_ary_new();

  if ( TYPE(point) == T_ARRAY  ) {
    rb_ary_push(circle_ary, point);
  }
  else if ( TYPE(point) == T_DATA ) {
    if ( rb_funcall(point, rb_intern("is_a?"), 1, GeoJson) != Qtrue )
      rb_raise(OptionError, "[AerospikeC::GeoJson][circle] point must be Array or AerospikeC::GeoJson object");

    VALUE point_cords = geo_json_get_coordinates(point);

    VALUE point_point_ary = rb_ary_new();
    rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 0));
    rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 1));

    rb_ary_push(circle_ary, point_point_ary);
  }

  rb_ary_push(circle_ary, radius);
  rb_hash_aset(json, RB_COORDINATES_STR, circle_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.circle_array
//
static VALUE geo_json_circle_array(VALUE self, VALUE point, VALUE radius) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_CIRCLE_STR);

  VALUE circle_ary = rb_ary_new();

  rb_ary_push(circle_ary, point);
  rb_ary_push(circle_ary, radius);
  rb_hash_aset(json, RB_COORDINATES_STR, circle_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.circle_obj
//
static VALUE geo_json_circle_obj(VALUE self, VALUE point, VALUE radius) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_CIRCLE_STR);

  VALUE circle_ary = rb_ary_new();
  VALUE point_cords = geo_json_get_coordinates(point);

  VALUE point_point_ary = rb_ary_new();
  rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 0));
  rb_ary_push(point_point_ary, rb_ary_entry(point_cords, 1));

  rb_ary_push(circle_ary, radius);
  rb_hash_aset(json, RB_COORDINATES_STR, circle_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// def GeoJson.circle_point
//
static VALUE geo_json_circle_point(VALUE self, VALUE lng, VALUE lat, VALUE radius) {
  VALUE json = rb_hash_new();
  rb_hash_aset(json, rb_str_new2("type"), RB_CIRCLE_STR);

  VALUE circle_ary = rb_ary_new();

  VALUE point_point_ary = rb_ary_new();
  rb_ary_push(point_point_ary, lng);
  rb_ary_push(point_point_ary, lat);

  rb_ary_push(circle_ary, radius);
  rb_hash_aset(json, RB_COORDINATES_STR, circle_ary);

  return NEW_JSON(json);
}

// ----------------------------------------------------------------------------------
//
// Init
//
void init_aerospike_c_geo_json(VALUE AerospikeC) {
  //
  // class AerospikeC::GeoJson < Object
  //
  GeoJson = rb_define_class_under(AerospikeC, "GeoJson", rb_cObject);
  rb_define_alloc_func(GeoJson, geo_json_allocate);

  //
  // methods
  //
  rb_define_method(GeoJson, "initialize", RB_FN_ANY()geo_json_initialize, 1);
  rb_define_method(GeoJson, "json", RB_FN_ANY()geo_json_json, 0);
  rb_define_method(GeoJson, "coordinates", RB_FN_ANY()geo_json_coordinates, 0);
  rb_define_method(GeoJson, "inspect", RB_FN_ANY()geo_json_inspect, 0);
  rb_define_method(GeoJson, "type", RB_FN_ANY()geo_json_type, 0);
  rb_define_method(GeoJson, "point?", RB_FN_ANY()geo_json_is_point, 0);
  rb_define_method(GeoJson, "polygon?", RB_FN_ANY()geo_json_is_polygon, 0);
  rb_define_method(GeoJson, "circle?", RB_FN_ANY()geo_json_is_circle, 0);

  //
  // class methods
  //
  rb_define_singleton_method(GeoJson, "point", RB_FN_ANY()geo_json_point, 2);
  rb_define_singleton_method(GeoJson, "polygon", RB_FN_ANY()geo_json_polygon, 1);
  rb_define_singleton_method(GeoJson, "polygon_array", RB_FN_ANY()geo_json_polygon_array, 1);
  rb_define_singleton_method(GeoJson, "polygon_obj", RB_FN_ANY()geo_json_polygon_obj, 1);
  rb_define_singleton_method(GeoJson, "circle", RB_FN_ANY()geo_json_circle, 2);
  rb_define_singleton_method(GeoJson, "circle_array", RB_FN_ANY()geo_json_circle_array, 2);
  rb_define_singleton_method(GeoJson, "circle_obj", RB_FN_ANY()geo_json_circle_obj, 2);
  rb_define_singleton_method(GeoJson, "circle_point", RB_FN_ANY()geo_json_circle_point, 3);
}