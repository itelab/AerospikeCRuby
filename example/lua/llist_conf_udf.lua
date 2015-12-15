local llist_settings = require("ldt/lib_llist")
local mymodule = {}

function mymodule.adjust_settings(ldtMap)
  local configMap = map();
  configMap['MaxObjectSize'] = 200000
  llist_settings.compute_settings(ldtMap, configMap)
  info ("setting LDT to be %d",configMap['MaxObjectSize'])
end

function mymodule.my_filter_func(ldt_key,args)
    if (ldt_key % args[1])==0 then
        return ldt_key
    else
        return nil
    end
end

return mymodule;