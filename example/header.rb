require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/header.log")

key = AerospikeC::Key.new("test", "test", "header")
client.delete(key)

bins = {
  "a" => 1,
  "b" => 2,
  "c" => "trzy",
}

ttl = 3

client.put(key, bins, ttl: ttl)
puts client.get_header(key).inspect

(ttl - 1).times do
  sleep 1
  puts client.get_header(key).inspect
end

ttl = 5

puts client.touch(key, ttl: ttl).inspect

(ttl + 1).times do
  sleep 1
  puts client.get_header(key).inspect
end