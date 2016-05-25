require "aerospike_c_ruby"
require "pp"

client = AerospikeC::Client.new("127.0.0.1", 3000)

keys = []

i = 0
10.times do
  keys << AerospikeC::Key.new("test", "batch_exists", "key#{i}")
  key = AerospikeC::Key.new("test", "batch_exists", i)
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



pp client.batch_exists(keys)