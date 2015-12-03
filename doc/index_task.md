# IndexTask Class

IndexTask class is used to hold information about asynchronus index operations performed on server. Instances of it are returned by client index operations and wrapps aerospike-c-client index_task into ruby.

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)
task = client.create_index("test", "test", "test_bin", "test_test_test_bin_idx", :numeric)
task.wait_till_completed
```

*Do not create instances of it by yourself (unless you want to just have fun).*

With a new index task, you can use any of the methods specified below:

- [Methods](#methods)
  - [#done?](#done?)
  - [#wait_till_completed](#wait_till_completed)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- done? -->
<a name="done?"></a>

### done?

Retruns `true` or `false` depend on task execution state

<!--===============================================================================-->
<hr/>
<!-- wait_till_completed -->
<a name="wait_till_completed"></a>

### wait_till_completed(interval_ms = 0)

Waits for the end of task performed on server.

Parameters:

- `interval_ms` - the polling interval in milliseconds. If zero, 1000 ms is used

Returns

- `true` after task completed
