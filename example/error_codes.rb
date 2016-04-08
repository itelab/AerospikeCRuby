require "aerospike_c_ruby"
require "pp"

AerospikeC::ErrorCodes.constants.each do |c|
  err = AerospikeC::ErrorCodes.new(AerospikeC::ErrorCodes.const_get(c))
  puts "  "
  pp err
end

raise AerospikeC::ErrorCodes.new(AerospikeC::ErrorCodes::AS_PROTO_RESULT_FAIL_BATCH_MAX_REQUESTS)