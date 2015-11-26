require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)
key = AerospikeC::Key.new("test", "test", "operate")

client.delete(key)

bins = {
  "int" => 1,
  "string" => "b"
}

ops = AerospikeC::Operation.new # eql to client.operation

ops.increment!("int", 1).append!("string", "c").prepend!("string", "a")
ops.write!("new_bin_int", 10).write!("new_bin_str", "nowy")

ops.read!("int")
ops.read!("string")
ops.read!("new_bin_int")
ops.read!("new_bin_str")

puts ops.inspect

client.put(key, bins)
puts client.get(key).inspect

rec = client.operate(key, ops)
puts rec.inspect