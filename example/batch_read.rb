require "aerospike_c_ruby"
require "pp"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new(STDOUT)

keys = []

i = 0
100.times do
  keys << AerospikeC::Key.new("test", "batch_read", "key#{i}")
  key = AerospikeC::Key.new("test", "batch_read", i)
  keys << key

  if i % 2 == 0
    bins = {
      bin1: 1,
      bin2: "bin2"
    }

    client.put(key, bins)
  end

  i += 1
end



pp client.batch_read(keys)

client.batch_get(keys)