require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/geo_json.log")

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

puts "\n------------------"

polygon_points_array = [
  [1, 2],
  [3, 5],
  [10, 11],
  [1, 2]
]

polygon_points_obj = [
  AerospikeC::GeoJson.point(10, -10),
  AerospikeC::GeoJson.point(10, 10),
  AerospikeC::GeoJson.point(-10, 10),
  AerospikeC::GeoJson.point(-10, -10),
  AerospikeC::GeoJson.point(10, -10)
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




#
#
# query
#
#
puts "\n------------------"
client.drop_index("test", "test_geo_json_geo_bin_idx")

polygon = AerospikeC::GeoJson.polygon_obj(polygon_points_obj)

circle = AerospikeC::GeoJson.circle([0, 0], 100 * 1000)
puts "\n------------------ circle"
puts circle.inspect

# build query
q_geo = AerospikeC::Query.new("test", "geo_json")
q_geo.geo_within!("geo_bin", circle)

puts q_geo.inspect
puts "\n------------------"

# create index
puts "creating index"
task = client.create_index("test", "geo_json", "geo_bin", "test_geo_json_geo_bin_idx", :geo_sphere)
task.wait_till_completed(50)

#
# build data to operate on
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "geo_json", "query#{i}")
  cord = (i * 0.1).round(5)
  bins = {
    geo_bin: AerospikeC::GeoJson.point(cord, cord)
  }

  client.put(key, bins)
  i += 1
end

# puts "\n------------------ scan:"
# puts client.scan("test", "geo_json")

require "benchmark"

time = Benchmark.realtime do
  puts "\n------------------ query: "
  recs = client.query(q_geo)
  puts recs
end
puts "#{(time*1000).round(2)}ms query"



#
# cleanup
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "geo_json", "query#{i}")

  client.delete(key)
  i += 1
end

client.drop_index("test", "test_geo_json_geo_bin_idx")