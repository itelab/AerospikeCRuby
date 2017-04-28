# PredExp Class

PredExp is a class for usage with [Client #predexp](client.md#predexp) methods.

PredExp allows to apply filters in the query.


```ruby
q_range = AerospikeC::Query.new("test", "query_test")
q_range.range!("int_bin", 5, 10)
predexp = AerospikeC::PredExp.new
q_range.predexp = predexp
```

With a new operation, you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#[]](#ary)
  - [#where](#where)
  - [#and](#and)
  - [#or](#or)
  - [#predexp](#predexp)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize()

Create a new object.

Example:

```ruby
predexp = AerospikeC::PredExp.new
```

<!--===============================================================================-->
<hr/>
<!-- [] -->
<a name="ary"></a>

### \[\](value)

Get a new AeroSpikeC::PredExpNode object with a chosen bin

Parameters:

- `value` - value of bin - String or Symbol

Retrun new AeroSpikeC::PredExpNode object

Example:

```ruby
predexp = AerospikeC::PredExp.new
node = predexp[:int_bin]
node_2 = predexp["str_bin"]
```

<!--===============================================================================-->
<hr/>
<!-- where -->
<a name="where"></a>

### where(val)

Apply database filter from AerospikeC::PredExpNode

Parameters:

- `val` - AeroSpikeC::PredExpNode object

Retrun `self`

Example:

```ruby
predexp = AerospikeC::PredExp.new
node = predexp[:int_bin].eq(2)
predexp.where(node)
```


<!--===============================================================================-->
<hr/>
<!-- and -->
<a name="and"></a>

### and(val)

Add another filter with 'and' predicate

Parameters:

- `val` - AeroSpikeC::PredExpNode object

Retrun `self`

Example:

```ruby
predexp = AerospikeC::PredExp.new
node = predexp[:int_bin].eq(2)
node_2 = predexp[:int_bin].gt(6)
# methods can be chained
predexp.where(node).and(node2)
# node1 && node2
```

<!--===============================================================================-->
<hr/>
<!-- or -->
<a name="or"></a>

### or(val)

Add another filter with 'or' predicate

Parameters:

- `val` - AeroSpikeC::PredExpNode object

Retrun `self`

Example:

```ruby
predexp = AerospikeC::PredExp.new
node = predexp[:int_bin].eq(2)
node_2 = predexp[:int_bin].gt(6)
# methods can be chained
predexp.where(node).or(node2)
# node1 || node2
```


<!--===============================================================================-->
<hr/>
<!-- predexp -->
<a name="predexp"></a>

### predexp

Retruns Hash with currently applied filters

Example:

```ruby
predexp = AerospikeC::PredExp.new
node = predexp[:int_bin].eq(2)
predexp.where(node)

predexp.predexp # => {:bin=>"int_bin", :true=>true, :filters=>{:filter=>:eq, :value=>2}, :and=>[], :or=>[]}

```
