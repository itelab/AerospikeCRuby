require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new(STDOUT)

key = AerospikeC::Key.new("test", "test", "utf8")

bins = {
  "title" => "Tytuł",
  "content" => "Jaskółka",
}
client.put(key, bins)

puts AerospikeC.encoding.inspect
puts client.get(key)

puts "------------------"
AerospikeC.encoding = 'UTF-8'

puts AerospikeC.encoding.inspect

puts client.get(key)