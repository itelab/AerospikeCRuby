class AerospikeC::Password < AerospikeC::PasswordEngine
  def self.valid_hash?(h)
    h =~ /^\$[0-9a-z]{2}\$[0-9]{2}\$[A-Za-z0-9\.\/]{53}$/
  end

  def initialize(raw_hash)
    if valid_hash?(raw_hash)
      self.replace(raw_hash)
      @version, @cost, @salt, @checksum = split_hash(self)
    else
      raise "AerospikeC::Password, invalid hash"
    end
  end

  def ==(secret)
    super(self.class.hash_secret(secret, @salt))
  end
  alias_method :is_password?, :==

  private
    def valid_hash?(h)
      self.class.valid_hash?(h)
    end


    def split_hash(h)
      _, v, c, mash = h.split('$')
      return v.to_str, c.to_i, h[0, 29].to_str, mash[-31, 31].to_str
    end
end