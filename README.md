# AerospikeCRuby
### master version - 0.1.0

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

#### Bin info:
- Bin names can be anything that responds to `to_s`, except `nil`
- Values:
  - Integers
  - Floats
  - Strings
  - Arrays
  - Hashes
  - Symbols (mapped to string)
- Nested values like `[[1, 2], {"h1" => {"h2" => 2}}]` possible

#### Major TODOs:
- Large Data Types
- Bytes support (and object values mapping)
- Response types (hashes or expanded objects)
- More Policies
- Exceptions



<!--===============================================================================-->
<br/><hr/>
<a name="instalation"></a>

### Installation:

Build aerospike-c-client from source:

https://github.com/aerospike/aerospike-client-c

extension is looking for:
- `/usr/local/lib/libaerospike.so` shared lib
- `/usr/local/include/aerospike` headers folder

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

* aerospike          - 3.6.2
* aerospike-c-client - 3.1.24
* ruby version       - 2.2.2



<!--===============================================================================-->
<br/><hr/>
<a name="license"></a>

```
Copyright (c) 2015 Itelab (www.itelab.eu)

MIT License

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```