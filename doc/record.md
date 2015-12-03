# Record Class

Represents Aerospike record in cluster. Wrapps aerospike-c-client record into ruby. Note that beacaouse of wrapping, memory usage is higher than expected from pure ruby hash objects.

With a new record, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#bins](#bins)
  - [#bins=](#bins=)
  - [#ttl](#ttl)
  - [#ttl=](#ttl=)
  - [#length](#length)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(bins, options = {})

Creates instance of AerospikeC::Record class.

Parameters:

- `bins` - hash where key is bin name, and value is bin value
- `options:`

  - `:ttl` - time to live record (default: 0, which means server will use namespace default)

Example:

```ruby
bins = {
  "bin_int" => 15,
  "bin_string" => "string",
  "bin_tab" => [1, 2, "three", [1, "nested_tab"]],

  "bin_hash" => {
    "hash_int" => 1,
    "hash_string" => "two",
    "hash_nested" => {
      "hash_nested_int" => 5,
      "hash_nested_sting" => "hash_nested_string"
    }
  }
}

record = AerospikeC::Record.new(bins, ttl: 60)
```

<!--===============================================================================-->
<hr/>
<!-- bins -->
<a name="bins"></a>

### bins

Retruns `bins` saved in record instance

Example:
```ruby
bins = {
  "x" => 1,
  "y" => "two"
}

record = AerospikeC::Record.new(bins)
record.bins # => {"x" => 1, "y" => two}
```

<!--===============================================================================-->
<hr/>
<!-- bins= -->
<a name="bins="></a>

### bins=(bins)

Recreates wrapped aerospike-c-client record into ruby, for given bins hash value

Parameters:

- `bins` - hash where key is bin name, and value is bin value

Return `self`

Example:
```ruby
bins = {
  "x" => 1,
  "y" => "two"
}

record = AerospikeC::Record.new(bins)
record.bins # => {"x" => 1, "y" => two}

record.bins = {"z" => 3, "q" => "four"}
record.bins # => {"z" => 3, "q" => "four"}
```

<!--===============================================================================-->
<hr/>
<!-- ttl -->
<a name="ttl"></a>

### ttl

Retruns `ttl` saved in record instance

Example:

```ruby
bins = {
  "x" => 1,
  "y" => "two"
}

record = AerospikeC::Record.new(bins, ttl: 60)
record.ttl # => 60
```

<!--===============================================================================-->
<hr/>
<!-- ttl= -->
<a name="ttl="></a>

### ttl=(ttl)

Save new value of time to live

Parameters:

- `ttl` - time to live record

Example:
```ruby
bins = {
  "x" => 1,
  "y" => "two"
}

record = AerospikeC::Record.new(bins, ttl: 60)
record.ttl # => 60
record.ttl = 30
record.ttl # => 30
```

<!--===============================================================================-->
<hr/>
<!-- length -->
<a name="length"></a>

### length

Bins hash length

Example:

```ruby
bins = {
  "x" => 1,
  "y" => "two"
}

record = AerospikeC::Record.new(bins)
record.length # => 2