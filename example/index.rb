require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)
tasks = []
tasks << client.create_index("test", "test", "test_bin", "test_test_test_bin_idx", :numeric)
tasks << client.create_index("test", "test", "test_bin2", "test_test_test_bin2_idx", :string)

tasks.each do |task|
  puts task.inspect
end

tasks.each do |task|
  task.wait_till_completed
end

#
# check with aql -> show indexes
#
tasks.each do |task|
  puts task.inspect
end


# uncomment to drop

client.drop_index("test", "test_test_test_bin_idx")
client.drop_index("test", "test_test_test_bin2_idx")