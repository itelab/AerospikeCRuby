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