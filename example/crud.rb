require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/crud.log")

key = AerospikeC::Key.new("test", "test", "crud")
client.delete(key)

puts "\n--- key:"
puts key.inspect

bins = {
  :x => 15,
  "y" => "abcd",
  "current_lat" => 12132435435,
  "current_lng" => 234324435345,
}

puts "\n--- bins:"
puts bins.inspect

client.put(key, bins)

puts "\n--- get key:"
puts client.get(key).inspect

bins["y"]   = ["A", "B"]
bins["x"]   = nil

puts "\n--- bins:"
puts bins.inspect

client.put(key, bins)

puts "\n--- get key updated:"
puts client.get(key).inspect

puts "\n--- key exists?"
puts client.exists?(key)

client.delete(key)
puts "\n--- get key deleted:"
puts client.get(key).inspect

puts "\n--- key exists?"
puts client.exists?(key)