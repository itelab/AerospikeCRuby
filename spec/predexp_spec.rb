require 'spec_helper'

describe AerospikeC::PredExp do
  before(:each) do
    @predexp = AerospikeC::PredExp.new
  end

  it "is AerospikeC::PredExp" do
    expect(@predexp).to be_kind_of(AerospikeC::PredExp)
  end
end
