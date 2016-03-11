# Operation Class

Operation is a class for usage with [Client #operate](client.md#operate) method. Use it if you want to perform many operations on record in single server call. Note that operations are just array with hashes of options. There is no wrapping any aerospike-c-client struct. If you understand the options hashes you can add them freely without usage of specific methods.

With a new operation, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#ttl](#ttl)
  - [#ttl=](#ttl=)
  - [#increment!](#increment!)
  - [#touch!](#touch!)
  - [#append!](#append!)
  - [#prepend!](#prepend!)
  - [#read!](#read!)
  - [#write!](#write!)
  - [#operations](#operations)
  - [#<<](#<<)
  - [#count](#count)
  - [#list_append!](#list_append!)
  - [#list_set!](#list_set!)
  - [#list_trim!](#list_trim!)
  - [#list_clear!](#list_clear!)
  - [#list_pop!](#list_pop!)
  - [#list_pop_range!](#list_pop_range!)
  - [#list_pop_range_from!](#list_pop_range_from!)
  - [#list_remove!](#list_remove!)
  - [#list_remove_range!](#list_remove_range!)
  - [#list_remove_range_from!](#list_remove_range_from!)
  - [#list_get!](#list_get!)
  - [#list_get_range!](#list_get_range!)
  - [#list_get_range_from!](#list_get_range_from!)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(operations = [], options = {})

Parameters:

- `operations` - array of operations
- `options:`

  - `:ttl` - time to live record (default: 0, which means server will use namespace default)

  - @TODO options policy


<!--===============================================================================-->
<hr/>
<!-- ttl -->
<a name="ttl"></a>

### ttl

Retruns `ttl` saved in operation instance.

Example:

```ruby
operation = AerospikeC::Operation.new
operation.ttl # => 0
```


<!--===============================================================================-->
<hr/>
<!-- ttl= -->
<a name="ttl="></a>

### ttl=(ttl)

Save new value of time to live.

Parameters:

- `ttl` - time to live record

Example:

```ruby
operation = AerospikeC::Operation.new
operation.ttl # => 0
operation.ttl = 30
operation.ttl # => 30
```

<!--===============================================================================-->
<hr/>
<!-- increment! -->
<a name="increment!"></a>

### increment!(bin, val)

Add increment operation. Works only with integer bins.

Parameters:

- `bin` - name of the bin to perform the operation on
- `val` - integer, number to be used with increment

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new
operation.increment!("int", 1).increment!("int2", 20)
operation.operations
# => [{:operation=>:increment, :bin=>"int", :value=>1}, {:operation=>:increment, :bin=>"int2", :value=>20}]
```

<!--===============================================================================-->
<hr/>
<!-- touch! -->
<a name="touch!"></a>

### touch!

Add touch operation.

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.touch!
operation.operations # => [{:operation=>:touch}]
```

<!--===============================================================================-->
<hr/>
<!-- append! -->
<a name="append!"></a>

### append!(bin, val)

Add append operation. Works only with string bins.

Parameters:

- `bin` - name of the bin to perform the operation on
- `val` - value to append on bin

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.append!("string_bin", "append_text")
operation.operations # => [{:operation=>:append, :bin=>"string_bin", :value=>"append_text"}]
```

<!--===============================================================================-->
<hr/>
<!-- prepend! -->
<a name="prepend!"></a>

### prepend!(bin, val)

Add prepend operation. Works only with string bins.

Parameters:

- `bin` - name of the bin to perform the operation on
- `val` - value to prepend on bin

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.prepend!("string_bin", "prepend_text")
operation.operations # => [{:operation=>:prepend, :bin=>"string_bin", :value=>"prepend_text"}]
```

<!--===============================================================================-->
<hr/>
<!-- read! -->
<a name="read!"></a>

### read!(bin)

Add read operation.

Parameters:

- `bin` - name of the bin to read

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.read!("int")
operation.operations # => [{:operation=>:read, :bin=>"int"}]
```

<!--===============================================================================-->
<hr/>
<!-- write! -->
<a name="write!"></a>

### write!(bin, val)

Add write operation.

Parameters:

- `bin` - name of the bin to write
- `val` - value of bin

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.write!("new_int_bin", 10)
operation.operations # => [{:operation=>:write, :bin=>"new_bin_int", :value=>10}]
```

<!--===============================================================================-->
<hr/>
<!-- operations -->
<a name="operations"></a>

### operations

Operations in object

Returns `array` of hashes where each hash represent operation

Example:

```ruby
operation = AerospikeC::Operation.new

operation.write!("new_int_bin", 10).touch!.read!("int")
operation.operations
# => [{:operation=>:write, :bin=>"new_bin_int", :value=>10}, {:operation=>:touch}, {:operation=>:read, :bin=>"int"}]
```

<!--===============================================================================-->
<hr/>
<!-- << -->
<a name="<<"></a>

### <<(operation)

Add operation.

Parameters:

- `operation` - hash representing operation

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new
operation.operations # => []

operation << {:operation=>:touch}
operation.operations # => [{:operation=>:touch}]
```

<!--===============================================================================-->
<hr/>
<!-- count -->
<a name="count"></a>

### count

`alias: size, length`

Operations count

Example:

```ruby
operation = AerospikeC::Operation.new
operation.write!("new_int_bin", 10).touch!.read!("int")
operation.count # => 3
```

<!--===============================================================================-->
<hr/>
<!-- list_append! -->
<a name="list_append!"></a>

### list_append!(bin, val)

Append value to the list

Parameters:

- `bin` - name of the bin with list
- `val` - value to append

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_append!("tab_bin", 10)
operation.operations # => [{:operation=>:list_append, :bin=>"tab_bin", :value=>10}]
```

<!--===============================================================================-->
<hr/>
<!-- list_set! -->
<a name="list_set!"></a>

### list_set!(bin, val, index)

Set value at given index

Parameters:

- `bin` - name of the bin with list
- `val` - value to set
- `index` - index to set value on

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_set!("tab_bin", 10, 1)
operation.operations # => [{:operation=>:list_set, :bin=>"tab_bin", :value=>10, :at=>1}]
```


<!--===============================================================================-->
<hr/>
<!-- list_trim! -->
<a name="list_trim!"></a>

### list_trim!(bin, index, count)

Remove values not in range (index, count)

Parameters:

- `bin` - name of the bin with list
- `index` - index of list
- `count` - count of values

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_trim!("tab_bin", 10, 5)
operation.operations # => [{:operation=>:list_trim, :bin=>"tab_bin", :value=>10, :count=>5}]
```

<!--===============================================================================-->
<hr/>
<!-- list_clear! -->
<a name="list_clear!"></a>

### list_clear!(bin)

Clear list

Parameters:

- `bin` - name of the bin with list

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_clear!("tab_bin")
operation.operations # => [{:operation=>:list_clear, :bin=>"tab_bin"}]
```

<!--===============================================================================-->
<hr/>
<!-- list_pop! -->
<a name="list_pop!"></a>

### list_pop!(bin, index)

Pop from list

Parameters:

- `bin` - name of the bin with list
- `index` - index to pop from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_pop!("tab_bin", 3)
operation.operations # => [{:operation=>:list_pop, :bin=>"tab_bin", :value=>3}]
```

<!--===============================================================================-->
<hr/>
<!-- list_pop_range! -->
<a name="list_pop_range!"></a>

### list_pop_range!(bin, index, count)

Pop range from list. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to pop from
- `count` - how many from index

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_pop_range!("tab_bin", 3, 4)
operation.operations # => [{:operation=>:list_pop_range, :bin=>"tab_bin", :value=>3, :count=>4}]
```

<!--===============================================================================-->
<hr/>
<!-- list_pop_range_from! -->
<a name="list_pop_range_from!"></a>

### list_pop_range_from!(bin, index)

Pop range from list, starting at index. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to start range from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_pop_range_from!("tab_bin", 3)
operation.operations # => [{:operation=>:list_pop_range_from, :bin=>"tab_bin", :value=>3}]
```

<!--===============================================================================-->
<hr/>
<!-- list_remove! -->
<a name="list_remove!"></a>

### list_remove!(bin, index)

Pop from list

Parameters:

- `bin` - name of the bin with list
- `index` - index to remove from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_remove!("tab_bin", 3)
operation.operations # => [{:operation=>:list_remove, :bin=>"tab_bin", :value=>3}]
```

<!--===============================================================================-->
<hr/>
<!-- list_remove_range! -->
<a name="list_remove_range!"></a>

### list_remove_range!(bin, index, count)

Pop range from list. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to remove from
- `count` - how many from index

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_remove_range!("tab_bin", 3, 4)
operation.operations # => [{:operation=>:list_remove_range, :bin=>"tab_bin", :value=>3, :count=>4}]
```

<!--===============================================================================-->
<hr/>
<!-- list_remove_range_from! -->
<a name="list_remove_range_from!"></a>

### list_remove_range_from!(bin, index)

Pop range from list, starting at index. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to start range from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_remove_range_from!("tab_bin", 3)
operation.operations # => [{:operation=>:list_remove_range_from, :bin=>"tab_bin", :value=>3}]
```

<!--===============================================================================-->
<hr/>
<!-- list_get! -->
<a name="list_get!"></a>

### list_get!(bin, index)

Pop from list

Parameters:

- `bin` - name of the bin with list
- `index` - index to get from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_get!("tab_bin", 3)
operation.operations # => [{:operation=>:list_get, :bin=>"tab_bin", :value=>3}]
```

<!--===============================================================================-->
<hr/>
<!-- list_get_range! -->
<a name="list_get_range!"></a>

### list_get_range!(bin, index, count)

Pop range from list. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to get from
- `count` - how many from index

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_get_range!("tab_bin", 3, 4)
operation.operations # => [{:operation=>:list_get_range, :bin=>"tab_bin", :value=>3, :count=>4}]
```

<!--===============================================================================-->
<hr/>
<!-- list_get_range_from! -->
<a name="list_get_range_from!"></a>

### list_get_range_from!(bin, index)

Pop range from list, starting at index. Negative index counts from end of list.

Parameters:

- `bin` - name of the bin with list
- `index` - index to start range from

Returns `self`

Example:

```ruby
operation = AerospikeC::Operation.new

operation.list_get_range_from!("tab_bin", 3)
operation.operations # => [{:operation=>:list_get_range_from, :bin=>"tab_bin", :value=>3}]
```