require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/ttl.log")

key = AerospikeC::Key.new("test", "test", "ttl")
client.delete(key)

bins = {
  "a" => 1,
  "b" => 2,
  "c" => "three",
}

ttl = 2

client.put(key, bins, ttl: ttl)
puts client.get(key).inspect
puts "ttl: #{ttl}"

sleep ttl + 1
puts "after ttl:"
puts client.get(key).inspect