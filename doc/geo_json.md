# GeoJson Class

Represents Aerospike geo_json in cluster. Note that once initialized geo_json cannot be changed, becaouse of aerospike-c-client geo_json wrapping into ruby. This behaviuor may change in the future.

GeoJson reference: http://geojson.org/geojson-spec.html#introduction

Aerospike reference: http://www.aerospike.com/docs/guide/geospatial.html

<hr/>

With a new [AerospikeC::GeoJson](geo_json.md), you can use any of the methods specified below:

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#json](#json)
  - [#coordinates](#coordinates)
  - [#type](#type)
  - [#point?](#point?)
  - [#polygon?](#polygon?)
  - [#circle?](#circle?)
  - [#lat](#lat)
  - [#lng](#lng)
  - [#radius](#radius)

- [Class Methods](#class_methods)
  - [#point](#point)
  - [#polygon](#polygon)
  - [#circle](#circle)
  - [#polygon_array](#polygon_array)
  - [#polygon_obj](#polygon_obj)
  - [#circle_array](#circle_array)
  - [#circle_obj](#circle_obj)
  - [#circle_point](#circle_point)

<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(json)

Creates instance of AerospikeC::GeoJson class.

Parameters:

- `json` - json value, presumably hash - any other value will be called to_s and then parsed by JSON module

Example:

```ruby
AerospikeC::GeoJson.new({type: "Point", coordinates: [0.0, 0.0]})
AerospikeC::GeoJson.new('{"type":"Point", "coordinates":[0.0, 0.0]}')
# => #<AerospikeC::GeoJson {"type"=>"Point", "coordinates"=>[0.0, 0.0]}>
```

<!--===============================================================================-->
<hr/>
<!-- json -->
<a name="json"></a>

### json

Retruns instance `json value`

Example:

```ruby
geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.json # => {"type":"Point", "coordinates":[0.0, 0.0]}
```

<!--===============================================================================-->
<hr/>
<!-- coordinates -->
<a name="coordinates"></a>

### coordinates

`alias: to_a`

Retruns instance `coordinates`

Example:

```ruby
geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.coordinates # => [0.0, 0.0]
```

<!--===============================================================================-->
<hr/>
<!-- type -->
<a name="type"></a>

### type

Retruns instance geo json `type`

Example:

```ruby
geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.type # => "Point"
```

<!--===============================================================================-->
<hr/>
<!-- point? -->
<a name="point?"></a>

### point?

Check if geo json is a Point

Example:

```ruby
geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.point? # => true

circle = AerospikeC::GeoJson.circle([1, 5], 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
circle.point? # => false
```

<!--===============================================================================-->
<hr/>
<!-- polygon? -->
<a name="polygon?"></a>

### polygon?

Check if geo json is a Polygon

Example:

```ruby
polygon_points = [
  AerospikeC::GeoJson.point(1, 2),
  [2, 3],
  AerospikeC::GeoJson.point(4, 6),
  AerospikeC::GeoJson.point(1, 2),
]

polygon = AerospikeC::GeoJson.polygon(polygon_points)
# => #<AerospikeC::GeoJson {"type"=>"Polygon", "coordinates"=>[[[1, 2], [2, 3], [4, 6], [1, 2]]]}>
polygon.polygon? # => true

geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.polygon? # => false
```

<!--===============================================================================-->
<hr/>
<!-- circle? -->
<a name="circle?"></a>

### circle?

Check if geo json is an AeroCircle

Example:

```ruby
circle = AerospikeC::GeoJson.circle([1, 5], 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
circle.circle? # => true

geo = AerospikeC::GeoJson.new({type:"Point", coordinates: [0.0, 0.0]})
geo.circle? # => false
```

<!--===============================================================================-->
<hr/>
<!-- lat -->
<a name="lat"></a>

### lat

Latitude value for point or circle

Example:

```ruby
point = AerospikeC::GeoJson.point([11.23234, 55.53453])
# => #<AerospikeC::GeoJson {"type"=>"Point", "coordinates"=>[11.23234, 55.53453]}>

point.lat # => 55.53453

circle = AerospikeC::GeoJson.circle([11.23234, 55.53453], 1000)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[11.23234, 55.53453], 1000]}>

circle.lat # => 55.53453
```

<!--===============================================================================-->
<hr/>
<!-- lng -->
<a name="lng"></a>

### lng

Longitude value for point or circle

Example:

```ruby
point = AerospikeC::GeoJson.point([11.23234, 55.53453])
# => #<AerospikeC::GeoJson {"type"=>"Point", "coordinates"=>[11.23234, 55.53453]}>

point.lng # => 11.23234

circle = AerospikeC::GeoJson.circle([11.23234, 55.53453], 1000)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[11.23234, 55.53453], 1000]}>

circle.lng # => 11.23234
```

<!--===============================================================================-->
<hr/>
<!-- lng -->
<a name="lng"></a>

### radius

Radius of circle in meters

Example:

```ruby
circle = AerospikeC::GeoJson.circle([11.23234, 55.53453], 1000)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[11.23234, 55.53453], 1000]}>

circle.radius # => 1000
```


<hr/>

<a name="class_methods"></a>
## Class Methods

<!--===============================================================================-->
<hr/>
<!-- point -->
<a name="point"></a>

### point(x, y)

Creates new [AerospikeC::GeoJson](geo_json.md) Point

Parameters:

- `x` - x coordinate
- `y` - y coordinate

Example:

```ruby
AerospikeC::GeoJson.point(12, 34.54)
# => #<AerospikeC::GeoJson {"type"=>"Point", "coordinates"=>[12, 34.54]}>
```

<!--===============================================================================-->
<hr/>
<!-- polygon -->
<a name="polygon"></a>

### polygon(coordinates)

Creates new [AerospikeC::GeoJson](geo_json.md) Polygon

Parameters:

- `coordinates` - coordinates array, either AerospikeC::GeoJson](geo_json.md) Point or arrays [x, y]

Example:

```ruby
polygon_points = [
  AerospikeC::GeoJson.point(1, 2),
  [2, 3],
  AerospikeC::GeoJson.point(4, 6),
  [1, 2],
]

polygon = AerospikeC::GeoJson.polygon(polygon_points)
# => #<AerospikeC::GeoJson {"type"=>"Polygon", "coordinates"=>[[[1, 2], [2, 3], [4, 6], [1, 2]]]}>
```

<!--===============================================================================-->
<hr/>
<!-- circle -->
<a name="circle"></a>

### circle(point, radius)

Creates new [AerospikeC::GeoJson](geo_json.md) AeroCircle

Parameters:

- `point` - point array or [AerospikeC::GeoJson](geo_json.md) Point
- `radius` - radius of AeroCircle

Example:

```ruby
AerospikeC::GeoJson.circle([1, 5], 5)
AerospikeC::GeoJson.circle(AerospikeC::GeoJson.point(1, 5), 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
```

<!--===============================================================================-->
<hr/>
<!-- polygon_array -->
<a name="polygon_array"></a>

### polygon_array(cords)

Creates new [AerospikeC::GeoJson](geo_json.md) Polygon from array values.
This method does not perfom checking cords, so its slightly faster than [AerospikeC::GeoJson.polygon](#polygon) but passing [AerospikeC::GeoJson](geo_json.md) Point will throw an error

Parameters:

- `cords` - array of points arrays

Example:

```ruby
polygon_points = [
  [1, 2],
  [2, 3],
  [4, 6],
  [1, 2],
]

polygon = AerospikeC::GeoJson.polygon_array(polygon_points)
# => #<AerospikeC::GeoJson {"type"=>"Polygon", "coordinates"=>[[[1, 2], [2, 3], [4, 6], [1, 2]]]}>
```

<!--===============================================================================-->
<hr/>
<!-- polygon_obj -->
<a name="polygon_obj"></a>

### polygon_obj(cords)

Creates new [AerospikeC::GeoJson](geo_json.md) Polygon from [AerospikeC::GeoJson](geo_json.md) Point values.
This method does not perfom checking cords, so its slightly faster than [AerospikeC::GeoJson.polygon](#polygon) but passing array will throw an error

Parameters:

- `cords` - array of [AerospikeC::GeoJson](geo_json.md) Points

Example:

```ruby
polygon_points = [
  AerospikeC::GeoJson.point(1, 2),
  AerospikeC::GeoJson.point(2 ,3),
  AerospikeC::GeoJson.point(4, 6),
  AerospikeC::GeoJson.point(1, 2),
]

polygon = AerospikeC::GeoJson.polygon_obj(polygon_points)
# => #<AerospikeC::GeoJson {"type"=>"Polygon", "coordinates"=>[[[1, 2], [2, 3], [4, 6], [1, 2]]]}>
```

<!--===============================================================================-->
<hr/>
<!-- circle_array -->
<a name="circle_array"></a>

### circle_array(cords, radius)

Creates new [AerospikeC::GeoJson](geo_json.md) AeroCircle from array values.
This method does not perfom checking cords, so its slightly faster than [AerospikeC::GeoJson.circle](#circle) but passing [AerospikeC::GeoJson](geo_json.md) Point will throw an error

Parameters:

- `cords` - array of points arrays
- `radius` - Circle radius

Example:

```ruby
AerospikeC::GeoJson.circle_array([1, 5], 5)
AerospikeC::GeoJson.circle(AerospikeC::GeoJson.point(1, 5), 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
```

<!--===============================================================================-->
<hr/>
<!-- circle_obj -->
<a name="circle_obj"></a>

### circle_obj(cords, radius)

Creates new [AerospikeC::GeoJson](geo_json.md) AeroCircle from array values.
This method does not perfom checking cords, so its slightly faster than [AerospikeC::GeoJson.circle](#circle) but passing array will throw an error

Parameters:

- `cords` - [AerospikeC::GeoJson](geo_json.md) Point
- `radius` - Circle radius

Example:

```ruby
AerospikeC::GeoJson.circle_obj(AerospikeC::GeoJson.point(1, 5), 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
```

<!--===============================================================================-->
<hr/>
<!-- circle_point -->
<a name="circle_point"></a>

### circle_point(x, y, radius)

Creates new [AerospikeC::GeoJson](geo_json.md) AeroCircle from x,y coordinates

Parameters:

- `x` - x coordinate
- `y` - y coordinate
- `radius` - Circle radius

Example:

```ruby
AerospikeC::GeoJson.circle_point(1, 5, 5)
# => #<AerospikeC::GeoJson {"type"=>"AeroCircle", "coordinates"=>[[1, 5], 5]}>
```
