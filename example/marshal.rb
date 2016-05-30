require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

key = AerospikeC::Key.new("test", "marshal", "key")

x = Marshal.dump(key)

key2 = Marshal.load(x)

puts key2.inspect

puts key == key2