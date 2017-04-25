require 'spec_helper'

describe AerospikeC::PredExp do
  before(:each) do
    @predexp = AerospikeC::PredExp.new
  end

  it "is AerospikeC::PredExp" do
    expect(@predexp).to be_kind_of(AerospikeC::PredExp)
  end

  context "#eql" do
    it "as string" do
      @predexp.eql("string_bin", "string")
      expect(@predexp.predexp).to eq([{:predexp=>:equal, :bin=>"string_bin", :value=>"string", :type=>:string}])
    end

    it "as integer" do
      @predexp.eql("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:equal, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.eql("array_bin", [])
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#uneql" do
    it "as string" do
      @predexp.uneql("string_bin", "string")
      expect(@predexp.predexp).to eq([{:predexp=>:unequal, :bin=>"string_bin", :value=>"string", :type=>:string}])
    end

    it "as integer" do
      @predexp.uneql("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:unequal, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.eql("array_bin", [])
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#greater" do
    it "as integer" do
      @predexp.greater("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:greater, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.greater("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#greatereq" do
    it "as integer" do
      @predexp.greatereq("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:greatereq, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.greatereq("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#less" do
    it "as integer" do
      @predexp.less("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:less, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.less("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#lesseq" do
    it "as integer" do
      @predexp.lesseq("int_bin", 0)
      expect(@predexp.predexp).to eq([{:predexp=>:lesseq, :bin=>"int_bin", :value=>0, :type=>:numeric}])
    end

    it "raises error" do
      expect{
        @predexp.lesseq("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#within" do
    it "as AerospikeC::GeoJson" do
      point = AerospikeC::GeoJson.point(1,1)
      @predexp.within("geojson_bin", point)
      expect(@predexp.predexp).to eq([{:predexp=>:within, :bin=>"geojson_bin", :value=>point, :type=>:geo_json}])
    end

    it "raises error" do
      expect{
        @predexp.within("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#contains" do
    it "as AerospikeC::GeoJson" do
      point = AerospikeC::GeoJson.point(1,1)
      @predexp.contains("geojson_bin", point)
      expect(@predexp.predexp).to eq([{:predexp=>:contains, :bin=>"geojson_bin", :value=>point, :type=>:geo_json}])
    end

    it "raises error" do
      expect{
        @predexp.contains("string_bin", "string")
      }.to raise_error(AerospikeC::OptionError)
    end
  end


  context "#regexp" do
    it "as String" do
      point = AerospikeC::GeoJson.point(1,1)
      @predexp.regexp("string_bin", "hay")
      expect(@predexp.predexp).to eq([{:predexp=>:regexp, :bin=>"string_bin", :value=>"hay", :type=>:string}])
    end

    it "raises error" do
      expect{
        @predexp.regexp("string_bin", /hay/)
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#expiration_time" do
    it "as valid Symbol" do
      @predexp.expiration_time(:equal, 10)
      expect(@predexp.predexp).to eq([{:predexp=>:record, :record_predexp_type=>:expiration_time, :record_predexp => :equal, :value=>10, :type=>:numeric}])
    end

    context "valid symbols" do
      it "equal" do
        @symbol = :equal
      end

      it "unequal" do
        @symbol = :unequal
      end

      it "greater" do
        @symbol = :greater
      end

      it "greatereq" do
        @symbol = :greatereq
      end

      it "less" do
        @symbol = :less
      end

      it "lesseq" do
        @symbol = :lesseq
      end

      after(:each) do
        @predexp.expiration_time(@symbol, 10)
        expect(@predexp.predexp.last[:record_predexp]).to eq(@symbol)
      end
    end

    it "invalid Symbol" do
      expect{
        @predexp.expiration_time(:eql, 10)
      }.to raise_error(AerospikeC::OptionError)
    end

    it "invalid type" do
      expect{
        @predexp.expiration_time(:equal, "10")
      }.to raise_error(AerospikeC::OptionError)
    end
  end

  context "#last_update" do
    it "as valid Symbol" do
      @predexp.last_update(:equal, 10)
      expect(@predexp.predexp).to eq([{:predexp=>:record, :record_predexp_type=>:last_update, :record_predexp => :equal, :value=>10, :type=>:numeric}])
    end

    context "valid symbols" do
      it "equal" do
        @symbol = :equal
      end

      it "unequal" do
        @symbol = :unequal
      end

      it "greater" do
        @symbol = :greater
      end

      it "greatereq" do
        @symbol = :greatereq
      end

      it "less" do
        @symbol = :less
      end

      it "lesseq" do
        @symbol = :lesseq
      end

      after(:each) do
        @predexp.last_update(@symbol, 10)
        expect(@predexp.predexp.last[:record_predexp]).to eq(@symbol)
      end
    end

    it "invalid Symbol" do
      expect{
        @predexp.last_update(:eql, 10)
      }.to raise_error(AerospikeC::OptionError)
    end

    it "invalid type" do
      expect{
        @predexp.last_update(:equal, "10")
      }.to raise_error(AerospikeC::OptionError)
    end
  end
end
