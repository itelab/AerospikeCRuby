require 'spec_helper'

describe AerospikeC::PredExpNode do
  before(:each) do
    @node = AerospikeC::PredExpNode.new("foo")
  end

  it "is AerospikeC::PredExpNode" do
    expect(@node).to be_kind_of(AerospikeC::PredExpNode)
  end

  context "#eq" do
    it "integer" do
      expect(@node.eq(1).node).to include(
        filters: {
          filter: :eq,
          value: 1
        }
      )
    end

    it "string" do
      expect(@node.eq("str").node).to include(
        filters: {
          filter: :eq,
          value: "str"
        }
      )
    end

    it "invalid" do
      point = AerospikeC::GeoJson.point(1,1)
      expect {
        @node.eq(point)
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#not_eq" do
    it "integer" do
      expect(@node.not_eq(1).node).to include(
        filters: {
          filter: :not_eq,
          value: 1
        }
      )
    end

    it "string" do
      expect(@node.not_eq("str").node).to include(
        filters: {
          filter: :not_eq,
          value: "str"
        }
      )
    end

    it "invalid" do
      point = AerospikeC::GeoJson.point(1,1)
      expect {
        @node.not_eq(point)
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#lt" do
    it "integer" do
      expect(@node.lt(1).node).to include(
        filters: {
          filter: :lt,
          value: 1
        }
      )
    end

    it "invalid" do
      expect {
        @node.lt("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#lteq" do
    it "integer" do
      expect(@node.lteq(1).node).to include(
        filters: {
          filter: :lteq,
          value: 1
        }
      )
    end

    it "invalid" do
      expect {
        @node.lteq("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#gt" do
    it "integer" do
      expect(@node.gt(1).node).to include(
        filters: {
          filter: :gt,
          value: 1
        }
      )
    end

    it "invalid" do
      expect {
        @node.gt("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#gteq" do
    it "integer" do
      expect(@node.gteq(1).node).to include(
        filters: {
          filter: :gteq,
          value: 1
        }
      )
    end

    it "invalid" do
      expect {
        @node.gteq("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#within" do
    it "AerospikeC::GeoJson" do
      point = AerospikeC::GeoJson.point(1,1)
      expect(@node.within(point).node).to include(
        filters: {
          filter: :within,
          value: point
        }
      )
    end

    it "invalid" do
      expect {
        @node.within("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#contains" do
    it "AerospikeC::GeoJson" do
      point = AerospikeC::GeoJson.point(1,1)
      expect(@node.contains(point).node).to include(
        filters: {
          filter: :contains,
          value: point
        }
      )
    end

    it "invalid" do
      expect {
        @node.contains("a")
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end
end
