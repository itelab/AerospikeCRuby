require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

key = AerospikeC::Key.new("test", "test", "geo_json")

client.delete(key)

polygon_points = [
  AerospikeC::GeoJson.point(1, 2),
  [2, 3],
  AerospikeC::GeoJson.point(4, 6),
  AerospikeC::GeoJson.point(1, 2),
]

bins = {
  geo: AerospikeC::GeoJson.point(1, 2),
  geo_polygon: AerospikeC::GeoJson.polygon(polygon_points),
  geo_circle: AerospikeC::GeoJson.circle([1, 5], 5),
}
puts "------------------"
client.put(key, bins)
rec = client.get(key)

puts rec
puts rec["geo"].json
puts rec["geo_polygon"].json
puts rec["geo_circle"].json

puts "------------------"

polygon_points_array = [
  [1, 2],
  [3, 5],
  [10, 11],
  [1, 2]
]

polygon_points_obj = [
  AerospikeC::GeoJson.point(1, 2),
  AerospikeC::GeoJson.point(3, 5),
  AerospikeC::GeoJson.point(10, 11),
  AerospikeC::GeoJson.point(1, 2)
]

key2 = AerospikeC::Key.new("test", "test", "geo_json2")

bins2 = {
  polygon_array: AerospikeC::GeoJson.polygon_array(polygon_points_array),
  polygon_obj: AerospikeC::GeoJson.polygon_obj(polygon_points_obj)
}

client.delete(key2)
client.put(key2, bins2)

rec2 = client.get(key2)
puts rec2
puts "------------------"