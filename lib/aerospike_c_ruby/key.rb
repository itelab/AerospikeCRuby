AerospikeC::Key.class_eval do

  #----------------------------------------------------------------------------
  ##
  ## dump for marshalling
  ##
  def _dump(level)
    [namespace, set, key].join(":")
  end

  #----------------------------------------------------------------------------
  ##
  ## load for marshalling
  ##
  def self._load(args)
    new(*args.split(':'))
  end


  #----------------------------------------------------------------------------
  ##
  ## Create new key from string
  ##
  ## @param str [String] string ns:set:key
  ## @param options = {} [Hash]
  ##   @option key [Symbol] - :int if key should be integer
  ##
  ## @return [AerospikeC::Key] new key object
  ##
  def self.from_string(str, options = {})
    ns, set, k, = str.split(":")
    k = k.to_i if options[:key] == :int
    AerospikeC::Key.new(ns, set, k)
  end
end