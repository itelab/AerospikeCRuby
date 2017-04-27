require 'spec_helper'

describe AerospikeC::PredExp do
  before(:each) do
    @predexp = AerospikeC::PredExp.new
  end

  it "is AerospikeC::PredExp" do
    expect(@predexp).to be_kind_of(AerospikeC::PredExp)
  end

  context "[]" do
    it "returns AerospikeC::PredExpNode" do
      expect(@predexp["foo"]).to be_kind_of(AerospikeC::PredExpNode)
    end

    it "can chain methods" do
      expect(@predexp["foo"].eq(5).node).to include(
        bin: "foo",
        true: true,
        filters: {
          filter: :eq,
          value: 5
        },
        and: [],
        or: []
      )
    end
  end

  context "record" do
    it "returns AerospikeC::PredExpNode" do
      expect(@predexp.record).to be_kind_of(AerospikeC::PredExpNode)
    end

    it "can chain methods" do
      expect(@predexp.record.last_update.eq(5).node).to include(
        bin: :record,
        true: true,
        filters: {
          rec_method: :last_update,
          filter: :eq,
          value: 5
        },
        and: [],
        or: []
      )
    end
  end

  context "#where" do
    before(:each) do
      @node = @predexp["foo"].eq(1)
    end

    it "sets predicates" do
      expect(@predexp.where(@node).predexp).to eq(@node.node)
    end

    it "returns error" do
      expect{@predexp.where({foo: :bar})}.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#and" do
    before(:each) do
      @node = @predexp["foo"].eq(1)
      @node_2 = @predexp["bar"].not_eq(2)
    end


    it "sets error when where has not been used first" do
      expect{@predexp.and(@node)}.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end

    it "sets predicates" do
      sol_hash = @node.node
      sol_hash[:and] << @node_2.node
      expect(@predexp.where(@node).and(@node_2).predexp).to eq(sol_hash)
    end

    it "returns error" do
      expect{@predexp.where(@node).and({foo: :bar})}.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#or" do
    before(:each) do
      @node = @predexp["foo"].eq(1)
      @node_2 = @predexp["bar"].not_eq(2)
    end


    it "sets error when where has not been used first" do
      expect{@predexp.or(@node)}.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end

    it "sets predicates" do
      sol_hash = @node.node
      sol_hash[:or] << @node_2.node
      expect(@predexp.where(@node).or(@node_2).predexp).to eq(sol_hash)
    end

    it "returns error" do
      expect{@predexp.where(@node).or({foo: :bar})}.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end
end
