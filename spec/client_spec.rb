require 'spec_helper'

describe AerospikeC::Client do
  before(:all) do
    lua_path = File.expand_path(File.join(File.dirname(__FILE__), '../example/lua'))
    @client = AerospikeC::Client.new('127.0.0.1', 3000, {lua_path: lua_path})
  end

  before(:each) do
    @key    = AerospikeC::Key.new('test', 'test', 'test')

    @bin_int = rand(1..100)
    @bin_string = rand_string

    @bins = {
      'bin_int' => @bin_int,
      'bin_int_minus' => rand(-1000..-20),
      'bin_string' => @bin_string,
      'bin_tab' => [rand(1..100), rand(1..100), rand_string(100), [rand(1..200), rand_string(25)], rand(1.2...276.9)],
      'bin_float' => rand(-123.2...123.2),

      'bin_hash' => {
        'hash_int' => rand(1..100),
        'hash_string' => rand_string(1),
        'hash_float' => rand(1.2..ttl_default),
        'hash_nested' => {
          'hash_nested_int' => rand(1..500),
          'hash_nested_sting' => rand_string(rand(1..1000)),
          'hash_nested_tab' => [rand(1..100), rand_string(rand(8..256))],
          'hash_nested_float' => rand(1.2...276.9)
        }
      }
    }
  end

  after(:each) do
    @client.delete(@key)
  end

  #
  # delete
  #
  context '#delete' do
    it 'with nil' do
      put_default
      bins = @client.get(@key)

      bins['bin_int'] = nil
      @client.put(@key, bins)

      expect(@client.get(@key)['bin_int']).to eq(nil)
    end

    it 'return nil if record not found' do
      put_default
      @client.delete(@key)
      expect( @client.delete(@key) ).to eq(nil)
    end
  end

  #
  # get
  #
  context '#get' do
    it 'return nil if record not found' do
      key = AerospikeC::Key.new('test', 'test', 'somekey')
      expect( @client.get(key) ).to eq(nil)
    end

    it 'reads all bins' do
      put_default
      expect( @client.get(@key)).to eq(@bins)
    end

    it 'read specific bins' do
      put_default
      expect( @client.get(@key, ['bin_int', 'bin_string']) ).to eq({'bin_int' => @bin_int, 'bin_string' => @bin_string})
    end

    it 'read all with specific_bins empty array' do
      put_default
      expect( @client.get(@key, [])).to eq(@bins)
    end

    it 'with_header' do
      put_default
      expect( @client.get(@key, [], with_header: true)['header'] ).to eq({'gen'=>1, 'expire_in'=>ttl_default})
    end
  end

  #
  # put
  #
  context '#put' do
    before(:each) do
      @test_put_key = AerospikeC::Key.new('test', 'test', 'test_put')
    end

    after(:each) do
      @client.delete(@test_put_key)
    end

    it 'returns true' do
      expect(put_default).to eq(true)
    end

    it 'can put with bin name symbol' do
      expect( @client.put(@test_put_key, {:symbin => 'symbin'}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'symbin' => 'symbin'})
    end

    it 'can put nil' do
      @client.put(@test_put_key, {'nilbin' => 1})
      # nil is for deleting bins from the record
      expect( @client.put(@test_put_key, {'nilbin' => nil}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq(nil)
    end

    it 'can put int' do
      intbin = rand(1..ttl_default)
      expect( @client.put(@test_put_key, {'intbin' => intbin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'intbin' => intbin})
    end

    it 'can put string' do
      string_bin = rand_string
      expect( @client.put(@test_put_key, {'string_bin' => string_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'string_bin' => string_bin})
    end

    it 'can put symbol' do
      symbol_bin = :symbol_bin
      expect( @client.put(@test_put_key, {'symbol_bin' => symbol_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'symbol_bin' => symbol_bin.to_s})
    end

    it 'can put array' do
      some_ary = [rand(100..ttl_default), rand_string, {'h1' => ttl_default, 'h2' => rand_string, 'h3' => {'1' => '2'}}]
      array_bin = [
        rand(1..100),
        rand_string,
        some_ary,
        {'h1' => ttl_default, 'h2' => rand_string, 'h3' => some_ary}
      ]

      expect( @client.put(@test_put_key, {'array_bin' => array_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'array_bin' => array_bin})
    end

    it 'can put hash' do
      some_ary = [rand(100..ttl_default), rand_string, {'h1' => ttl_default, 'h2' => rand_string, 'h3' => {'1' => '2'}}]

      hash_bin = {
        'keyint' => rand(1..100),
        'keystr' => rand_string,
        'key_ary' => some_ary,
        'key_hash' => {'h1' => ttl_default, 'h2' => rand_string, 'h3' => some_ary}
      }

      expect( @client.put(@test_put_key, {'hash_bin' => hash_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'hash_bin' => hash_bin})
    end

    it 'can put float' do
      float_bin = rand(1.23..543.12)
      expect( @client.put(@test_put_key, {'float_bin' => float_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'float_bin' => float_bin})

      float_bin = rand(543.12..ttl_default)
      expect( @client.put(@test_put_key, {'float_bin' => float_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'float_bin' => float_bin})

      float_bin = rand(-543.12..-143.12)
      expect( @client.put(@test_put_key, {'float_bin' => float_bin}) ).to eq(true)
      expect( @client.get(@test_put_key) ).to eq({'float_bin' => float_bin})
    end

    it 'can set ttl' do
      expect( @client.put(@test_put_key, @bins, ttl: 5) ).to eq(true)
      expect( @client.get(@test_put_key, [], with_header: true)['header']['expire_in'] ).to eq(5)
    end

    it 'update' do
      put_default
      bins = @client.get(@key)

      new_val = rand(101..200)
      bins['bin_int'] = new_val
      @client.put(@key, bins)

      expect(@client.get(@key)['bin_int']).to eq(new_val)
    end
  end

  #
  # exists?
  #
  context '#exists?' do
    it 'true if exists' do
      put_default
      expect( @client.exists?(@key) ).to eq(true)
    end

    it 'false if not exists' do
      expect( @client.exists?(@key) ).to eq(false)
    end
  end

  #
  # get_header
  #
  context '#get_header' do
    it 'returns header' do
      put_default_ttl(5)
      expect( @client.get_header(@key) ).to eq({'gen' => 1, 'expire_in' => 5})
    end

    it 'nil when not exists' do
      expect( @client.get_header(@key) ).to eq(nil)
    end
  end

  #
  # batch_get
  #
  context '#batch_get' do
    before(:each) do
      @keys = []
      @batch_bins = []

      i = 0
      10.times do
        key = AerospikeC::Key.new('test', 'test', "test#{i}")
        key_bins = {
          'msg' => "message#{i}",
          'int' => rand(1..ttl_default),
          'float' => rand(-128.128..128.128)
        }

        @batch_bins << key_bins
        @keys << key

        @client.put(key, key_bins, ttl: 60)
        i += 1
      end
    end

    after(:each) do
      @keys.each do |k|
        @client.delete(k)
      end
    end

    it 'can read' do
      expect(@client.batch_get(@keys)).to eq(@batch_bins)
    end

    it 'specific bins' do
      bins = []
      @batch_bins.each do |b|
        bins << {'msg' => b['msg']}
      end

      expect(@client.batch_get(@keys, ['msg'])).to eq(bins)
    end

    it 'retruns header, with_header: true' do
      bins = []
      @batch_bins.each do |b|
        bins << {'msg' => b['msg'], 'header' => {'gen' => 1, 'expire_in' => 60}}
      end

      expect(@client.batch_get(@keys, ['msg'], with_header: true)).to eq(bins)
    end
  end

  #
  # touch
  #
  context '#touch' do
    it 'nil when not found record' do
      expect(@client.touch(@key)).to eq(nil)
    end

    it 'refresh ttl', slow: true do
      put_default_ttl(10)
      sleep 1
      expect( @client.get_header(@key)['expire_in'] ).to eq(9)
      @client.touch(@key, ttl: 70)
      expect( @client.get_header(@key)['expire_in'] ).to eq(70)
    end

    it 'increment generation' do
      put_default
      expect( @client.get_header(@key)['gen'] ).to eq(1)
      @client.touch(@key)
      expect( @client.get_header(@key)['gen'] ).to eq(2)
    end
  end

  #
  # operate
  #
  context '#operate' do
    before(:each) do
      @operations = AerospikeC::Operation.new

      @operations.increment!('int', 1).append!('string', 'c').prepend!('string', 'a')
      @operations.write!('new_bin_int', 10).write!('new_bin_str', 'new')

      @operations.read!('int')
      @operations.read!('string')
      @operations.read!('new_bin_int')
      @operations.read!('new_bin_str')

      @operations_bins = {
        'int' => 1,
        'string' => 'b'
      }
    end

    it 'must use AerospikeC::Operation' do
      [
        1,
        'str',
        [1, 'arry'],
        {'hash' => 1},
        AerospikeC::Record.new({'x' => 1}),
        AerospikeC::Key.new('test', 'query_test', 'test'),
        AerospikeC::Query.new('test', 'test')
      ].each do |val|
        begin
          @client.operate(@key, val)
        rescue => e
          expect(e.inspect).to eq('#<AerospikeC::OptionError: [AerospikeC::Client][operate] use AerospikeC::Operation class to perform operations>')
        end
      end
    end

    it 'nil when not found record' do
      ops = AerospikeC::Operation.new
      ops.read!('int')
      expect(@client.operate(@key, ops)).to eq(nil)
    end

    it 'perform operations' do
      @client.put(@key, @operations_bins)
      expect(@client.operate(@key, @operations)).to eq({'int'=>2, 'string'=>'abc', 'new_bin_int'=>10, 'new_bin_str'=>'new'})
    end

    it 'client.operation retruns new AerospikeC::Operation' do
      expect(@client.operation).to be_kind_of(AerospikeC::Operation)
    end
  end

  #
  # indexes
  #
  context 'indexes' do
    before(:each) do
      @task = @client.create_index('test', 'test', 'test_bin', 'test_test_test_bin_idx', :numeric)
    end

    context 'created index' do
      after(:each) do
        @task.wait_till_completed(100)
        @client.drop_index('test', 'test_test_test_bin_idx')
      end

      it 'returns IndexTask', slow: true do
        expect(@task).to be_kind_of(AerospikeC::IndexTask)
      end

      it 'is not done without wait', slow: true do
        expect(@task.done?).to eq(false)
      end

      it 'can wait for done', slow: true do
        expect(@task.wait_till_completed(100)).to eq(true)
        expect(@task.done?).to eq(true)
      end

      it 'creates index', slow: true do
        expect(@task.wait_till_completed(100)).to eq(true)
        expect(@client.list_indexes.map{|ind| ind['indexname']}).to include('test_test_test_bin_idx')
      end
    end

    it 'drops_index', slow: true do
      @task.wait_till_completed(100)
      expect(@client.drop_index('test', 'test_test_test_bin_idx')).to eq(true)
      sleep 0.1
      @indexes = @client.list_indexes
      expect(@indexes.map{|ind| ind['indexname']}).not_to include('test_test_test_bin_idx')
    end
  end

  #
  # asinfo
  #
  context 'asinfo' do
    it 'can use commands' do
      expect(@client.info_cmd('logs')).to include('logs')
      expect(@client.info_cmd('namespaces')).to include('namespaces')
      expect(@client.info_cmd('build')).to include('build')
      expect(@client.info_cmd('node')).to include('node')
      expect(@client.info_cmd('services')).to include('services')
      expect(@client.info_cmd('sindex')).to include('sindex')
      expect(@client.info_cmd('status')).to include('status')
    end

    it 'namespaces' do
      expect(@client.namespaces).to include('test')
    end

    it 'statistic' do
      # @deprecated in version 3.9
      # expect(@client.statistics).to include('aggr_scans_failed')
      # expect(@client.statistics).to include('rw_err_dup_cluster_key')
      # expect(@client.statistics).to include('total-bytes-disk')
      # expect(@client.statistics).to include('udf_read_success')

      expect(@client.statistics).to include('batch_error')
      expect(@client.statistics).to include('cluster_integrity')
    end
  end

  #
  # udf
  #
  context 'udf' do
    before(:each) do
      @hello_world_lua = File.expand_path(File.join(File.dirname(__FILE__), '../example/lua/hello_world.lua'))
    end

    after(:each) do
      @client.drop_udf('hello.lua')
    end

    it 'register udf returns AerospikeC::UdfTask' do
      expect( @client.register_udf(@hello_world_lua, 'hello.lua') ).to be_kind_of(AerospikeC::UdfTask)
    end

    it 'registers udf', slow: true do
      task = @client.register_udf(@hello_world_lua, 'hello.lua')
      task.wait_till_completed(100)

      has_lua = false

      @client.list_udf.each do |udf|
        if udf[:name] == 'hello.lua'
          expect( udf ).to include(:name => 'hello.lua')
          expect( udf ).to include(:udf_type => :lua)
          has_lua = true
        end
      end

      expect(has_lua).to eq(true)
    end

    it 'can execute udf', slow: true do
      task = @client.register_udf(@hello_world_lua, 'hello.lua')
      task.wait_till_completed(100)

      expect( @client.execute_udf(@key, 'hello', 'hello_world') ).to eq('hello_world')
    end
  end

  #
  # query
  #
  context 'query' do
    before(:each) do
      @query_keys = []
      i = 0
      100.times do
        key = AerospikeC::Key.new('test', 'query_test', "query#{i}")
        bins = {
          int_bin: i,
          string_bin: "str#{i}",
          float_bin: i/2.5
        }

        @query_keys << key

        @client.put(key, bins)
        i += 1
      end
    end

    before(:all) do
      aggregate_udf = File.expand_path(File.join(File.dirname(__FILE__), '../example/lua/aggregate_udf.lua'))

      tasks = []
      tasks << @client.register_udf(aggregate_udf, 'aggregate_udf.lua')
      tasks << @client.create_index('test', 'query_test', 'int_bin', 'test_query_test_int_bin_idx', :numeric)
      tasks << @client.create_index('test', 'query_test', 'float_bin', 'test_query_test_float_bin_idx', :numeric)
      tasks << @client.create_index('test', 'query_test', 'string_bin', 'test_query_test_string_bin_idx', :string)

      tasks.each do |task|
        task.wait_till_completed(100)
      end
    end

    after(:each) do
      @query_keys.each {|k| @client.delete(k) }
    end

    after(:all) do
      @client.drop_index('test', 'test_query_test_int_bin_idx')
      @client.drop_index('test', 'test_query_test_float_bin_idx')
      @client.drop_index('test', 'test_query_test_string_bin_idx')

      @client.drop_udf('aggregate_udf.lua')
    end

    it 'need AerospikeC::Query', slow: true do
      [
        1,
        'str',
        [1, 'arry'],
        {'hash' => 1},
        AerospikeC::Record.new({'x' => 1}),
        AerospikeC::Key.new('test', 'query_test', 'test'),
        AerospikeC::Operation.new
      ].each do |val|
        begin
          @client.query(val)
        rescue => e
          expect(e).to be_kind_of(AerospikeC::OptionError)
          expect(e.inspect).to eq('#<AerospikeC::OptionError: [AerospikeC::Client][query] use AerospikeC::Query class to perform queries>')
        end
      end
    end

    it 'queries int', slow: true do
      searched = [
        {'int_bin'=>8, 'string_bin'=>'str8', 'float_bin'=>3.2},
        {'int_bin'=>9, 'string_bin'=>'str9', 'float_bin'=>3.6},
        {'int_bin'=>10, 'string_bin'=>'str10', 'float_bin'=>4.0}
      ]

      q_range = AerospikeC::Query.new('test', 'query_test')
      q_range.range!('int_bin', 8, 10)

      expect(@client.query(q_range)).to eq(searched)
    end


    it 'queries string', slow: true do
      q_eql = AerospikeC::Query.new('test', 'query_test')
      q_eql.eql!('string_bin', 'str8')

      expect(@client.query(q_eql)).to eq([{'int_bin'=>8, 'string_bin'=>'str8', 'float_bin'=>3.2}])
    end

    it 'can aggregate', slow: true do
      q_range = AerospikeC::Query.new('test', 'query_test')
      q_range.range!('int_bin', 5, 15)

      expect(@client.execute_udf_on_query(q_range, 'aggregate_udf', 'mycount')).to eq([11])
      # alias:
      # expect(@client.aggregate(q_range, 'aggregate_udf', 'mycount')).to eq([11])
    end

    it 'background query returns AerospikeC::QueryTask', slow: true do
      q_range = AerospikeC::Query.new('test', 'query_test')
      q_range.range!('int_bin', 5, 15)
      expect(@client.bg_aggregate(q_range, 'aggregate_udf', 'mycount')).to be_kind_of(AerospikeC::QueryTask)
    end

    it 'can wait for for bg query to complete', slow: true do
      q_range = AerospikeC::Query.new('test', 'query_test')
      q_range.range!('int_bin', 5, 15)

      task = @client.bg_aggregate(q_range, 'aggregate_udf', 'mycount')
      expect(task.done?).to eq(false)

      task.wait_till_completed(100)
      expect(task.done?).to eq(true)
    end

    it 'has last_query_id', slow: true do
      q_range = AerospikeC::Query.new('test', 'query_test')
      q_range.range!('int_bin', 5, 15)

      task = @client.bg_aggregate(q_range, 'aggregate_udf', 'mycount')

      expect(@client.last_query_id).to eq(task.query_id)
      task.wait_till_completed(100)
    end

    context 'predexp' do
      let(:query) { AerospikeC::Query.new('test', 'query_test') }
      let(:predexp) { AerospikeC::PredExp.new }

      let(:execute_query) do
        @client.query(query)
      end

      context 'integer' do
        before(:each) do
          query.range!('int_bin', 8, 10)
        end

        it 'eq' do
          predexp.where(predexp['int_bin'].eq(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(1)
          expect(result.last['int_bin']).to eq(9)
        end

        it 'not_eq' do
          predexp.where(predexp['int_bin'].not_eq(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(2)
          expect(result.first['int_bin']).to eq(8)
          expect(result.last['int_bin']).to eq(10)
        end

        it 'lt' do
          predexp.where(predexp['int_bin'].lt(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(1)
          expect(result.last['int_bin']).to eq(8)
        end

        it 'lteq' do
          predexp.where(predexp['int_bin'].lteq(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(2)
          expect(result.first['int_bin']).to eq(8)
          expect(result.last['int_bin']).to eq(9)
        end

        it 'gt' do
          predexp.where(predexp['int_bin'].gt(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(1)
          expect(result.last['int_bin']).to eq(10)
        end

        it 'gteq' do
          predexp.where(predexp['int_bin'].gteq(9))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(2)
          expect(result.first['int_bin']).to eq(9)
          expect(result.last['int_bin']).to eq(10)
        end
      end

      context 'string' do
        before(:each) do
          query.range!('int_bin', 8, 10)
        end

        it 'eq' do
          predexp.where(predexp['string_bin'].eq('str9'))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(1)
          expect(result.last['int_bin']).to eq(9)
        end

        it 'not_eq' do
          predexp.where(predexp['string_bin'].not_eq('str9'))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(2)
          expect(result.first['int_bin']).to eq(8)
          expect(result.last['int_bin']).to eq(10)
        end

        it 'regexp' do
          predexp.where(predexp['string_bin'].regexp('str'))
          query.predexp = predexp
          result = execute_query

          expect(result.length).to eq(3)
          expect(result.first['int_bin']).to eq(8)
          expect(result[1]['int_bin']).to eq(9)
          expect(result.last['int_bin']).to eq(10)
        end
      end

      context 'geo json' do
        let(:circle_in) do
          AerospikeC::GeoJson.circle([20.479373931884766, 53.77722490339079], 1000)
        end

        let(:circle_out) do
          AerospikeC::GeoJson.circle([20.564002990722656, 53.750943916700976], 1000)
        end

        let(:point) do
          AerospikeC::GeoJson.point(20.479373931884766, 53.77722490339079)
        end

        let(:polygon_in) do
          AerospikeC::GeoJson.polygon([
            [
              20.452766418457028,
              53.78746901588889
            ],
            [
              20.45207977294922,
              53.754800844049306
            ],
            [
              20.516624450683594,
              53.76170183021049
            ],
            [
              20.50323486328125,
              53.7923366283218
            ],
            [
              20.452766418457028,
              53.78746901588889
            ]
          ])
        end

        let(:polygon_out) do
          AerospikeC::GeoJson.polygon([
            [
              20.378265380859375,
              53.787671844350285
            ],
            [
              20.321273803710938,
              53.77874646437006
            ],
            [
              20.36109924316406,
              53.72677931616645
            ],
            [
              20.440750122070312,
              53.74424420118154
            ],
            [
              20.378265380859375,
              53.787671844350285
            ]
          ])
        end

        context 'within' do
          before(:each) do
            lat = point.lat
            lng = point.lng
            ii = 0.01
            (101..110).each do |i|
              key = AerospikeC::Key.new('test', 'query_test', "query#{i}")
              bins = {
                int_bin: i,
                string_bin: "str#{i}",
                geojson_bin: AerospikeC::GeoJson.point(lng + ii, lat + ii)
              }

              @query_keys << key
              @client.put(key, bins)
              ii += 0.01
            end
            query.range!('int_bin', 101, 110)
          end

          context 'circle' do
            let(:circle) { AerospikeC::GeoJson.circle(point.coordinates, 2000) }
            it 'finds points' do
              predexp.where(predexp['geojson_bin'].within(circle))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(1)
              expect(result.last["int_bin"]).to eq(101)
            end

            it 'no points' do
              predexp.where(predexp['geojson_bin'].within(circle_out))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(0)
            end
          end

          context "polygon" do
            let(:polygon) do
              AerospikeC::GeoJson.polygon(
                [
                  [
                    20.484352111816406,
                    53.82132777853571
                  ],
                  [
                    20.470104217529297,
                    53.787367601290434
                  ],
                  [
                    20.528812408447266,
                    53.78402078201105
                  ],
                  [
                    20.536537170410153,
                    53.82254375222333
                  ],
                  [
                    20.484352111816406,
                    53.82132777853571
                  ]
                ]
              )
            end

            it 'finds points' do
              predexp.where(predexp['geojson_bin'].within(polygon))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(4)
              expect(result.map { |pnt| pnt['int_bin'] }).to eq([101, 102, 103, 104])
            end

            it 'no points' do
              predexp.where(predexp['geojson_bin'].within(polygon_out))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(0)
            end
          end
        end

        context 'contains' do
          context "circle" do
            before(:each) do
              key_1 = AerospikeC::Key.new('test', 'query_test', "query101")
              bins = {
                int_bin: 101,
                geojson_bin: circle_in
              }

              @query_keys << key_1
              @client.put(key_1, bins)
              key_2 = AerospikeC::Key.new('test', 'query_test', "query102")
              bins = {
                int_bin: 102,
                geojson_bin: circle_out
              }

              @query_keys << key_2
              @client.put(key_2, bins)
              query.range!('int_bin', 101, 102)
            end

            it 'cirles only containing point' do
              predexp.where(predexp['geojson_bin'].contains(point))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(1)
              expect(result.last['int_bin']).to eq(101)
            end
          end

          context "polygon" do
            before(:each) do
              key_1 = AerospikeC::Key.new('test', 'query_test', "query101")
              bins = {
                int_bin: 101,
                geojson_bin: polygon_in
              }

              @query_keys << key_1
              @client.put(key_1, bins)
              key_2 = AerospikeC::Key.new('test', 'query_test', "query102")
              bins = {
                int_bin: 102,
                geojson_bin: polygon_out
              }

              @query_keys << key_2
              @client.put(key_2, bins)
              query.range!('int_bin', 101, 102)
            end

            it 'polygons only containing point' do
              predexp.where(predexp['geojson_bin'].contains(point))
              query.predexp = predexp
              result = @client.query(query)

              expect(result.length).to eq(1)
              expect(result.last['int_bin']).to eq(101)
            end
          end
        end
      end

      context 'list' do
        before(:each) do
          (101..110).each do |f|
            key = AerospikeC::Key.new('test', 'query_test', "query#{f}")
            bins = {
              int_bin: f,
              list_bin: [f, f + 1]
            }

            @query_keys << key

            @client.put(key, bins)
          end

          query.range!("int_bin", 101, 110)
        end

        context "and" do
          it "returns records which all elements of the list are greater" do
            predexp.where(predexp["list_bin"].list_and.gt(108))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(2)
            expect(result.map{|b| b["int_bin"]}).to eq([109,110])
          end
        end

        context "or" do
          it "returns records which list contains number" do
            predexp.where(predexp["list_bin"].list_or.eq(108))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(2)
            expect(result.map{|b| b["int_bin"]}).to eq([107,108])
          end
        end
      end

      context "map" do
        before(:each) do
          (101..110).each do |f|
            key = AerospikeC::Key.new('test', 'query_test', "query#{f}")
            bins = {
              int_bin: f,
              hash_bin: {
                second_int_bin: f,
                second_str_bin: "str#{f}"
              },
              hash_and_bin: {
                second_int_bin: f,
                third_int_bin: f + 1
              }
            }

            @query_keys << key

            @client.put(key, bins)
          end

          query.range!("int_bin", 101, 110)
        end

        context "key" do
          it "map_key_and returns records which all map keys match" do
            predexp.where(predexp["hash_bin"].map_key_and.regexp("second"))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(10)
          end

          it "map_key_or returns records which any map key match" do
            predexp.where(predexp["hash_bin"].map_key_or.eq("second_int_bin"))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(10)
          end
        end

        context "val" do
          it "map_val_and returns records which all map values match" do
            predexp.where(predexp["hash_and_bin"].map_val_and.gt(108))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(2)
            expect(result.map{|b| b["int_bin"]}).to eq([109,110])
          end

          it "map_val_or returns records which any map value match" do
            predexp.where(predexp["hash_bin"].map_val_or.eq("str105"))
            query.predexp = predexp
            result = @client.query(query)
            expect(result.count).to eq(1)
            expect(result.last["int_bin"]).to eq(105)
          end
        end
      end
    end
  end
end
