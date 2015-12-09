require 'spec_helper'

describe AerospikeC::Query do
  before(:each) do
    @query = AerospikeC::Query.new("test", "query_test")
  end

  it "is AerospikeC::Query" do
    expect(@query).to be_kind_of(AerospikeC::Query)
  end

  it "#namespace returns namespace" do
    expect(@query.namespace).to eq("test")
  end

  it "#set returns set" do
    expect(@query.set).to eq("query_test")
  end

  it "#bins returns bins" do
    expect(@query.bins).to eq([])
    @query.bins = ["int_bin"]
    expect(@query.bins).to eq(["int_bin"])
  end

  it "#bins<< add new bin" do
    expect(@query.bins).to eq([])
    @query.bins << "int_bin"
    expect(@query.bins).to eq(["int_bin"])
  end

  it "#eql! add eql filter" do
    @query.eql!("string_bin", "string")
    expect(@query.filter).to eq({:filter_type=>:eql, :bin=>"string_bin", :value=>"string", :type=>:string})
  end

  it "#range! add range filter" do
    @query.range!("int_bin", 5, 10)
    expect(@query.filter).to eq({:filter_type=>:range, :bin=>"int_bin", :min=>5, :max=>10})
  end

  it "can have only one filter" do
    @query.eql!("string_bin", "string")
    expect(@query.filter).to eq({:filter_type=>:eql, :bin=>"string_bin", :value=>"string", :type=>:string})

    @query.range!("int_bin", 5, 10)
    expect(@query.filter).to eq({:filter_type=>:range, :bin=>"int_bin", :min=>5, :max=>10})
  end

  it "#order_by! add :asc order" do
    @query.order_by!("string_bin", :asc)
    expect(@query.order).to eq([{:order=>0, :bin=>"string_bin"}])
  end

  it "#order_by! add :desc order" do
    @query.order_by!("string_bin", :desc)
    expect(@query.order).to eq([{:order=>1, :bin=>"string_bin"}])
  end

  it "can have multiple orders" do
    @query.order_by!("string_bin", :asc)
    @query.order_by!("other_bin", :desc)
    expect(@query.order).to eq([{:order=>0, :bin=>"string_bin"}, {:order=>1, :bin=>"other_bin"}])
  end
end