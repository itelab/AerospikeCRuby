require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

key1 = AerospikeC::Key.new("test", "test2", "key1")
key2 = AerospikeC::Key.new("test", "test2", "key2")
key3 = AerospikeC::Key.new("test", "test2", "key2")

puts key1 == key2
puts key1 == key1
puts key2 == key3


hash = {
  key1 => true,
  key2 => false
}

hash[key3] = true

puts hash

puts key1.digest.inspect

puts key1.hash
puts key3.hash
puts key2.hash