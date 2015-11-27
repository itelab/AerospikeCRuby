function set_count(rec, value)
  rec["count"] = value

  if aerospike:exists(rec) then
    aerospike:update(rec)
  else
    aerospike:create(rec)
  end

  return value
end

function pi(rec)
  return 3
end

function set_bin_val(rec, bin, val, return_val)
  rec[bin] = val

  if aerospike:exists(rec) then
    aerospike:update(rec)
  else
    aerospike:create(rec)
  end

  return return_val
end