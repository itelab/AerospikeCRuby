AerospikeC::Client.class_eval do
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
end