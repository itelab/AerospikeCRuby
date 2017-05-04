## Usage

The aerospike_c_ruby client package is the main entry point to the client API.

```ruby
require "aerospike_c_ruby"
```

Create client object for conneting:

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)
```

### API Reference

- [AerospikeC module](#aerospike_c)
- [Client class](client.md)
- [Key class](key.md)
- [Record class (deprecated)](record.md)
- [Operation class](operation.md)
- [Query class](query.md)
- [IndexTask class](index_task.md)
- [UdfTask class](udf_task.md)
- [ScanTask class](scan_task.md)
- [QueryTask class](query_task.md)
- [Policy classes](policy.md)
- [Llist class](llist.md)
- [LdtProxy class](ldt_proxy.md)
- [GeoJson class](geo_json.md)
- [PasswordEngine class](password_engine.md)
- [Password class](password.md)
- [PredExp class](predexp.md)
- [PredExpNode class](predexp_node.md)


<a name="aerospike_c"></a>

# AerospikeC module

[Methods](#methods):
  - [#encoding](#encoding)
  - [#encoding=](#encoding=)
  - [#logger](#logger)
  - [#logger=](#logger=)


<!--===============================================================================-->
<hr/>
<!-- encoding -->
<a name="encoding"></a>

### encoding

Get current global encoding

Example:

```ruby
AerospikeC.encoding # => nil
AerospikeC.encoding = 'UTF-8'
AerospikeC.encoding # => 'UTF-8'
```

<!--===============================================================================-->
<hr/>
<!-- encoding= -->
<a name="encoding="></a>

### encoding=(value)

Set global encoding, this will be used to encode all string returned from aerospike

Return value

Example from examples/utf8.rb:

```ruby
bins = {
  "title" => "Tytuł",
  "content" => "Jaskółka",
}
client.put(key, bins)

puts AerospikeC.encoding.inspect # => nil
puts client.get(key) # => {"title"=>"Tytu\xC5\x82", "content"=>"Jask\xC3\xB3\xC5\x82ka"}

puts "------------------"
AerospikeC.encoding = 'UTF-8'

puts AerospikeC.encoding.inspect # => "UTF-8"

puts client.get(key) # {"title"=>"Tytuł", "content"=>"Jaskółka"}
```

<!--===============================================================================-->
<hr/>
<!-- logger= -->
<a name="logger="></a>

### logger=(log)

Setting global logger. AerospikeCRuby supports only global logger. This may change in the future into per client ones


<!--===============================================================================-->
<hr/>
<!-- logger -->
<a name="logger"></a>

### logger

Getting global logger. AerospikeCRuby supports only global logger. This may change in the future into per client ones
