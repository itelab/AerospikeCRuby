$LOAD_PATH.unshift File.expand_path('../../lib', __FILE__)
require 'aerospike_c_ruby'

def put_default
  @client.put(@key, @bins)
end

def put_default_ttl(ttl)
  @client.put(@key, @bins, ttl: ttl)
end

def ttl_default
  4294967295
end

def rand_string(len = 8)
  (0...len).map { (65 + rand(26)).chr }.join
end