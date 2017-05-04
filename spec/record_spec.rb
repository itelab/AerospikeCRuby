require 'spec_helper'

describe AerospikeC::Record do
  before(:all) do
    @client = AerospikeC::Client.new("127.0.0.1", 3000)
  end

  before(:each) do
    @bins = {
      "bin_int" => 1,
      "bin_string" => "hey!",
      "bin_tab" => [rand(1..100), rand(1..100), rand_string(100), [rand(1..200), rand_string(25)], rand(1.2...276.9)],
      "bin_float" => rand(-123.2...123.2),

      "bin_hash" => {
        "hash_int" => rand(1..100),
        "hash_string" => rand_string(1),
        "hash_float" => rand(1.2..ttl_default),
        "hash_nested" => {
          "hash_nested_int" => rand(1..500),
          "hash_nested_sting" => rand_string(rand(1..1000)),
          "hash_nested_tab" => [rand(1..100), rand_string(rand(8..256))],
          "hash_nested_float" => rand(1.2...276.9)
        }
      }
    }

    @rec = AerospikeC::Record.new(@bins)
  end

  it "is AerospikeC::Record" do
    expect(@rec).to be_kind_of(AerospikeC::Record)
  end

  it "#bins returns bins" do
    expect(@rec.bins).to eq(@bins)
  end

  it "#length returns bins length" do
    expect(@rec.length).to eq(@bins.length)
  end

  # it "#bins= set new bins" do
  #   new_bins = {"int" => 1, "str" => "str"}
  #   @rec.bins = new_bins
  #   expect(@rec.bins).to eq(new_bins)

  #   key = AerospikeC::Key.new("test", "test", "test")

  #   @client.put(key, @rec)
  #   expect(@client.get(key)).to eq(new_bins)
  # end

  it "has ttl default 0" do
    expect(@rec.ttl).to eq(0)
  end

  it "#ttl sets new ttl" do
    @rec.ttl = 60
    expect(@rec.ttl).to eq(60)
  end
end
