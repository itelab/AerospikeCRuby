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

# indexes = client.info_cmd("sindex")
# indexes = indexes.gsub!(/sindex\s/, '').split(";")

# indexes.each do |index|
#   puts "==============================="
#   stats = index.split(":")

#   stats.each do |st|
#     puts st
#   end
# end

puts client.list_indexes