# Client Class

The `Client` class provides methods which can be used to perform operations on an Aerospike
database cluster. In order to get an instance of the Client class, you need to initialize it:

```ruby
  client = AerospikeC::Client.new("127.0.0.1", 3000)
```

*Notice*: Examples in the section are only intended to illuminate simple use cases without too much distraction. Always follow good coding practices in production.

With a new client, you can use any of the methods specified below:

[Methods](#methods):
  - Core:
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

  - Operations:
    - [#operate](#operate)
    - [#operation](#operation)

  - Indexes:
    - [#create_index](#create_index)
    - [#drop_index](#drop_index)

  - Info:
    - [#info_cmd](#info_cmd)
    - [#list_indexes](#list_indexes)
    - [#statistics](#statistics)
    - [#namespaces](#namespaces)

  - Udf:
    - [#register_udf](#register_udf)
    - [#drop_udf](#drop_udf)
    - [#list_udf](#list_udf)
    - [#execute_udf](#execute_udf)

  - Scan:
    - [#scan](#scan)
    - [#execute_udf_on_scan](#execute_udf_on_scan)
    - [#background_execute_udf_on_scan](#background_execute_udf_on_scan)

  - Query & Aggregation:
    - [#query](#query)
    - [#execute_udf_on_query](#execute_udf_on_query)
    - [#background_execute_udf_on_query](#background_execute_udf_on_query)

  - Large Data Type:
    - [#llist](#llist)


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

  - `:lua_path` - if you are using using user-defined functions (UDF) for processing query results (i.e aggregations), then you will find it useful to set the :lua_path settings. Of particular importance is the :lua_path, which allows you to define a path to where the client library will look for Lua files for processing, `default: nil`

  - `:conn_timeout` - initial host connection timeout in milliseconds. The timeout when opening a connection to the server host for the first time, `default: 1000`

  - `:fail_not_connected` - if fail_not_connected is true, the cluster creation will fail when all seed hosts are not reachable. If fail_not_connected is false, an empty cluster will be created and the client will automatically connect when Aerospike server becomes available, `default: true`

  - `:logger` - sets logger for the AerospikeC classes operations, see also [#logger=](#logger=), `default: nil`

  - `:hosts` - array of hashes {host: 'hostip', port: someportint}. You can define up to 256 hosts for the seed. The client will iterate over the list until it connects with one of the hosts, `default: nil`

  - `:user` - user authentication to cluster, `default: nil`

  - `:password` - password authentication to cluster. The hashed value of password will be stored by the client and sent to server in same format, `default: nil`

  - `:interval` - polling interval in milliseconds for cluster tender, `default: 1000`

  - `:thread_pool_size` - number of threads stored in underlying thread pool that is used in batch/scan/query commands. These commands are often sent to multiple server nodes in parallel threads. A thread pool improves performance because threads do not have to be created/destroyed for each command. Calculate your value using the following formula: <b>thread_pool_size = (concurrent batch/scan/query commands) * (server nodes)</b>, `default: 16`

  - `:max_threads` - estimate of incoming threads concurrently using synchronous methods in the client instance. This field is used to size the synchronous connection pool for each server node, `default: 300`

  - `:ldt_proxy` - usage of [AerospikeC::LdtProxy](llist.md#workaround_note), `default: true`

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

  - `:policy` - [AerospikeC::WritePolicy](policy.md#write_policy) object

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

  - `:policy` - [AerospikeC::ReadPolicy](policy.md#read_policy) object

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
- `options`:

  - `:policy` - [AerospikeC::RemovePolicy](policy.md#remove_policy) object

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

### exists?(key, options = {})

Check if key exist in cluster.

Parameters:

- `key` - [AerospikeC::Key](key.md) object
- `options`:

  - `:policy` - [AerospikeC::ReadPolicy](policy.md#read_policy) object

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

### get_header(key, options = {})

Get header of record.

Parameters:

- `key` - [AerospikeC::Key](key.md) object
- `options`:

  - `:policy` - [AerospikeC::ReadPolicy](policy.md#read_policy) object

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
- `data_type` - symbol, data type of index, `:string`, `:numeric` or `:geosphere`
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

Execute `info_cmd("sindex")` and parse response.

Retruns:

- `array` of hashes, each for index info


<!--===============================================================================-->
<hr/>
<!-- statistics -->
<a name="statistics"></a>

### statistics

Execute `info_cmd("statistics")` and parse response.

Retruns:

- `hash` of informations

<!--===============================================================================-->
<hr/>
<!-- namespaces -->
<a name="namespaces"></a>

### namespaces

Execute `info_cmd("namespaces")` and parse response.

Retruns:

- `array` of namespaces names


<!--===============================================================================-->
<hr/>
<!-- register_udf -->
<a name="register_udf"></a>

### register_udf(path_to_file, server_path, language = :lua, options = {})

Register udf on server.

Parameters:

- `path_to_file` - absolute path to udf file
- `server_path`  - where to put udf on the server
- `language`     - udf language (in aerospike-c-client v3.1.24, only lua language is available)
- `options`:

  - @TODO options policy

Return:

- [AerospikeC::UdfTask](udf_task.md) object

Example:

```ruby
hello_world_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/hello_world.lua")

task = client.register_udf(hello_world_udf, "hello.lua")
task.wait_till_completed
```

<!--===============================================================================-->
<hr/>
<!-- drop_udf -->
<a name="drop_udf"></a>

### drop_udf(server_path, options = {})

Drops udf on server.

Parameters:

- `server_path`  - path to udf in server
- `options`:

  - @TODO options policy

Return:

- `true` if dropped succesfuly

Example:

```ruby
client.drop_udf("hello.lua") # => true
```

<!--===============================================================================-->
<hr/>
<!-- list_udf -->
<a name="list_udf"></a>

### list_udf(options = {})
`alias: list_udfs`

List all udf registered on server.

Parameters:

- `options`:

  - @TODO options policy

Return:

- `array` of hashes representing each udf

Example:

```ruby
client.list_udf # => [{:name => "hello.lua", :udf_type => :lua, :hash => 32154001}]
```

<!--===============================================================================-->
<hr/>
<!-- execute_udf -->
<a name="execute_udf"></a>

### execute_udf(key, module_name, func_name, udf_args = [], options = {})

Execute udf on record.
Aerospike reference: http://www.aerospike.com/docs/udf/developing_record_udfs.html

Parameters:

- `key`         - [AerospikeC::Key](key.md) object
- `module_name` - registered module name
- `func_name`   - function name in module to execute
- `udf_args`    - arguments passed to udf
- `options`:

  - `:policy` - [AerospikeC::ApplyPolicy](policy.md#apply_policy) object

Return:

-  `data` returned from udf

Example:
```lua
-- lua/hello_world.lua:
function hello_world(rec)
  return "hello_world"
end
```

```ruby
hello_world_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/hello_world.lua")

task = client.register_udf(hello_world_udf, "hello.lua")
task.wait_till_completed

key = AerospikeC::Key.new("test", "test", "udf_example")
client.execute_udf(key, "hello", "hello_world") # => "hello_world"
```

<!--===============================================================================-->
<hr/>
<!-- scan -->
<a name="scan"></a>

### scan(ns, set, options = {})

Scan records in specified namespace and set.

Aerospike reference: http://www.aerospike.com/docs/guide/scan.html

Parameters:

- `ns`  - namespace to scan
- `set` - set to scan
- `options`:

  - `:priority` - [AerospikeC::ScanTask priority](scan_task.md#priority)

  - @TODO options policy

Return:

- `data` returned from scan

Example:

```ruby
i = 0
5.times do
  key = AerospikeC::Key.new("test", "scan_test", "scan_test_#{i}")
  bins = {"x" => i, "lat" => 34, "lng" => 56#{i}"}

  client.put(key, bins)

  i += 1
end

client.scan("test", "scan_test")
# => [{"x"=>1, "lat" => 34, "lng" => 56}, {"x"=>3, "y"=>"str3"}, {"x"=>2, "y"=>"str2"}, {"x"=>0, "y"=>"str0"}, {"x"=>4, "y"=>"str4"}]
```

<!--===============================================================================-->
<hr/>
<!-- execute_udf_on_scan -->
<a name="execute_udf_on_scan"></a>

### execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
`alias: scan_udf`

Scan records in specified namespace and set and apply udf on each record scanned.

Aerospike reference: http://www.aerospike.com/docs/guide/scan.html

Parameters:

- `ns`          - namespace to scan
- `set`         - set to scan
- `module_name` - registered module name
- `func_name`   - function name in module to execute
- `udf_args`    - arguments passed to udf
- `options`:

  - `:priority` - [AerospikeC::ScanTask priority](scan_task.md#priority)

  - @TODO options policy

Return:

- `data` returned from scan

Example:

```ruby
i = 0
5.times do
  key = AerospikeC::Key.new("test", "scan_test", "scan_test_#{i}")
  bins = {"x" => i}

  client.put(key, bins)

  i += 1
end

client.execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56])
# => [{"x"=>1, "lat" => 34, "lng" => 56}, {"x"=>3, "lat" => 34, "lng" => 56}, {"x"=>2, "lat" => 34, "lng" => 56}, {"x"=>0, "lat" => 34, "lng" => 56}, {"x"=>4, "lat" => 34, "lng" => 56}]
```


<!--===============================================================================-->
<hr/>
<!-- background_execute_udf_on_scan -->
<a name="background_execute_udf_on_scan"></a>

### background_execute_udf_on_scan(ns, set, module_name, func_name, udf_args = [], options = {})
`alias: bg_scan_udf`

Execute udf on scan records in specified namespace and set in background.
Aerospike reference: http://www.aerospike.com/docs/guide/scan.html

Parameters:

- `ns`          - namespace to scan
- `set`         - set to scan
- `module_name` - registered module name
- `func_name`   - function name in module to execute
- `udf_args`    - arguments passed to udf
- `options`:

  - `:priority` - [AerospikeC::ScanTask priority](scan_task.md#priority)

  - `:policy` - [AerospikeC::WritePolicy](policy.md#write_policy) object

Return:

- [AerospikeC::ScanTask](scan_task.md) object

Example:
```ruby
scan_task = client.background_execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56])

loop do
  scan_task.check_status

  # (...)

  break if scan_task.completed?
end

```

<!--===============================================================================-->
<hr/>
<!-- query -->
<a name="query"></a>

### query(query_obj)

Simple query execution.


Parameters:

- `query_obj` - [AerospikeC::Query](query.md) object

Return:

- `data` returned from query

Example:

```ruby
#
# query need indexes
#
task = client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
task.wait_till_completed

#
# build data to operate on
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "query_test", "query#{i}")
  bins = {
    int_bin: i,
  }

  client.put(key, bins)
  i += 1
end

q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 8, 10)

client.query(q_range) # => [{"int_bin" => 8}, {"int_bin" => 9}, {"int_bin" => 10}]
```

<!--===============================================================================-->
<hr/>
<!-- execute_udf_on_query -->
<a name="execute_udf_on_query"></a>

### execute_udf_on_query(query_obj, module_name, func_name, udf_args = []))
`alias: aggregate`

Execute udf on query (aka aggregation).

Aerospike reference:
- http://www.aerospike.com/docs/guide/aggregation.html
- http://www.aerospike.com/docs/udf/developing_stream_udfs.html

Aggragations need knowlage of indexes and udf, see:
- [creating index](#create_index)
- [registering udf](#register_udf)

Parameters:

- `query_obj` - [AerospikeC::Query](query.md) object
- `module_name` - registered module name
- `func_name`   - function name in module to execute
- `udf_args`    - arguments passed to udf

Return:

- `Array` - data returned from query

Example:

```lua
-- lua/aggregate_udf.lua

local function one(rec)
  return 1
end

local function add(a, b)
  return a + b
end

function mycount(stream)
  return stream : map(one) : reduce(add);
end
```

```ruby
#
# aggregations are performed both on server and client
# http://www.aerospike.com/docs/udf/developing_stream_udfs.html
#
lua_path = File.expand_path(File.join(File.dirname(__FILE__), "lua"))
client = AerospikeC::Client.new("127.0.0.1", 3000, {lua_path: lua_path})

#
# register stream udf
#
aggregate_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/aggregate_udf.lua"))
task = client.register_udf(aggregate_udf, "aggregate_udf.lua")
task.wait_till_completed

#
# query need indexes
#
task = client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
task.wait_till_completed

#
# build data to operate on
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "query_test", "query#{i}")
  bins = {
    int_bin: i,
  }

  client.put(key, bins)
  i += 1
end

q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 8, 10)

client.execute_udf_on_query(q_range, "aggregate_udf", "mycount") # => [3]
# alias:
client.aggregate(q_range, "aggregate_udf", "mycount") # => [3]
```

<!--===============================================================================-->
<hr/>
<!-- background_execute_udf_on_query -->
<a name="background_execute_udf_on_query"></a>

### background_execute_udf_on_query(query_obj, module_name, func_name, udf_args = []))
`alias: bg_aggregate`

Execute udf on query in background.

Aerospike reference:
- http://www.aerospike.com/docs/guide/aggregation.html
- http://www.aerospike.com/docs/udf/developing_stream_udfs.html

Aggragations need knowlage of indexes and udf, see:
- [creating index](#create_index)
- [registering udf](#register_udf)

`Records are not returned to the client.`

Parameters:

- `query_obj` - [AerospikeC::Query](query.md) object
- `module_name` - registered module name
- `func_name`   - function name in module to execute
- `udf_args`    - arguments passed to udf

Return:

- [AerospikeC::QueryTask](query_task.md) object

Example:

```lua
-- lua/aggregate_udf.lua

local function one(rec)
  return 1
end

local function add(a, b)
  return a + b
end

function mycount(stream)
  return stream : map(one) : reduce(add);
end
```

```ruby
#
# register stream udf
#
aggregate_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/aggregate_udf.lua"))
task = client.register_udf(aggregate_udf, "aggregate_udf.lua")
task.wait_till_completed

#
# query need indexes
#
task = client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
task.wait_till_completed

#
# build data to operate on
#
i = 0
100.times do
  key = AerospikeC::Key.new("test", "query_test", "query#{i}")
  bins = {
    int_bin: i,
  }

  client.put(key, bins)
  i += 1
end

q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 8, 10)

query_task = client.background_execute_udf_on_query(q_range, "aggregate_udf", "mycount")
query_task.wait_till_completed
query_task.done? # => true
```


<!--===============================================================================-->
<hr/>
<!-- llist -->
<a name="llist"></a>

### llist(key, bin_name, options = {})

Create new [Aerospike::Llist](llist.md) object

Parameters:

- `key` - [AerospikeC::Key](key.md) object
- `bin_name` - bin name to save llist under
- `options`:
  - `:module` - configurator udf module name: http://www.aerospike.com/docs/guide/ldt_advanced.html

Example:
```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)
key    = AerospikeC::Key.new("test", "test", "llist")
bin_name = "some_bin_name"

llist = client.llist(key, bin_name)
# => #<AerospikeC::Llist:0x00000000ce67b0 @bin_name="some_bin_name", @client=#<AerospikeC::Client:0x00000000ce6e68 @host="127.0.0.1", @port=3000, @last_scan_id=nil, @last_query_id=nil, @ldt_proxy=true, @options={}>, @key=#<AerospikeC::Key:0x00000000ce6b70 @namespace="test", @set="test", @key="llist">>
```
