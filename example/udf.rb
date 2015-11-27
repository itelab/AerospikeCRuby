require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/udf.log")

hello_world_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/hello_world.lua"))
math_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/math.lua"))
# absolute path
puts hello_world_udf.inspect

task = client.register_udf(hello_world_udf, "hello.lua")

puts task.inspect

task.wait_till_completed

puts task.inspect
puts "-------- after add:"
puts client.list_udf

client.drop_udf("hello.lua")
puts "-------- after drop:"
puts client.list_udf

puts "\n----------------------------"
key = AerospikeC::Key.new("test", "test", "udf_example")

bins = {
  "count" => 0
}

client.delete(key)
client.put(key, bins)
puts client.get(key)

puts "adding math_udf.lua"
task = client.register_udf(math_udf, "math_udf.lua")
task.wait_till_completed

puts "\n----------------------------"
puts client.list_udf
puts "\n----------------------------"

puts "math pi:"
puts client.execute_udf(key, "math_udf", "pi").inspect

puts "math set_count:"
puts client.execute_udf(key, "math_udf", "set_count", [35]).inspect
puts "\n----------------------------"
puts client.get(key)
puts "\n----------------------------"


puts "math set_bin_val:"
puts client.execute_udf(key, "math_udf", "set_bin_val", ["count2", 15, "return_val"]).inspect
puts "\n----------------------------"
puts client.get(key)


client.drop_udf("math_udf.lua")