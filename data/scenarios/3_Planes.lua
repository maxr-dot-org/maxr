scenarioName = "Attack plane assault : destroy ennemy base"
description = "Watchout for anti aircrafts"

settings = LuaSettings()
settings:setBridgeHeadDefinite(true)  -- default to mobile (no landing mining station)
game:setSettings(settings)

human = game:getHumanPlayer()
human:setLandingPosition( {x=35, y=60} )
human:setClan(0)
for i=1,6 do human:addLandingUnit("bomber") end
human:addLandingUnit("awac")

mapName = "Iron Cross.wrl"
game:loadMap(mapName)

billy = game:addPlayer("Billy")
billy:setClan(6)
billy:setLandingPosition( {x=50, y=37} )
for i=1,8 do billy:addLandingUnit("missel") end
for i=1,10 do billy:addBuilding("block", 43 + i, 33) end
for i=1,10 do billy:addBuilding("block", 43 + i, 43) end
for i=1,10 do billy:addBuilding("block", 44, 33 + i) end
for i=1,10 do billy:addBuilding("block", 54, 33 + i) end
billy:addBuilding("radar", 49, 39)
billy:addBuilding("gun_aa", 45, 34)
billy:addBuilding("gun_aa", 53, 34)
billy:addBuilding("gun_aa", 45, 42)
billy:addBuilding("gun_aa", 53, 42)

-- Start the game
game:start()

-- Lua log file
io.output(logPath .. "Planes.log")
io.write("Starting scenario : ", scenarioName, "\n")

-- Victory
function victoryCondition()
   -- count AA gun remainings
   local buildings = billy:getBuildingIdList()
   local aaCount = 0
   for i,v in pairs(buildings) do
      if (v == "gun_aa") then aaCount = aaCount + 1 end
   end

   io.write("Victory condition, aa guns remainings : ", aaCount, "\n")
   io.flush()
   if aaCount == 0 then return 1 end
   return 0
end