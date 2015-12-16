# Llist Class

Represents Aerospike large list in cluster. Note that once initialized llist cannot be changed, becaouse of aerospike-c-client as_ldt wrapping into ruby. This behaviuor may change in the future.
Aerospike reference:
- http://www.aerospike.com/docs/architecture/ldt.html
- http://www.aerospike.com/docs/guide/ldt.html

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)
key    = AerospikeC::Key.new("test", "test", "llist")

bin_name = "some_bin_name"
llist    = client.llist(key, bin_name)

llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"c", "key"=>2}]
```
<hr />
<a name="workaround_note"></a>

### Important note

Because of strange behavior of C client returning AS_NIL when record have a ldt value (https://discuss.aerospike.com/t/parsing-record-with-ldt/2264/2), aerospike_c_ruby has workaround for getting keys with llist. Lets continue example:

```ruby
bins = client.get(key)
# => {"LDTCONTROLBIN"=>nil, "_rblliststat_"=>["some_bin_name"], "some_bin_name"=>#<AerospikeC::LdtProxy:0x0000000192d848 @bin_name="some_bin_name", @client=#<AerospikeC::Client:0x0000000193b8a8 @host="127.0.0.1", @port=3000, @last_scan_id=nil, @last_query_id=nil, @ldt_proxy=true, @options={}>, @key=#<AerospikeC::Key:0x0000000193b588 @namespace="test", @set="test", @key="llist">>}
```

Adding large list also adds `_rblliststat_` bin with array of bin names, which have llist under it. This is neccessary for parsing these names to [AerospikeC::LdtProxy](ldt_proxy.md) objects(because they are not `Llist` already you can pass your own options)

```ruby
ldt_proxy = bins["some_bin_name"]

llist = ldt_proxy.llist
llist.update({"key" => 2, "text" => "z"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"z", "key"=>2}]
```

If you don't want this behavior, you can disable it with setting client's `:ldt_proxy` option to `false`
```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000, ldt_proxy: false)
key    = AerospikeC::Key.new("test", "test", "llist")

bin_name = "some_bin_name"
llist    = client.llist(key, bin_name)

#
# (...) some llist operations
#

bins = client.get(key) # => {"LDTCONTROLBIN"=>nil, "some_bin_name"=>nil}
```

<hr />

#### With a new llist, you can use any of the methods specified below:

[Methods](#methods):
  - [#add](#add)
  - [#add_all](#add_all)
  - [#update](#update)
  - [#update_all](#update_all)
  - [#scan](#scan)
  - [#delete](#delete)
  - [#find](#find)
  - [#find_first](#find_first)
  - [#first](#first)
  - [#find_last](#find_last)
  - [#last](#last)
  - [#find_from](#find_from)
  - [#size](#size)
  - [#key](#key)
  - [#bin_name](#bin_name)



<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- add -->
<a name="add"></a>

### add(value, options = {})

Add value into list.

Parameters:

- `value` - some value to add
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `true` if added succesfuly

Example:

```ruby
llist.add(1) # => true
llist.add(2) # => true
llist.add(3) # => true
llist.scan # => [1, 2, 3]
```


<!--===============================================================================-->
<hr/>
<!-- add_all -->
<a name="add_all"></a>

### add_all(values, options = {})

Add many values into list in one operation

Parameters:

- `values` - array of values to add
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `true` if added succesfuly

```ruby
values = [1, 2, 3]
llist.add_all(values) # => true
llist.scan # => [1, 2, 3]
```

<!--===============================================================================-->
<hr/>
<!-- update -->
<a name="update"></a>

### update(value, options = {})

Add value into list.

Parameters:

- `value` - some value to update
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `true` if updated succesfuly

Example:

```ruby
llist.add({"key" => 0, "val" => 1}) # => true
llist.scan # => [{"key" => 0, "val" => 1}]

llist.update({"key" => 0, "val" => 3}) # => true
llist.scan # => [{"key" => 0, "val" => 3}]
```


<!--===============================================================================-->
<hr/>
<!-- update_all -->
<a name="update_all"></a>

### update_all(values, options = {})

Add many values into list in one operation

Parameters:

- `values` - array of values to update
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `true` if updated succesfuly

```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"c", "key"=>2}]

values = [
{"text"=>"aa", "key"=>0},
{"text"=>"bb", "key"=>1},
{"text"=>"cc", "key"=>2}
]

llist.update(values) # => true
llist.scan # => [{"text"=>"aa", "key"=>0}, {"text"=>"bb", "key"=>1}, {"text"=>"cc", "key"=>2}]
```


<!--===============================================================================-->
<hr/>
<!-- scan -->
<a name="scan"></a>

### scan(options = {})

Return all values in large list

Parameters:

- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object


<!--===============================================================================-->
<hr/>
<!-- delete -->
<a name="delete"></a>

### delete(value, options = {})

Delete value from large list

Parameters:

- `value` - some value `key` to remove
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `true` if removed succesfuly

Example:

```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"b", "key"=>1}, {"text"=>"c", "key"=>2}]

llist.delete(1)
llist.scan # => [{"text"=>"a", "key"=>0}, {"text"=>"c", "key"=>2}]
```


<!--===============================================================================-->
<hr/>
<!-- find -->
<a name="find"></a>

### find(value, options = {})

Find value by key

Parameters:

- `value` - some value `key` to find
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:
- `hash` with data if found
- `nil` if AEROSPIKE_ERR_LARGE_ITEM_NOT_FOUND

Example:

```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})

llist.find(1) # => {"key" => 1, "text" => "b"}
llist.find(69) # => nil
```


<!--===============================================================================-->
<hr/>
<!-- find_first -->
<a name="find_first"></a>

### find_first(count, options = {})

Find first `count` elements in list

Parameters:

- `count` - integer, how many elements to get
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Retruns:
- `hash` with data

Example:
```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})

llist.find_first(2) # => [{"key" => 0, "text" => "a"}, {"key" => 1, "text" => "b"}]
```


<!--===============================================================================-->
<hr/>
<!-- first -->
<a name="first"></a>

### first(options = {})
`shortcut for: find_first(1, options)`

Get first element in list

Parameters:

- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object


<!--===============================================================================-->
<hr/>
<!-- find_last -->
<a name="find_last"></a>

### find_last(count, options = {})

Find last `count` elements in list

Parameters:

- `count` - integer, how many elements to get
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Retruns:
- `hash` with data

Example:
```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})

llist.find_last(2) # => [{"key" => 2, "text" => "c"}, {"key" => 1, "text" => "b"}]
```


<!--===============================================================================-->
<hr/>
<!-- last -->
<a name="last"></a>

### last(options = {})
`shortcut for: find_last(1, options)`

Get last element in list

Parameters:

- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object



<!--===============================================================================-->
<hr/>
<!-- find_from -->
<a name="find_from"></a>

### find_from(value, count, options = {})

Get `count` values, starting from `value`

Parameters:

- `value` - value to start searching
- `count` - integer, how many elements to get
- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Retruns:
- `hash` with data

Example:

```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.add({"key" => 3, "text" => "d"})

llist.find_from(1, 2) # => [{"key" => 1, "text" => "b"}, {"key" => 2, "text" => "c"}]
```


<!--===============================================================================-->
<hr/>
<!-- size -->
<a name="size"></a>

### size(options = {})

Llist size

Parameters:

- `options`:
  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Retruns:
- `hash` with data

Example:

```ruby
llist.add({"key" => 0, "text" => "a"})
llist.add({"key" => 1, "text" => "b"})
llist.add({"key" => 2, "text" => "c"})
llist.add({"key" => 3, "text" => "d"})

llist.size # => 4
```

<!--===============================================================================-->
<hr/>
<!-- key -->
<a name="key"></a>

### key

Returns [AerospikeC::Key](key.md) object used with Llist

<!--===============================================================================-->
<hr/>
<!-- bin_name -->
<a name="bin_name"></a>

### bin_name

Returns `bin_name` string used with Llist