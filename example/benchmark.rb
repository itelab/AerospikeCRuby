require "aerospike_c_ruby"
require "benchmark"

client = AerospikeC::Client.new("127.0.0.1", 3000)

keys = []

bins = {
  "message" => "message"
}

bin = AerospikeC::Record.new(bins)

i = 0
100000.times do
  keys << AerospikeC::Key.new("test", "test", "key-#{i}")
  i += 1
end

all_time = Benchmark.realtime do
  time = Benchmark.realtime do
    keys.each do |key|
      client.put(key, bin)
    end
  end

  puts "#{(time*1000).round(2)}ms write"

  time = Benchmark.realtime do
    keys.each do |key|
      client.get(key)
    end
  end

  puts "#{(time*1000).round(2)}ms read"

  time = Benchmark.realtime do
    client.batch_get(keys[0..4999])
  end

  puts "#{(time*1000).round(2)}ms batch read"

  time = Benchmark.realtime do
    keys.each do |key|
      client.delete(key)
    end
  end

  puts "#{(time*1000).round(2)}ms delete"
end

puts "all time: #{(all_time*1000).round(2)}ms"