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

- [Client class](client.md)
- [Key class](key.md)
- [Record class](record.md)
- [Operation class](operation.md)
- [Query class](query.md)
- [IndexTask class](index_task.md)
- [UdfTask class](udf_task.md)
- [ScanTask class](scan_task.md)
- [QueryTask class](query_task.md)
- [Policy classes](policy.md)
- [Llist class](llist.md)