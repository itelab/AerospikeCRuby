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
  - [#close](#close)
  - [#put](#put)
  - [#get](#get)
  - [#delete](#delete)
  - [#logger=](#logger=)
  - [#exists?](#exists?)
  - [#get_header](#get_header)
  - [#batch_get](#batch_get)
  - [#touch](#touch)
  - [#operate](#operate)
  - [#operation](#operation)
  - [#create_index](#create_index)
  - [#drop_index](#drop_index)
  - [#info_cmd](#info_cmd)
  - [#list_indexes](#list_indexes)
  - [#statistics](#statistics)
  - [#namespaces](#namespaces)


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
<!-- close -->
<a name="close"></a>

### close

Close connections to the cluster.

Return `self`

Example:

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)

key = AerospikeC::Key.new("test", "test", "test")

bins = {
  "bin1" => 1,
  "bin2" => "two"
  "bin3" => 3
}

client.put(key, bins)

# (...) other operations

client.close
```

<!--===============================================================================-->
<hr/>
<!-- put -->
<a name="put"></a>

### put(key, bins, options = {})

Adding record to the cluster. Creates [AerospikeC::Record](record.md) object, inside c-api if bins parameter is hash

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

- `logger` - to be honest any object that responds to `debug`, `info`, `warn`, `error`, `fatal` methods

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
client.delete(key)
client.get_header(key) # => nil
```

<!--===============================================================================-->
<hr/>
<!-- batch_get -->
<a name="batch_get"></a>

### batch_get(keys, specific_bins = nil, options = {})

Getting batch of records in one call. Batch size is limited on aerospike server (default: 5000)

Parameters:

- `keys` - array of [AerospikeC::Key](key.md) objects
- `specific_bins` - array of strings representing bin names
- `options`:

  - `with_header` - returns also generation and expire_in field (default: false)

  - @TODO options policy

Return:

- `array of hashes` where each hash represents record bins

Example:

```ruby
keys = []
i    = 0

5.times do
  bins = {"index" => i, "message" => "message_#{i}"}
  key = AerospikeC::Key.new("test", "test", "batch_get_#{i}")

  client.put(key, bins, ttl: 60)

  keys << key
  i += 1
end

client.batch_get(keys)
# [{"index"=>0, "message"=>"message_0"}, {"index"=>1, "message"=>"message_1"}, {"index"=>2, "message"=>"message_2"}, {"index"=>3, "message"=>"message_3"}, {"index"=>4, "message"=>"message_4"}]

client.batch_get(keys, ["index"], with_header: true)
# [{"index"=>0, "header"=>{"gen"=>1, "expire_in"=>60}}, {"index"=>1, "header"=>{"gen"=>1, "expire_in"=>60}}, {"index"=>2, "header"=>{"gen"=>1, "expire_in"=>60}}, {"index"=>3, "header"=>{"gen"=>1, "expire_in"=>60}}, {"index"=>4, "header"=>{"gen"=>1, "expire_in"=>60}}]
```

<!--===============================================================================-->
<hr/>
<!-- touch -->
<a name="touch"></a>

### touch(key, options = {})

Refresh expiration time of record and increment generation

Parameters:

- `key`  - [AerospikeC::Key](key.md) object
- `options:`

  - `:ttl` - time to live record (default: 0, which means server will use namespace default)

  - @TODO options policy

Return:
- `hash` representing record header
- `nil` when AEROSPIKE_ERR_RECORD_NOT_FOUND

Example:

```ruby

key = AerospikeC::Key.new("test", "test", "test")

bins = {"bin" => "value"}
client.put(key, bins, ttl: 60)

sleep 5
client.get_header(key) # => {"gen" => 1, "expire_in" => 55}
client.touch(key, ttl: 70) # => {"gen" => 2, "expire_in" => 70}
```

<!--===============================================================================-->
<hr/>
<!-- operate -->
<a name="operate"></a>

### operate(key, operations)

Perform many operations on record in single server call.

Parameters:

- `key`        - [AerospikeC::Key](key.md) object
- `operations` - [AerospikeC::Operation](operation.md) object

Return:

- `hash` representing record if succeds
- `nil` when AEROSPIKE_ERR_RECORD_NOT_FOUND

Example:

```ruby
key = AerospikeC::Key.new("test", "test", "operate")

bins = {
  "int" => 1,
  "string" => "b"
}

ops = AerospikeC::Operation.new # eql to client.operation

ops.increment!("int", 1).append!("string", "c").prepend!("string", "a").touch!
ops.write!("new_bin_int", 10).write!("new_bin_str", "new")

ops.read!("int")
ops.read!("string")
ops.read!("new_bin_int")
ops.read!("new_bin_str")

client.operate(key, ops) # => {"int"=>2, "string"=>"abc", "new_bin_int"=>10, "new_bin_str"=>"new"}
```

<!--===============================================================================-->
<hr/>
<!-- operation -->
<a name="operation"></a>

### operation

Returns new [AerospikeC::Operation](operation.md) object

Example:

```ruby
client.operation # => #<AerospikeC::Operation:0x00000000cec368 @operations=[], @ttl=0>
```

<!--===============================================================================-->
<hr/>
<!-- create_index -->
<a name="create_index"></a>

### create_index(namespace, set, bin, name, data_type, options = {})

Creates index on `bin`, within `namespace` and `set`, and names it as `name`.
Aerospike reference: http://www.aerospike.com/docs/architecture/secondary-index.html

Parameters:

- `namespace` - string, namespace to be indexed
- `set`       - string, set to be indexed
- `bin`       - string, bin or complex position name to be indexed
- `name`      - string, name of the index
- `data_type` - symbol, data type of index, `:string` or `:numeric`
- `options`:

  - @TODO options policy

Returns:
- [AerospikeC::IndexTask](index_task.md) object

Example:

```ruby
tasks = []
tasks << client.create_index("test", "test", "test_bin", "test_test_test_bin_idx", :numeric)
tasks << client.create_index("test", "test", "test_bin2", "test_test_test_bin2_idx", :string)

tasks.each do |task|
  puts task.inspect
end

tasks.each do |task|
  task.wait_till_completed
end
```

<!--===============================================================================-->
<hr/>
<!-- drop_index -->
<a name="drop_index"></a>

### drop_index(namespace, name, options = {})

Drops index with `name` in given `namespace`

Parameters:
- `namespace` - string, index namespace
- `name`      - string, index name
- `options`:

  - @TODO options policy

Returns:

- `true` if drop executed correctly

Example:

```ruby
client.drop_index("test", "test_test_test_bin_idx") # => true
client.drop_index("test", "test_test_test_bin2_idx") # => true
```


<!--===============================================================================-->
<hr/>
<!-- info_cmd -->
<a name="info_cmd"></a>

### info_cmd(cmd)

Execute asinfo command on server.
Aerospike reference: http://www.aerospike.com/docs/reference/info/

Parameters:

- `cmd` - string, command to execute

Returns:
- `string` - response from server

Example:

```ruby
client.info_cmd("logs") # => "logs  0:/var/log/aerospike/aerospike.log"
```

<!--===============================================================================-->
<hr/>
<!-- list_indexes -->
<a name="list_indexes"></a>

### list_indexes

Execute `info_cmd("sindex")` and parse response

Retruns:

- `array` of hashes, each for index info


<!--===============================================================================-->
<hr/>
<!-- statistics -->
<a name="statistics"></a>

### statistics

Execute `info_cmd("statistics")` and parse response

Retruns:

- `hash` of informations

<!--===============================================================================-->
<hr/>
<!-- namespaces -->
<a name="namespaces"></a>

### namespaces

Execute `info_cmd("namespaces")` and parse response

Retruns:

- `array` of namespaces names