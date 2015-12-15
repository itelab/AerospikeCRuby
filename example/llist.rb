require "aerospike_c_ruby"

client = AerospikeC::Client.new("127.0.0.1", 3000)
key    = AerospikeC::Key.new("test", "test", "llist")
client.delete(key)

llist_conf_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/llist_conf_udf.lua"))

task = client.register_udf(llist_conf_udf, "llist_conf_udf.lua")
task.wait_till_completed


bin_name = "some_bin_name"
llist = client.llist(key, bin_name, :module => "llist_conf_udf")

###################################################################
#
# add
#
puts "-------------------------------"
puts "-> llist inspect:"
puts llist.inspect
puts "-------------------------------"
puts "-> add:"
puts llist.add({"key" => 0, "text" => "a"})
puts llist.add({"key" => 1, "text" => "b"})
puts llist.add({"key" => 2, "text" => "c"})
puts llist.scan.inspect

###################################################################
#
# delete
#
puts "-------------------------------"
puts "-> delete:"
puts llist.delete(1)
puts llist.scan.inspect

###################################################################
#
# add_all
#
ary = [
  {"key" => 3, "int" => 1},
  {"key" => 4, "int" => 2}
]

puts "-------------------------------"
puts "-> add_all:"
puts llist.add_all(ary)
puts llist.scan.inspect


###################################################################
#
# find
#
puts "-------------------------------"
puts "-> find:"
puts llist.find(2).inspect
puts "-------------------------------"
puts "-> find, deleted value:"
puts llist.find(1).inspect


###################################################################
#
# find_first
#
puts "-------------------------------"
puts "-> find_first:"
puts llist.find_first(3).inspect

###################################################################
#
# first
#
puts "-------------------------------"
puts "-> first:"
puts llist.first.inspect


###################################################################
#
# find_last
#
puts "-------------------------------"
puts "-> find_last:"
puts llist.find_last(3).inspect

###################################################################
#
# last
#
puts "-------------------------------"
puts "-> last:"
puts llist.last.inspect

###################################################################
#
# find_from
#
puts "-------------------------------"
puts "-> find_from:"
puts llist.find_from(2, 2).inspect

###################################################################
#
# size
#
puts "-------------------------------"
puts "-> size:"
puts llist.size.inspect

###################################################################
#
# capacity=
#
# puts "-------------------------------"
# puts "-> capacity=:"
# puts llist.capacity = 5000


###################################################################
#
# capacity
#
# puts "-------------------------------"
# puts "-> capacity:"
# puts llist.capacity.inspect

###################################################################
#
# update
#
puts "-------------------------------"
puts "-> update:"
puts llist.update({"key" => 3, "string" => "string"}).inspect
puts llist.scan.inspect

###################################################################
#
# update_all
#

ary = [
  {"key" => 3, "float" => 1.5},
  {"key" => 4, "float" => 3.14}
]


puts "-------------------------------"
puts "-> update_all:"
puts llist.update_all(ary).inspect
puts llist.scan.inspect

###################################################################
#
# filter
#
puts "-------------------------------"
puts "-> filter:"
puts llist.filter("my_filter_func", [4]).inspect
puts llist.scan.inspect

###################################################################
#
# cleanup
#
puts "-------------------------------"
puts "-> bug?"
puts client.get(key)
client.delete(key)

