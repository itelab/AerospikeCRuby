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
end