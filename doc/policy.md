# Policy classes

Policy classes are used to define behavior for database operations.
Note that once initialized policy cannot be changed, becaouse of aerospike-c-client policy wrapping into ruby. This behaviuor may change in the future.
Aerospike reference: http://www.aerospike.com/docs/client/c/best_practices/policies.html

```ruby
write_policy  = AerospikeC::Policy.new(:write, exists: AerospikeC::Policy::AS_POLICY_EXISTS_UPDATE)
write_policy2 = AerospikeC::WritePolicy.new(exists: AerospikeC::Policy::AS_POLICY_EXISTS_UPDATE)
remove_policy = AerospikeC::RemovePolicy.new(gen: AerospikeC::Policy::AS_POLICY_GEN_GT, generation: 2)
```

Policy classes:
- [WritePolicy](#write_policy)
- [ReadPolicy](#read_policy)
- [RemovePolicy](#remove_policy)
- [ApplyPolicy](#apply_policy)
- [QueryPolicy](#query_policy)
- [OperatePolicy](#operate_policy)

[Policy options:](#options)
- [:commit_level](#commit_level)
- [:consistency_level](#consistency_level)
- [:exists](#exists)
- [:gen](#gen)
- [:key](#key)
- [:replica](#replica)
- [:retry](#retry)
- [:timeout](#timeout)
- [:generation](#generation)

<!--===============================================================================-->
<br /><hr />
<!-- write_policy -->
<a name="write_policy"></a>

### WritePolicy

Uses options:
- [:commit_level](#commit_level)
- [:exists](#exists)
- [:gen](#gen)
- [:key](#key)
- [:retry](#retry)
- [:timeout](#timeout)

<!--===============================================================================-->
<hr />
<!-- read_policy -->
<a name="read_policy"></a>

### ReadPolicy

Uses options:
- [:replica](#replica)
- [:consistency_level](#consistency_level)
- [:key](#key)
- [:retry](#retry)
- [:timeout](#timeout)

<!--===============================================================================-->
<hr />
<!-- remove_policy -->
<a name="remove_policy"></a>

### RemovePolicy

Uses options:
- [:commit_level](#commit_level)
- [:gen](#gen)
- [:generation](#generation)
- [:key](#key)
- [:retry](#retry)
- [:timeout](#timeout)

<!--===============================================================================-->
<hr />
<!-- apply_policy -->
<a name="apply_policy"></a>

### ApplyPolicy

Uses options:
- [:commit_level](#commit_level)
- [:key](#key)
- [:timeout](#timeout)

<!--===============================================================================-->
<hr />
<!-- query_policy -->
<a name="query_policy"></a>

### QueryPolicy

Uses options:
- [:timeout](#timeout)

<!--===============================================================================-->
<hr />
<!-- operate_policy -->
<a name="operate_policy"></a>

### OperatePolicy

Uses options:
- [:consistency_level](#consistency_level)
- [:commit_level](#commit_level)
- [:replica](#replica)
- [:gen](#gen)
- [:key](#key)
- [:retry](#retry)
- [:timeout](#timeout)


<!--===============================================================================-->
<br /><hr />
<!-- options  AerospikeC::Policy:: -->
<a name="options"></a>

### Policy options - these are enumerators in c

<br />
<a name="commit_level"></a>
`:commit_level` - Specifies the number of replicas required to be successfully committed before returning success in a write operation to provide the desired consistency guarantee.

- *default:* `AerospikeC::Policy::AS_POLICY_COMMIT_LEVEL_ALL` - return succcess only after successfully committing all replicas
- `AerospikeC::Policy::AS_POLICY_COMMIT_LEVEL_MASTER` - return succcess after successfully committing the master replica

<br />
<br />
<a name="consistency_level"></a>
`:consistency_level` - Specifies the number of replicas to be consulted in a read operation to provide the desired consistency guarantee.

- *default:* `AerospikeC::Policy::AS_POLICY_CONSISTENCY_LEVEL_ONE` - involve a single replica in the operation
- `AerospikeC::Policy::AS_POLICY_CONSISTENCY_LEVEL_ALL` - involve all replicas in the operation

<br />
<br />
<a name="exists"></a>
`:exists` - Specifies the behavior for writing the record depending whether or not it exists.

- *default:* `AerospikeC::Policy::AS_POLICY_EXISTS_IGNORE` - write the record, regardless of existence. (i.e. create or update.)
- `AerospikeC::Policy::AS_POLICY_EXISTS_CREATE` - create a record, ONLY if it doesn't exist
- `AerospikeC::Policy::AS_POLICY_EXISTS_UPDATE` - update a record, ONLY if it exists
- `AerospikeC::Policy::AS_POLICY_EXISTS_REPLACE` - completely replace a record, ONLY if it exists
- `AerospikeC::Policy::AS_POLICY_EXISTS_CREATE_OR_REPLACE` - completely replace a record if it exists, otherwise create it

<br />
<br />
<a name="gen"></a>
`:gen` - Specifies the behavior of record modifications with regard to the generation value.

- *default:* `AerospikeC::Policy::AS_POLICY_GEN_IGNORE` - write a record, regardless of generation
- `AerospikeC::Policy::AS_POLICY_GEN_EQ` - write a record, ONLY if generations are equal
- `AerospikeC::Policy::AS_POLICY_GEN_GT` - write a record, ONLY if local generation is greater-than remote generation

<br />
<br />
<a name="key"></a>
`:key` - Specifies the behavior for whether keys or digests should be sent to the cluster.

- *default:* `AerospikeC::Policy::AS_POLICY_KEY_DIGEST` - this is the recommended mode of operation. This calculates the digest and send the digest to the server. The digest is only calculated on the client, and not on the server
- `AerospikeC::Policy::AS_POLICY_KEY_SEND` - if you want keys to be returned when scanning or querying, the keys must be stored on the server. This policy causes a write operation to store the key. Once a key is stored, the server will keep it - there is no need to use this policy on subsequent updates of the record. If this policy is used on read or delete operations, or on subsequent updates of a record with a stored key, the key sent will be compared with the key stored on the server. A mismatch will cause AEROSPIKE_ERR_RECORD_KEY_MISMATCH to be returned

<br />
<br />
<a name="replica"></a>
`:replica` - Specifies which partition replica to read from.

- *default:* `AerospikeC::Policy::AS_POLICY_REPLICA_MASTER` - read from the partition master replica node
- `AerospikeC::Policy::AS_POLICY_REPLICA_ANY` - read from an unspecified replica node

<br />
<br />
<a name="retry"></a>
`:retry` - Specifies the behavior of failed operations. You can use custom number of retries

- `AerospikeC::Policy::AS_POLICY_RETRY_NONE` - only attempt an operation once
- *default:* `AerospikeC::Policy::AS_POLICY_RETRY_ONCE` - if an operation fails, attempt the operation one more time

<br />
<br />
<a name="timeout"></a>
`:timeout` - Maximum time in milliseconds to wait for the operation to complete. *(default: 1000)*

<br />
<br />
<a name="generation"></a>
`:generation` - The generation of the record. *(default: 0)*