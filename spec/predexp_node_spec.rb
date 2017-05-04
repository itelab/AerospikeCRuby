require 'spec_helper'

describe AerospikeC::PredExpNode do
  before(:each) do
    @node = AerospikeC::PredExpNode.new("foo")
  end

  it "is AerospikeC::PredExpNode" do
    expect(@node).to be_kind_of(AerospikeC::PredExpNode)
  end

  it "raises error when trying to access the node without setting it first" do
    expect{
      @node.node
    }.to raise_error(AerospikeC::PredExpNode::AttributeError)
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

  context "#and" do
    it "another node" do
      node2 = AerospikeC::PredExpNode.new("bar").eq(3)
      @node = @node.eq(3).and(node2)
      expect(@node.node).to include(
        and: [node2.node]
      )
    end

    it "hash" do
      expect{
        @node.eq(4).and({foo: "bar"})
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "#or" do
    it "another node" do
      node2 = AerospikeC::PredExpNode.new("bar").eq(3)
      @node = @node.eq(3).or(node2)
      expect(@node.node).to include(
        or: [node2.node]
      )
    end

    it "hash" do
      expect{
        @node.eq(4).or({foo: "bar"})
      }.to raise_error(AerospikeC::PredExpNode::AttributeError)
    end
  end

  context "array" do
    it "and" do
      @node = @node.list_and.eq(5)
      expect(@node.node[:filters]).to include(
        collection: :array,
        collection_pred: :and,
        value: 5
      )
    end

    it "or" do
      @node = @node.list_or.eq(5)
      expect(@node.node[:filters]).to include(
        collection: :array,
        collection_pred: :or,
        value: 5
      )
    end
  end

  context "mapkey/hash" do
    context "key" do
      it "and" do
        @node = @node.map_key_and.eq(5)
        expect(@node.node[:filters]).to include(
          collection: :mapkey,
          collection_pred: :and,
          value: 5
        )
      end

      it "or" do
        @node = @node.map_key_or.eq(5)
        expect(@node.node[:filters]).to include(
          collection: :mapkey,
          collection_pred: :or,
          value: 5
        )
      end
    end

    context "value" do
      it "and" do
        @node = @node.map_val_and.eq(5)
        expect(@node.node[:filters]).to include(
          collection: :mapval,
          collection_pred: :and,
          value: 5
        )
      end

      it "or" do
        @node = @node.map_val_or.eq(5)
        expect(@node.node[:filters]).to include(
          collection: :mapval,
          collection_pred: :or,
          value: 5
        )
      end
    end
  end
end
