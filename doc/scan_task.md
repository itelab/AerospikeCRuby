# ScanTask Class

ScanTask class is used to hold information about progress of scan operations performed on server. Instances of it are returned by client scan background operations.

```ruby
scan_task = client.background_execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56])
scan_task.inspect
# => #<AerospikeC::ScanTask:0x0000000197be30 @scan_id=1820142628934228794, @client=#<AerospikeC::Client:0x000000019c0148 @host="127.0.0.1", @port=3000, @last_scan_id=1820142628934228794, @last_query_id=nil>, @status="AS_SCAN_STATUS_INPROGRESS", @status_id=1, @progress=9>
```

*Do not create instances of it by yourself (unless you want to just have fun).*

With a new udf task, you can use any of the methods specified below:

- [Methods](#methods)
  - [#check_status](#check_status)
  - [#wait_till_completed](#wait_till_completed)
  - [#name](#name)
  - [#scan_id](#scan_id)
  - [#progress](#progress)
  - [#status](#status)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- check_status -->
<a name="check_status"></a>

### check_status

Execute scan info on server and returns response, perform this method to get newest info about scan.

Example:

```ruby
scan_task = client.background_execute_udf_on_scan("test", "scan_test", "scan_udf", "add_cords", [34, 56])
scan_task.check_status # => {"status"=>"AS_SCAN_STATUS_INPROGRESS", "status_id"=>1, "progress"=>2}
```

<!--===============================================================================-->
<hr/>
<!-- completed? -->
<a name="completed?"></a>

### completed?

- `true` if status is AS_SCAN_STATUS_COMPLETED
- `false` otherwise


<!--===============================================================================-->
<hr/>
<!-- scan_id -->
<a name="scan_id"></a>

### scan_id

Retruns scan id


<!--===============================================================================-->
<hr/>
<!-- progress -->
<a name="progress"></a>

### progress

Retruns progress as integer

<!--===============================================================================-->
<hr/>
<!-- status -->
<a name="status"></a>

### status

Returns current object status.

Possible statuses (self explanatory):

- `AS_SCAN_STATUS_UNDEF`
- `AS_SCAN_STATUS_INPROGRESS`
- `AS_SCAN_STATUS_ABORTED`
- `AS_SCAN_STATUS_COMPLETED`