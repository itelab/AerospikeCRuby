Gem::Specification.new do |s|
  s.name        = 'aerospike_c_ruby'
  s.version     = '0.5.1'
  s.date        = '2017-05-04'
  s.summary     = "aerospike_c_ruby"
  s.description = "aerospike_c_ruby"
  s.authors     = ["Adam Stomski", "Arkadiusz Jasinski"]
  s.email       = 'office@itelab.eu'

  s.files       = Dir["{lib}/**/*", "Rakefile", "README.md", 'ext/**/*.c', 'ext/**/*.h']
  s.test_files  = Dir["spec/**/*"]
  s.extensions = %w[ext/aerospike_c_ruby/extconf.rb]

  s.license = 'MIT'
end
