# Key Class

Represents Aerospike key in cluster. Note that once initialized key cannot be changed, becaouse of aerospike-c-client key wrapping into ruby. This behaviuor may change in the future.

With a new key, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#namespace](#namespace)
  - [#set](#set)
  - [#key](#key)
  - [#digest](#digest)
  - [#==](#==)
  - [#hash](#hash)

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

Retruns instance `namespace`

Example:

```ruby
AerospikeC::Key.new("testnamespace", "test", "test") # => "testnamespace"
```

<!--===============================================================================-->
<hr/>
<!-- set -->
<a name="set"></a>

### set

Retruns instance `set`

Example:

```ruby
AerospikeC::Key.new("test", "testset", "test") # => "testset"
```

<!--===============================================================================-->
<hr/>
<!-- key -->
<a name="key"></a>

### key

Retruns instance `key`

Example:

```ruby
AerospikeC::Key.new("test", "test", "testkey") # => "testkey"
```

<!--===============================================================================-->
<hr/>
<!-- digest -->
<a name="digest"></a>

### digest

Retruns key digest as array of integers

Example:

```ruby
key1 = AerospikeC::Key.new("test", "test2", "key1")
key1.digest # => [202, 73, 124, 49, 153, 105, 218, 146, 189, 22, 79, 241, 76, 93, 140, 205, 25, 58, 100, 122]
```


<!--===============================================================================-->
<hr/>
<!-- == -->
<a name="=="></a>

### ==(key)
`alias: eql?`

Compare keys

Example:

```ruby
key1 = AerospikeC::Key.new("test", "test2", "key1")
key2 = AerospikeC::Key.new("test", "test2", "key2")
key3 = AerospikeC::Key.new("test", "test2", "key2")

key1 == key2 # => false
key1 == key1 # => true
key2 == key3 # => true
```


<!--===============================================================================-->
<hr/>
<!-- hash -->
<a name="hash"></a>

### hash

Compute hash of key, it is based on [digest](#digest)

Example:

```ruby
key1 = AerospikeC::Key.new("test", "test2", "key1")
key2 = AerospikeC::Key.new("test", "test2", "key2")
key3 = AerospikeC::Key.new("test", "test2", "key2")

key1.hash # => -4521481596764537602
key2.hash # => 814698596044429939
key3.hash # => 814698596044429939
```