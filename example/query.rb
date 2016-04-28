require "aerospike_c_ruby"
require "logger"

lua_path = File.expand_path(File.join(File.dirname(__FILE__), "lua"))
client = AerospikeC::Client.new("127.0.0.1", 3000, {lua_path: lua_path})
client.logger = Logger.new("log/query.log")

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

#
# need indexes
#
tasks = []
tasks << client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
tasks << client.create_index("test", "query_test", "string_bin", "test_query_test_string_bin_idx", :string)

puts "building indexes...."
tasks.each do |task|
  task.wait_till_completed(50)
end

def build_data(client)
  #
  # build data to operate on
  #
  i = 0
  100.times do
    key = AerospikeC::Key.new("test", "query_test", "query#{i}")
    bins = {
      int_bin: i,
      string_bin: "string#{(i%10) == 0 ? 21 : i }",
      other_bin: i+5,
      other_bin2: [1, 2, "three"]
    }

    client.put(key, bins, ttl: 20)
    i += 1
  end
end

build_data(client)



#
# query
#
puts "\nquery range int_bin 1,5:"
recs = client.query(q_range)
puts recs.inspect

puts "\n-----------------------------\n"
puts "\nquery range int_bin 1,5 with header:"
q_range.with_header!(true)
recs = client.query(q_range)
puts recs.inspect

puts "\nquery eql string_bin string21:"
recs2 = client.query(q_eql)
puts recs2.inspect
puts "\n-----------------------------\n"

#
# udf
#
aggregate_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/aggregate_udf.lua"))

puts "registering aggregate_udf.lua"
task = client.register_udf(aggregate_udf, "aggregate_udf.lua")
task.wait_till_completed(50)
puts "registering aggregate_udf.lua done."

rs = client.execute_udf_on_query(q_range, "aggregate_udf", "mycount")
puts rs.inspect
puts "alias:"
rs = client.aggregate(q_range, "aggregate_udf", "mycount")
puts rs.inspect

5.times do
  rs = client.execute_udf_on_query(q_eql, "aggregate_udf", "other_bin_min", [50])
  puts rs.inspect
end

puts "------------- background_execute_udf_on_query:"
query_task = client.background_execute_udf_on_query(q_range, "aggregate_udf", "mycount")
puts query_task.inspect
query_task.wait_till_completed(50)
puts query_task.done?

puts "alias:"
query_task = client.bg_aggregate(q_range, "aggregate_udf", "mycount")
puts query_task.inspect
query_task.wait_till_completed(50)
puts query_task.done?

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
client.drop_udf("aggregate_udf.lua")

puts GC.count