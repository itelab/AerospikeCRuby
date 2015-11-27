function add_cords(rec, lat, lng)
  rec["lat"] = lat
  rec["lng"] = lng

  if aerospike:exists(rec) then
    aerospike:update(rec)
  else
    aerospike:create(rec)
  end

  return rec
end