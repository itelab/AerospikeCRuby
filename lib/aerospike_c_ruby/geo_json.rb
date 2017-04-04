#
# ruby implementation methods for AerospikeC::GeoJson
# create there methods that does not require blazing speed
#
AerospikeC::GeoJson.class_eval do

  #----------------------------------------------------------------------------
  ## @brief      lng of point
  ##
  ## @return     lng coordinate
  ##
  def lng
    if point?
      coordinates[0]
    elsif circle?
      coordinates[0][0]
    else
      raise NotImplementedError
    end
  end

  #----------------------------------------------------------------------------
  ## @brief      lat of point
  ##
  ## @return     lat coordinate
  ##
  def lat
    if point?
      coordinates[1]
    elsif circle?
      coordinates[0][1]
    else
      raise NotImplementedError
    end
  end

  #----------------------------------------------------------------------------
  ## @brief      radius of circle
  ##
  ## @return     radius radius of cirle in meters
  ##
  def radius
    if circle?
      coordinates[1]
    else
      raise NotImplementedError
    end
  end


  #----------------------------------------------------------------------------
  ## @brief      to array
  ##
  ## @return     [lng, lat]
  ##
  def to_a
    coordinates
  end
end
