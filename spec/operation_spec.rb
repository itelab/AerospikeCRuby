require 'spec_helper'

describe AerospikeC::Operation do
  before(:each) do
    @op = AerospikeC::Operation.new
  end

  it "is AerospikeC::Operation" do
    expect(@op).to be_kind_of(AerospikeC::Operation)
  end

  it "has ttl defaul 0" do
    expect(@op.ttl).to eq(0)
  end

  it "has defaul operations = []" do
    expect(@op.operations).to eq([])
  end

  it "#ttl= sets new ttl" do
    expect(@op.ttl).to eq(0)
    @op.ttl = 10
    expect(@op.ttl).to eq(10)
  end

  it "#increment! adds increment operation" do
    @op.increment!("int", 1)
    expect(@op.operations).to eq([{:operation=>:increment, :bin=>"int", :value=>1}])
  end

  it "#increment! returns self" do
    expect(@op.increment!("int", 1)).to eq(@op)
  end

  it "#touch! adds touch operation" do
    @op.touch!
    expect(@op.operations).to eq([{:operation=>:touch}])
  end

  it "#touch! returns self" do
    expect(@op.touch!).to eq(@op)
  end

  it "#append! adds append operation" do
    @op.append!("str_bin", "append")
    expect(@op.operations).to eq([{:operation=>:append, :bin=>"str_bin", :value=>"append"}])
  end

  it "#append! returns self" do
    expect(@op.append!("str_bin", "append")).to eq(@op)
  end

  it "#prepend! adds prepend operation" do
    @op.prepend!("str_bin", "prepend")
    expect(@op.operations).to eq([{:operation=>:prepend, :bin=>"str_bin", :value=>"prepend"}])
  end

  it "#prepend! returns self" do
    expect(@op.prepend!("str_bin", "prepend")).to eq(@op)
  end

  it "#read! adds read operation" do
    @op.read!("str_bin")
    expect(@op.operations).to eq([{:operation=>:read, :bin=>"str_bin"}])
  end

  it "#read! returns self" do
    expect(@op.read!("str_bin")).to eq(@op)
  end

  it "#write! adds write operation" do
    @op.write!("str_bin", "write")
    expect(@op.operations).to eq([{:operation=>:write, :bin=>"str_bin", :value=>"write"}])
  end

  it "#write! returns self" do
    expect(@op.write!("str_bin", "write")).to eq(@op)
  end

  it "<< adds operation" do
    @op << {:operation => :touch}
    expect(@op.operations).to eq([{:operation=>:touch}])
  end

  it "<< returns self" do
    expect(@op << {:operation => :touch}).to eq(@op)
  end
end