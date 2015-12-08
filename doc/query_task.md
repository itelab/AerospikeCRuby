# QueryTask Class

QueryTask class is used to hold information about asynchronus query operations performed on server.
Instances of it are returned by client query operations.
Aerospike reference: http://www.aerospike.com/docs/guide/query.html

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)

q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 5, 10)
q_range.order_by!("int_bin", :desc)

task = client.create_index("test", "query_test", "int_bin", "test_query_test_int_bin_idx", :numeric)
task.wait_till_completed

query_task = client.background_execute_udf_on_query(q_range, "aggregate_udf", "mycount")
query_task.wait_till_completed
query_task.done? # => true
```

*Do not create instances of it by yourself (unless you want to just have fun).*

With a new query task, you can use any of the methods specified below:

- [Methods](#methods)
  - [#done?](#done?)
  - [#wait_till_completed](#wait_till_completed)
  - [#name](#name)

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

<!--===============================================================================-->
<hr/>
<!-- name -->
<a name="name"></a>

### name

Returns udf `name` that task is performing operation on
