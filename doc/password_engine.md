# PasswordEngine Class

Bcrypt password hashing engine.

- [Class Methods](#class_methods)
  - [#create](#create)
  - [#gen_salt](#gen_salt)
  - [#hash_secret](#hash_secret)

<a name="class_methods"></a>
## Class Methods

<!--===============================================================================-->
<!-- create -->
<a name="create"></a>

### create(pass)

Creates new password hash

Parameters:

- `pass` - password to hash

Example:

```ruby
AerospikeC::PasswordEngine.create("mypassword")
```

<!--===============================================================================-->
<hr/>
<!-- gen_salt -->
<a name="gen_salt"></a>

### gen_salt

Generate salt

Example:

```ruby
AerospikeC::PasswordEngine.gen_salt
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
AerospikeC::PasswordEngine.hash_secret("mypassword", AerospikeC::PasswordEngine.gen_salt)
```