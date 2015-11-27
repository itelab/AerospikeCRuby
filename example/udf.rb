require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

hello_world_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/hello_world.lua"))
# absolute path
puts hello_world_udf.inspect

task = client.register_udf(hello_world_udf, "hello.lua")

puts task.inspect

task.wait_till_completed

puts task.inspect