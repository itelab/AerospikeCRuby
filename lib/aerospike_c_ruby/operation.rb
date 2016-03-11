class AerospikeC::Operation
  attr_accessor :operations
  attr_reader :ttl

  #-----------------------------------------------------------------------------
  ##@brief      init
  ##
  ##@param      ops      operations to init
  ##@param      options  options for operations obj
  ##
  ##@return     self
  ##
  def initialize(ops = [], options = {})
    @operations = ops

    @ttl = options[:ttl]

    if @ttl.nil?
      @ttl = 0
    else
      if !@ttl.is_a?(Fixnum)
        raise AerospikeC::OptionError.new(must_be_int_msg)
      end
    end
  end


  #-----------------------------------------------------------------------------
  ##@brief      setting ttl
  ##
  ##@param      val   ttl to set, integer only
  ##
  ##@return     ttl
  ##
  def ttl=(val)
    if val.is_a?(Fixnum)
      raise AerospikeC::OptionError.new(must_be_int_msg)
    end

    @ttl = val
  end


  #-----------------------------------------------------------------------------
  ##@brief      increment integer bin
  ##
  ##@param      bin,  bin name
  ##@param      val,  value to increment by
  ##
  ##@return     { description_of_the_return_value }
  ##
  def increment!(bin, val)
    @operations << { operation: :increment, bin: bin_name(bin), value: val }

    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      touch operation
  ##
  ##@return     self
  ##
  def touch!
    @operations << { operation: :touch }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      append string
  ##
  ##@param      bin,  bin name
  ##@param      val,  value to append
  ##
  ##@return     self
  ##
  def append!(bin, val)
    @operations << { operation: :append, bin: bin_name(bin), value: val }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      prepend string
  ##
  ##@param      bin,  bin name
  ##@param      val,  value to prepend
  ##
  ##@return     self
  ##
  def prepend!(bin, val)
    @operations << { operation: :prepend, bin: bin_name(bin), value: val }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      read given bin
  ##
  ##@param      bin   bin name
  ##
  ##@return     self
  ##
  def read!(bin)
    @operations << { operation: :read, bin: bin_name(bin) }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      write given bin with value
  ##
  ##@param      bin,  bin name
  ##@param      val,  value to write
  ##
  ##@return     self
  ##
  def write!(bin, val)
    @operations << { operation: :write, bin: bin_name(bin), value: val }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      operations count
  ##
  ##
  def length
    @operations.length
  end

  alias_method :size, :length
  alias_method :count, :length


  #-----------------------------------------------------------------------------
  ##@brief      append list with value
  ##
  ##@param      bin,  bin name
  ##@param      val,  value to append
  ##
  ##@return     self
  ##
  def list_append!(bin, val)
    @operations << { operation: :list_append, bin: bin_name(bin), value: val }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      set value in list at given index
  ##
  ##@param      bin    bin name
  ##@param      val    value to set
  ##@param      index  index to set at
  ##
  ##@return     self
  ##
  def list_set!(bin, val, index)
    @operations << { operation: :list_set, bin: bin_name(bin), value: val, at: index }
    self
  end


  #----------------------------------------------------------------------------
  ## @brief      remove values not in range <index, count>
  ##
  ## @param      bin    bin name
  ## @param      index  list index
  ## @param      count  range from index
  ##
  ## @return     self
  ##
  def list_trim!(bin, index, count)
    @operations << { operation: :list_trim, bin: bin_name(bin), value: index, count: count }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      clear list
  ##
  ##@param      bin   bin name
  ##
  ##@return     self
  ##
  def list_clear!(bin)
    @operations << { operation: :list_clear, bin: bin_name(bin) }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      pop from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_pop!(bin, index)
    @operations << { operation: :list_pop, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      pop_range from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##@param      count, count from index
  ##
  ##@return     self
  ##
  def list_pop_range!(bin, index, count)
    @operations << { operation: :list_pop_range, bin: bin_name(bin), value: index, count: count }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      remove range from list starting at index
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_pop_range_from!(bin, index)
    @operations << { operation: :list_pop_range_from, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      remove from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_remove!(bin, index)
    @operations << { operation: :list_remove, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      remove_range from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##@param      count, count from index
  ##
  ##@return     self
  ##
  def list_remove_range!(bin, index, count)
    @operations << { operation: :list_remove_range, bin: bin_name(bin), value: index, count: count }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      remove range from list starting at index
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_remove_range_from!(bin, index)
    @operations << { operation: :list_remove_range_from, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      get from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_get!(bin, index)
    @operations << { operation: :list_get, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      get_range from list
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##@param      count, count from index
  ##
  ##@return     self
  ##
  def list_get_range!(bin, index, count)
    @operations << { operation: :list_get_range, bin: bin_name(bin), value: index, count: count }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      get range from list starting at index
  ##
  ##@param      bin,  bin name
  ##@param      index,  index of list
  ##
  ##@return     self
  ##
  def list_get_range_from!(bin, index)
    @operations << { operation: :list_get_range_from, bin: bin_name(bin), value: index }
    self
  end


  #-----------------------------------------------------------------------------
  ##@brief      get list size
  ##
  ##@param      bin   bin name
  ##
  ##@return     self
  ##
  def list_size!(bin)
    @operations << { operation: :list_size, bin: bin_name(bin) }
  end


  #----------------------------------------------------------------------------
  ##                            private
  ##
  private
    def must_be_int_msg
      "[AerospikeC::Operation][initialize] ttl must be an integer"
    end

    def bin_name(bin)
      if bin.is_a?(String)
        bin
      else
        bin.to_s
      end
    end
end