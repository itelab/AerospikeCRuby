# LdtProxy Class

LdtProxy is a class that represents Large Data Types when getting record with [AerospikeC::Client get method](client.md#get)

With a new LdtProxy object, you can use any of the methods specified below:

[Methods](#methods):
  - [#llist](#llist)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- llist -->
<a name="llist"></a>

### llist(options = {})

Get [Aerospike::Llist](llist.md) object from this proxy ([more information](llist.md#workaround_note))

Parameters:

- `options`:
  - `:module` - configurator udf module name: http://www.aerospike.com/docs/guide/ldt_advanced.html

Example:

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)
key    = AerospikeC::Key.new("test", "test", "llist")

bin_name = "some_bin_name"
llist    = client.llist(key, bin_name)

llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"c", "key"=>2}]

bins = client.get(key)
# => {"LDTCONTROLBIN"=>nil, "_rblliststat_"=>["some_bin_name"], "some_bin_name"=>#<AerospikeC::LdtProxy:0x0000000192d848 @bin_name="some_bin_name", @client=#<AerospikeC::Client:0x0000000193b8a8 @host="127.0.0.1", @port=3000, @last_scan_id=nil, @last_query_id=nil, @ldt_proxy=true, @options={}>, @key=#<AerospikeC::Key:0x0000000193b588 @namespace="test", @set="test", @key="llist">>}

ldt_proxy = bins["some_bin_name"]


llist = ldt_proxy.llist
llist.update({"key" => 2, "text" => "z"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"z", "key"=>2}]
```