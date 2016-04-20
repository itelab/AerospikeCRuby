require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new(STDOUT)

class Someobj
  def initialize
    @a = 12.4325
    @b = [345, [1, 5, {as: "aerospike"}]]
  end
end

class Testobj
  def initialize
    @a = 1
    @b = "test"
    @c = [1, 2, "test"]
    @d = { a: 1, b: "test"}
    @obj = Someobj.new
  end
end

bins = {
  a: Testobj.new,
  b: [1, "test", Someobj.new],
  c: {key: Testobj.new}
}

x = Marshal.dump(Testobj.new)

key = AerospikeC::Key.new("test", "test", "bytess")

client.put(key, bins)

puts client.get(key).inspect