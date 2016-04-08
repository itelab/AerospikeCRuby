# AerospikeCRuby
### master version - 0.2.3

AerospikeCRuby is an aerospike library using aerospike-c-client wrapped into ruby. Responses does not build any objects and return pure ruby hashes for speed.

1. [Overwiew](#overwiew)
2. [Installation](#instalation)
3. [Basic usage](#basic_usage)
4. [Build information](#build_info)
5. [Api documentation](doc)
6. [Examples](example)
7. [License](#license)

<!--===============================================================================-->
<br/><hr/>
<a name="overwiew"></a>

### Overwiew:

#### AerospikeC::Client class provides support for:
- reads, writes, deletes
- batch reads
- asinfo
- operations
- indexes
- udfs
- scans (including background tasks)
- queries (aka aggregations, including background tasks)
- large lists (ldt)

#### Bin info:
- Bin names can be anything that responds to `to_s`, except `nil`
- Values:
  - Integers
  - Floats
  - Strings
  - Arrays
  - Hashes
  - Symbols (mapped to string)
  - [GeoJson](geo_json.md)
- Nested values like `[[1, 2], {"h1" => {"h2" => 2}}]` possible

#### Major TODOs:
- Bytes support (and object values mapping)
- Response types (hashes or expanded objects)
- More Policies



<!--===============================================================================-->
<br/><hr/>
<a name="instalation"></a>

### Installation:

Build aerospike-c-client from source:

https://github.com/aerospike/aerospike-client-c

You will need also module/common headers for usage.

Add to your Gemfile:

```ruby
gem 'aerospike_c_ruby'
```



<!--===============================================================================-->
<br/><hr/>
<a name="basic_usage"></a>

### Basic usage:

```ruby
require "aerospike_c_ruby"

#
# Create client object for conneting:
#
client = AerospikeC::Client.new("127.0.0.1", 3000)

#
# create key
#
key = AerospikeC::Key.new("test", "test", "test")

#
# chose bins to save
#
bins = {
  :bin_int => 15, # symbol bin names also supported
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

#
# save bins, with given key in cluster
#
client.put(key, bins)

#
# get bins from the cluster
#
rec_bins = client.get(key)

#
# update
#
rec_bins["bin_int"] = nil # bin names returned as string so you can change them freely
rec_bins["bin_string"] = "new_string"
client.put(key, rec_bins)

#
# remove key in the cluster
#
client.delete(key)
```



<!--===============================================================================-->
<br/><hr/>
<a name="build_info"></a>

### Build information:

* aerospike          - 3.7.1
* aerospike-c-client - 4.0.4
* ruby version       - 2.2.3



<!--===============================================================================-->
<br/><hr/>
<a name="license"></a>

```
Copyright (c) 2015-2016 Itelab (www.itelab.eu)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```