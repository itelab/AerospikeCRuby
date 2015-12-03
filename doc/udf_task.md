# UdfTask Class

UdfTask class is used to hold information about asynchronus udf operations performed on server. Instances of it are returned by client udf operations.

```ruby
client = AerospikeC::Client.new("127.0.0.1", 3000)

hello_world_udf = File.expand_path(File.join(File.dirname(__FILE__), "lua/hello_world.lua")

task = client.register_udf(hello_world_udf, "hello.lua")
task.wait_till_completed
```

*Do not create instances of it by yourself (unless you want to just have fun).*

With a new udf task, you can use any of the methods specified below:

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
