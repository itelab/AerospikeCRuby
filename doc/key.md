# Key Class

Represents Aerospike key in cluster. Note that once initialized key cannot be changed, becaouse of aerospike-c-client key wrapping into ruby. This behaviuor may change in the future.

With a new key, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#namespace](#namespace)
  - [#set](#set)
  - [#key](#key)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(namespace, set, key)

Creates instance of AerospikeC::Key class.

Parameters:

- `namespace` - namespace for the key
- `set`       - set for the key
- `key`       - value for key

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "test")
```

<!--===============================================================================-->
<hr/>
<!-- namespace -->
<a name="namespace"></a>

### namespace

Retruns instance namespace

Example:

```ruby
AerospikeC::Key.new("testnamespace", "test", "test") # => "testnamespace"
```

<!--===============================================================================-->
<hr/>
<!-- set -->
<a name="set"></a>

### set

Retruns instance set

Example:

```ruby
AerospikeC::Key.new("test", "testset", "test") # => "testset"
```

<!--===============================================================================-->
<hr/>
<!-- key -->
<a name="key"></a>

### key

Retruns instance key

Example:

```ruby
AerospikeC::Key.new("test", "test", "testkey") # => "testkey"
```