require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)

#
# eql
#
q_eql = AerospikeC::Query.new("test", "query_test", ["string_bin", "other_bin"])
q_eql.eql!("string_bin", "string21")
puts "-------- query eql obj:"
puts q_eql.inspect

#
# range
#
q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 5, 10)
q_range.order_by!("int_bin", :desc)
puts "-------- query eql obj:"
puts q_range.inspect

client.drop_index("test", "test_query_test_int_bin_idx")
client.drop_index("test", "test_query_test_string_bin_idx")

tasks = []
tasks << client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
tasks << client.create_index("test", "query_test", "string_bin", "test_query_test_string_bin_idx", :string)

puts "building indexes...."
tasks.each do |task|
  task.wait_till_completed
end

i = 0
100.times do
  key = AerospikeC::Key.new("test", "query_test", "query#{i}")
  bins = {
    int_bin: i,
    string_bin: "string#{(i%10) == 0 ? 21 : i }",
    other_bin: 123,
    other_bin2: [1, 2, "three"]
  }

  client.put(key, bins)
  i += 1
end

puts "\nquery range int_bin 1,5:"
recs = client.query(q_range)
puts recs.inspect

puts "\nquery eql string_bin string21:"
recs2 = client.query(q_eql)
puts recs2.inspect

#
# cleanup
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "query_test", "query#{i}")

  client.delete(key)
  i += 1
end

client.drop_index("test", "test_query_test_int_bin_idx")
client.drop_index("test", "test_query_test_string_bin_idx")