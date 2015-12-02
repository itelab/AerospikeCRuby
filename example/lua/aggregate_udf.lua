local function one(rec)
  return 1
end

local function add(a, b)
  return a + b
end

local function map_rec(rec)
  return rec["other_bin"]
end

function other_bin_min(stream, min)
  local function other_bin_filter(record)
    info("---------min: %s", min)
    info("---------other_bin: %s", record["other_bin"])
    if record["int_bin"] > 10 then
      return true
    else
      return false
    end
  end

  return stream : filter(other_bin_filter) : map(map_rec)
end

function mycount(stream)
  return stream : map(one) : reduce(add);
end