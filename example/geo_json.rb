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
}

client.put(key, bins)
rec = client.get(key)

puts rec
puts rec["geo"].json
puts rec["geo_polygon"].json