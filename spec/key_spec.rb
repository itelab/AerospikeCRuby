require 'spec_helper'

describe AerospikeC::Key do
  it "is a AerospikeC::Key" do
    key = AerospikeC::Key.new("test", "test", "test")
    expect(key).to be_kind_of(AerospikeC::Key)
  end

  it "#key returns key" do
    k = AerospikeC::Key.new("test", "test", "test2")
    expect(k.key).to eq("test2")
  end

  it "#set returns set" do
    k = AerospikeC::Key.new("test", "test2", "test")
    expect(k.set).to eq("test2")
  end

  it "#namespace returns namespace" do
    k = AerospikeC::Key.new("test2", "test", "test")
    expect(k.namespace).to eq("test2")
  end
end