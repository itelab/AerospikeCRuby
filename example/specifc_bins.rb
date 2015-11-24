require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/specific_bins.log")

key = AerospikeC::Key.new("test", "test", "specific_bins")
client.delete(key)

bins = {
  "a" => 1,
  "b" => 2,
  "c" => "trzy",
}

client.put(key, bins)

puts client.get(key, ["a", "b"]).inspect