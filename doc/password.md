# Password Class

Subclass of [AerospikeC::PasswordEngine](password_engine.md) for general usage

- [Methods](#methods)
  - [#initialize](#initialize)
  - [#==](#==)

- [Class Methods](#class_methods)
  - [#valid_hash?](#valid_hash?)


<a name="methods"></a>
## Methods

<!--===============================================================================-->
<!-- initialize -->
<a name="initialize"></a>

### initialize(hash)

Creates new [AerospikeC::Password](password.md) instance for given hash

Parameters:

- `hash` - encrypted password hash

Example:

```ruby
AerospikeC::Password.new("$2a$10$GB9j5v99Tl8T2/Qoh7Zzb.TkW5YJDFOLqPgZpxXzCFiHtIXzvkNyy")
```

<!--===============================================================================-->
<hr/>
<!-- == -->
<a name="=="></a>

### ==(pass)

`alias :is_password?`

Compare password against the hash

- `pass` - password

Example:

```ruby
pass = AerospikeC::Password.new(AerospikeC::Password.create("abc"))
pass == "some_password" # => false
pass == "abc" # => true
```

<!--===============================================================================-->
<hr/>
<!-- hash_secret -->
<a name="hash_secret"></a>

### hash_secret(pass, salt)

Creates new password hash for given password and salt

Parameters:

- `pass` - password to hash
- `salt` - salt for hash

Example:

```ruby
PasswordEngine.hash_secret("mypassword", PasswordEngine.gen_salt)
```

<hr/>
<br/>

<a name="class_methods"></a>
## Class Methods

<!--===============================================================================-->
<!-- valid_hash? -->
<a name="valid_hash?"></a>

### valid_hash?(hash)

Check if given hash is a valid hash

- `hash` - encrypted password hash

Example:

```ruby
AerospikeC::Password.valid_hash?("$2a$10$GB9j5v99Tl8T2/Qoh7Zzb.TkW5YJDFOLqPgZpxXzCFiHtIXzvkNyy")
```