require "aerospike_c_ruby"

include AerospikeC

client = Client.new("127.0.0.1", 3000)

key = Key.new("test", "test", "policy_test")

bins = {
  "bin1" => 1,
  "bin2" => "two"
}

policy = Policy.new(:write, exists: Policy::AS_POLICY_EXISTS_UPDATE)

client.put(key, bins, {policy: policy})
puts client.get(key).inspect
client.delete(key)

policy = WritePolicy.new(exists: Policy::AS_POLICY_EXISTS_UPDATE)

client.put(key, bins, {policy: policy})
puts client.get(key).inspect
client.delete(key)

policy = WritePolicy.new(exists: Policy::AS_POLICY_EXISTS_CREATE_OR_REPLACE)

client.put(key, bins, {policy: policy})
puts client.get(key).inspect
client.delete(key)

policy = ReadPolicy.new(timeout: 1)

client.put(key, bins)
puts client.get(key, [], policy: policy).inspect
client.delete(key)


policy = RemovePolicy.new(gen: Policy::AS_POLICY_GEN_GT, generation: -1)

client.put(key, bins)
puts client.get(key).inspect
client.delete(key, policy: policy)