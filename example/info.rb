#
# execute info command
# http://www.aerospike.com/docs/reference/info
#
require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

puts "----------------"
puts client.info_cmd("node")
puts "----------------"
puts client.info_cmd("logs")
puts "----------------"
puts client.info_cmd("bins")
puts "----------------"
puts client.list_indexes
puts "----------------"
puts client.statistics
puts "----------------"
puts client.namespaces