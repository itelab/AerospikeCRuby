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
  "d" => "dddd",
  "e" => "cycki",
}

class E
  def to_s
    "e"
  end
end

client.put(key, bins)

puts client.get(key, [:a, "b", E.new], with_header: true).inspect