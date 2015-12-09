require 'spec_helper'

describe AerospikeC::Client do
  def put_default
    @client.put(@key, @bins)
  end

  def ttl_default
    4294967295
  end

  def rand_string(len = 8)
    (0...len).map { (65 + rand(26)).chr }.join
  end

  before(:each) do
    @client = AerospikeC::Client.new("127.0.0.1", 3000)
    @key    = AerospikeC::Key.new("test", "test", "test")

    @bin_int = rand(1..100)
    @bin_string = rand_string

    @bins = {
      "bin_int" => @bin_int,
      "bin_string" => @bin_string,
      "bin_tab" => [rand(1..100), rand(1..100), rand_string(100), [rand(1..200), rand_string(25)]],

      "bin_hash" => {
        "hash_int" => rand(1..100),
        "hash_string" => rand_string(1),
        "hash_nested" => {
          "hash_nested_int" => rand(1..500),
          "hash_nested_sting" => rand_string(rand(1..1000)),
          "hash_nested_tab" => [rand(1..100), rand_string(rand(8..256))]
        }
      }
    }
  end

  after(:each) do
    @client.delete(@key)
  end

  context "#delete" do
    it "with nil" do
      put_default
      bins = @client.get(@key)

      bins["bin_int"] = nil
      @client.put(@key, bins)

      expect(@client.get(@key)["bin_int"]).to eq(nil)
    end

    it "return nil if record not found" do
      put_default
      @client.delete(@key)
      expect( @client.delete(@key) ).to eq(nil)
    end
  end

  context "#get" do
    it "return nil if record not found" do
      key = AerospikeC::Key.new("test", "test", "somekey")
      expect( @client.get(key) ).to eq(nil)
    end

    it "read specific bins" do
      put_default
      expect( @client.get(@key, ["bin_int", "bin_string"]) ).to eq({"bin_int" => @bin_int, "bin_string" => @bin_string})
    end

    it "with_header" do
      put_default
      expect( @client.get(@key, [], with_header: true) ).to eq({"header"=>{"gen"=>1, "expire_in"=>ttl_default}})
    end
  end

  context "#put" do
    before(:each) do
      @test_put_key = AerospikeC::Key.new("test", "test", "test_put")
    end

    after(:each) do
      @client.delete(@test_put_key)
    end

    it "returns true" do
      expect(put_default).to eq(true)
    end

    it "can put with bin name symbol" do
      expect( @client.put(@test_put_key, {:symbin => "symbin"}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"symbin" => "symbin"})
    end

    it "can put nil" do
      expect( @client.put(@test_put_key, {"nilbin" => nil}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq(nil)
    end

    it "can put int" do
      intbin = rand(1..ttl_default)
      expect( @client.put(@test_put_key, {"intbin" => intbin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"intbin" => intbin})
    end

    it "cant put string" do
      string_bin = rand_string
      expect( @client.put(@test_put_key, {"string_bin" => string_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"string_bin" => string_bin})
    end

    it "can put symbol" do
      symbol_bin = :symbol_bin
      expect( @client.put(@test_put_key, {"symbol_bin" => symbol_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"symbol_bin" => symbol_bin.to_s})
    end

    it "can put array" do
      some_ary = [rand(100..ttl_default), rand_string, {"h1" => ttl_default, "h2" => rand_string, "h3" => {"1" => "2"}}]
      array_bin = [
        rand(1..100),
        rand_string,
        some_ary,
        {"h1" => ttl_default, "h2" => rand_string, "h3" => some_ary}
      ]

      expect( @client.put(@test_put_key, {"array_bin" => array_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"array_bin" => array_bin})
    end

    it "can put hash" do
      some_ary = [rand(100..ttl_default), rand_string, {"h1" => ttl_default, "h2" => rand_string, "h3" => {"1" => "2"}}]

      hash_bin = {
        "keyint" => rand(1..100),
        "keystr" => rand_string,
        "key_ary" => some_ary,
        "key_hash" => {"h1" => ttl_default, "h2" => rand_string, "h3" => some_ary}
      }

      expect( @client.put(@test_put_key, {"hash_bin" => hash_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"hash_bin" => hash_bin})
    end

    it "can put float" do
      float_bin = 3.14
      expect( @client.put(@test_put_key, {"float_bin" => float_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({"float_bin" => float_bin})
    end

    it "update" do
      put_default
      bins = @client.get(@key)

      new_val = rand(101..200)
      bins["bin_int"] = new_val
      @client.put(@key, bins)

      expect(@client.get(@key)["bin_int"]).to eq(new_val)
    end
  end

  it "batch_read" do
    keys = []
    bins = []

    i = 0
    10.times do
      key = AerospikeC::Key.new("test", "test", "test#{i}")
      key_bins = { "msg" => "message#{i}" }

      bins << key_bins
      keys << key

      @client.put(key, key_bins)
      i += 1
    end

    expect(@client.batch_get(keys)).to eq(bins)
  end
end