require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/scan.log")

i = 0
10.times do
  key = AerospikeC::Key.new("test", "scan_test", "scan_test-#{i}")
  bins = {"x" => i, "y" => "two#{i}"}

  client.delete(key)
  client.put(key, bins)

  i += 1
end

puts client.scan("test", "scan_test").inspect
puts "\n----------------------------"

scan_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/scan.lua"))

task = client.register_udf(scan_udf, "scan_udf.lua")
task.wait_till_completed


puts client.execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56]).inspect

i = 0
10.times do
  key = AerospikeC::Key.new("test", "scan_test", "scan_test-#{i}")
  puts client.get(key).inspect

  i += 1
end

puts "\n----------------------------"

scan_task = client.background_execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56])
puts scan_task.inspect

puts scan_task.check_status
puts scan_task.inspect
puts "\n----------------------------"
puts scan_task.completed?

client.drop_udf("scan_udf.lua")