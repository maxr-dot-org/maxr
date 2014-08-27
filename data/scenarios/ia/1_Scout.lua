-- IA file : this file will run in the context of the AI player, will have access to data limited to the data the the user can see.
io.output("1_Scout_intel.log")
io.write("Loading AI for scout scenario.\n")
io.flush()

gameSettings = game:getSettings()
io.write("Players starting credits : ", gameSettings:getStartingCredits(), "\n")
if gameSettings:getClansEnabled() then 
    io.write("Clan enabled!\n")
else
    io.write("Clan disabled!\n")
end
if gameSettings:getBridgeHeadDefinite() then
    io.write("Bridge head ready\n")
else
    io.write("No bridge head !\n")
end
io.flush()

-- List ennemies, we have only one in this scenario so i do a shortcut to the only ennemy
io.write("My name is: ", ai:getName(), "\n")
io.write("...and my ennemies are: ")
for i,v in pairs(ennemies) do
    io.write(i, ",")
    ennemy = v
end
io.write("\n")
io.flush()

-- Turn begin callback
function newTurn()
  io.write("********* NEW TURN : ", turnCount, " ****************\n")
  
  local vCount = ennemy:getVehicleCount() 
  if vCount > 0 then
      io.write("Ennemi vehicles in sight: ", vCount, "\n")
      
      -- Get table that list ennemy units
      local ennemyVehicles = ennemy:getVehicleIdList()
      
      -- Take first iID
      for i, v in pairs(ennemyVehicles) do
        vID = i
        break
      end
      io.write("DEBUG: ennemy vehicle iID: ", vID, "\n")
      
      -- Get full unit description
      ennemyVehicle = ennemy:getVehicleById(vID)
      --io.write("DEBUG: ennemy position: ", ennemyVehicle["pos"], "\n")
            
      -- Move all units through ennemy position
      io.write("AI vehicle count: ", ai:getVehicleCount(), "\n")
      local myVehicles = ai:getVehicleIdList()
      for i2, v2 in pairs(myVehicles) do
        io.write("Moving unit ", i2, " to position ", ennemyVehicle["pos"]:x(), " - ", ennemyVehicle["pos"]:y(), "\n")
        --CAUTION: cannot move ON the unit, or you should attack the unit !
        --local success, error = game:move(i2, ennemyVehicle["pos"])
        local success, error = game:move(i2, LuaPosition(20, 45))
        if success == true then io.write("Moved with success!\n") else io.write(error, "\n") end       
      end
  end
  
  io.flush()
end