# Client Class

The `Client` class provides methods which can be used to perform operations on an Aerospike
database cluster. In order to get an instance of the Client class, you need to initialize it:

```ruby
  client = AerospikeC::Client.new("127.0.0.1", 3000)
```

*Notice*: Examples in the section are only intended to illuminate simple use cases without too much distraction. Always follow good coding practices in production.

With a new client, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#put](#put)
  - [#get](#get)
  - [#delete](#delete)
  - [#logger=](#logger=)
  - [#exists?](#exists?)
  - [#get_header](#get_header)


<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(host, port, options={})

Create instance of AerospikeC::Client class.

Parameters:

- `host`    - Aerospike server address
- `port`    - Aerospike server port
- `options:`

  - `:lua_path` - if you are using using user-defined functions (UDF) for processing query results (i.e aggregations), then you will find it useful to set the :lua_path settings. Of particular importance is the :lua_path, which allows you to define a path to where the client library will look for Lua files for processing

  - `:logger` - sets logger for the AerospikeC classes operations, see also [#logger=](#logger=)

Example:

```ruby
require "aerospike_c_ruby"
require "logger"

options = {
  logger: Logger.new("log/aerospike_c.log")
  lua_path: File.expand_path(File.join(File.dirname(__FILE__), "lua"))
}

client = AerospikeC::Client.new("127.0.0.1", 3000, options)
```

<!--===============================================================================-->
<hr/>
<!-- put -->
<a name="put"></a>

### put(key, bins, options = {})

Adding record to the cluster.

Parameters:

- `key`  - [AerospikeC::Key](key.md) object
- `bins` - either hash or [AerospikeC::Record](record.md) object
- `options:`

  - `:ttl` - time to live record (default: 0, which means server will use namespace default)

  - @TODO options policy

Retrun:
- `true` if added succesfuly

Example:

```ruby
key  = AerospikeC::Key.new("test", "test", "test")
key2 = AerospikeC::Key.new("test", "test", "test2")

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

client.put(key, bins) # => true
client.put(key2, bins, ttl: 60) # => true, 60s expiration time
```

<!--===============================================================================-->
<hr/>
<!-- get -->
<a name="get"></a>

### get(key, specific_bins = [], options = {})

Getting record for specifed key. Read all bins of record or given in specific_bins argument.

Parameters:

- `key`           - [AerospikeC::Key](key.md) object
- `specific_bins` - array of strings representing bin names
- `options`:

  - `with_header` - returns also generation and expire_in field (default: false)

  - @TODO options policy

Retrun:
- `hash` representing record if operation succeds
- `nil` when AEROSPIKE_ERR_RECORD_NOT_FOUND

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "test")

bins = {
  "bin1" => 1,
  "bin2" => "two"
  "bin3" => 3
}

client.put(key, bins, ttl: 60)

client.get(key) # => {"bin1" => 1, "bin2" => "two", "bin3" => 3}
client.get(key, ["bin1", "bin2"], {with_header: true})
# => {"bin1" => 1, "bin2" => "two", "header" => {"gen" => 1, "expire_in" => 60}}
```

<!--===============================================================================-->
<hr/>
<!-- delete -->
<a name="delete"></a>

### delete(key)

Removing record from the cluster.

Parameters:

- `key` - [AerospikeC::Key](key.md) object

Retrun:
- `true` if removed succesfuly
- `nil` when AEROSPIKE_ERR_RECORD_NOT_FOUND

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "test")
client.delete(key) # => true
client.delete(key) # => nil
```

<!--===============================================================================-->
<hr/>
<!-- logger= -->
<a name="logger="></a>

### logger=(logger)

Sets logger for the AerospikeC classes operations.

Parameters:

- `logger` - to be honest any class that responds to `debug`, `info`, `warn`, `error`, `fatal` methods

Retrun:

- `true` if no errors while assigning

Example:

```ruby
require "aerospike_c_ruby"
require "logger"

client = AerospikeC::Client.new("127.0.0.1", 3000)
client.logger = Logger.new("log/aerospike_c.log")
```

<!--===============================================================================-->
<hr/>
<!-- exists? -->
<a name="exists?"></a>

### exists?(key)

Check if key exist in cluster.

Parameters:

- `key` - [AerospikeC::Key](key.md) object

Retrun:
- `true` if exists
- `false` otherwise

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "test")

client.exists?(key) # => true
client.delete(key)
client.exists?(key) # => false
```

<!--===============================================================================-->
<hr/>
<!-- get_header -->
<a name="get_header"></a>

### get_header(key)

Get header of record.

Parameters:

- `key` - [AerospikeC::Key](key.md) object

Return:
- `hash` representing record header
- `nil` when AEROSPIKE_ERR_RECORD_NOT_FOUND

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "test")

bins = {"bin" => "value"}
client.put(key, bins, ttl: 60)

client.get_header(key) # => {"gen" => 1, "expire_in" => 60}
```