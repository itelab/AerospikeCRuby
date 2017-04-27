# frozen_string_literal: true

module AerospikeC
  class PredExpNode
    class AttributeError < StandardError
      def initialize(val, custom_err = nil)
        @val = val
        @custom_err = custom_err
      end

      def to_s
        return @custom_err unless @custom_err.nil?
        "[AerospikeC] Invalid attribute: #{@val.class}"
      end
    end

    def self.define_comparsion_method(name, classes = nil)
      define_method name do |val|
        check_class(val, classes)
        add_val(__method__, val)
        self
      end
    end

    define_comparsion_method :eq,     [Integer, String]
    define_comparsion_method :not_eq, [Integer, String]
    define_comparsion_method :gt,     [Integer]
    define_comparsion_method :gteq,   [Integer]
    define_comparsion_method :lt,     [Integer]
    define_comparsion_method :lteq,   [Integer]
    define_comparsion_method :regexp, [String]

    def initialize(bin)
      @node = {
        bin: bin,
        true: true,
        filters: {},
        and: [],
        or: []
      }
    end

    def node
      raise AttributeError.new(nil, "Empty node") if @node[:filters] == {}
      @node
    end

    def not
      @node[:true] = false
      self
    end

    def within(val)
      not_record_predicate!
      check_class(val, AerospikeC::GeoJson)
      add_val(__method__, val)
      self
    end

    def contains(val)
      not_record_predicate!
      check_class(val, AerospikeC::GeoJson)
      add_val(__method__, val)
      self
    end

    def and(val)
      check_class(val, AerospikeC::PredExpNode)
      @node[:and] << val.node
      self
    end

    def or(val)
      check_class(val, AerospikeC::PredExpNode)
      @node[:or] << val.node
      self
    end

    def last_update
      raise AttributeError.new(nil, "not a record predicate") unless @node[:bin] == :record
      @node[:filters][:rec_method] = __method__
      self
    end

    def void_time
      raise AttributeError.new(nil, "not a record predicate") unless @node[:bin] == :record
      @node[:filters][:rec_method] = __method__
      self
    end

    def to_h
      node
    end

    private

    attr_writer :node

    def check_class(val, klass)
      return if klass.nil?

      if klass.is_a?(Array)
        is_a = false
        ancestors = val.class.ancestors
        klass.each do |k|
          if ancestors.include?(k)
            is_a = true
            break
          end
        end

        raise(AttributeError, val) unless is_a
      else
        raise(AttributeError, val) unless val.class.ancestors.include?(klass)
      end

    end

    def add_val(mthd, val)
      @node[:filters][:filter] = mthd
      @node[:filters][:value] = val
    end

    def not_record_predicate!
      raise AttributeError.new(nil, "invalid function for record predicate") if @node[:bin] == :record
    end
  end
end

AerospikeC::PredExp.class_eval do
  def [](bin)
    AerospikeC::PredExpNode.new(bin.to_s)
  end

  def where(val)
    raise(AerospikeC::PredExpNode::AttributeError, val) unless val.is_a?(AerospikeC::PredExpNode)
    @predexp = val.node
    self
  end

  def and(val)
    raise(AerospikeC::PredExpNode::AttributeError, val) unless val.is_a?(AerospikeC::PredExpNode)
    if @predexp
      @predexp[:and] << val.node
    else
      raise(AerospikeC::PredExpNode::AttributeError, nil, "use #where first")
    end
    self
  end

  def or(val)
    raise(AerospikeC::PredExpNode::AttributeError, val) unless val.is_a?(AerospikeC::PredExpNode)
    if @predexp
      @predexp[:or] << val.node
    else
      raise(AerospikeC::PredExpNode::AttributeError, nil, "use #where first")
    end
    self
  end

  def record
    AerospikeC::PredExpNode.new(:record)
  end
end
