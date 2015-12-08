require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/batch_get.log")

keys = []

i = 0
5000.times do
  bins = {"index" => i, "message" => "message_#{i}"}
  key = AerospikeC::Key.new("test", "batch_get", "batch_get_#{i}")

  client.delete(key)
  client.put(key, bins)

  keys << key
  i += 1
end

recs = client.batch_get(keys)

puts recs

puts "-----------------------------"

recs = client.batch_get(keys, ["index"], with_header: true)

puts recs.inspect