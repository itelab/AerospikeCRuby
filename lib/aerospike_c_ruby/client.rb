#
# ruby implementation methods for AerospikeC::Client
# create there methods that does not require blazing speed
#
AerospikeC::Client.class_eval do
  #
  # list all indexes in server
  #
  def list_indexes
    index_ary = []
    indexes = info_cmd("sindex")
    indexes = indexes.gsub!(/sindex\s/, '').gsub!(/\n/, '').split(";")

    indexes.each do |index|
      indexes_hash = {}

      index.split(":").each do |st|
        type  = ""
        value = nil

        st.split("=").each_with_index do |val, i|
          type = val if i == 0
          value = val if i == 1
        end

        indexes_hash[type] = value
      end

      index_ary << indexes_hash
    end

    index_ary
  end

  #
  # list statistic for server
  #
  def statistics
    stats_hash = {}
    stats = info_cmd("statistics")

    stats = stats.gsub!(/statistics\s/, '').gsub!(/\n/, '').split(";")

    stats.each do |st|
      type  = ""
      value = nil

      st.split("=").each_with_index do |val, i|
        type = val if i == 0
        value = val if i == 1
      end

      stats_hash[type] = value
    end

    stats_hash
  end

  #
  # list all namespaces
  #
  def namespaces
    info_cmd("namespaces").gsub(/namespaces\s/, '').split(";")
  end
end