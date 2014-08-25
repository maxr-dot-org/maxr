-- This is a comment: Scenario name and description definition
scenarioName = "Scenario test-demo"
description = "This script is a live demo of all scriptable things in the scenario"

-- Game settings: this is optionnal
settings = LuaSettings()
settings:setStartingCredits(250)      -- default to 0.
settings:setClansEnabled(true)        -- default to true
settings:setHumanChooseClan(true)     -- default false
settings:setBridgeHeadDefinite(true)  -- default to mobile (no landing mining station)
game:setSettings(settings)

-- Set human player position
human = game:getHumanPlayer()
human:setLandingPosition(50, 50)

-- Add human player landing units (use name from vehicles directory of the game)
-- Note only Land and Air units are accepted, for sea units use addUnit(name, x, y) and ensure you place them in water
human:addLandingUnit("assault")
human:addLandingUnit("assault")
human:addLandingUnit("scout")
human:addLandingUnit("scanner")
human:addLandingUnit("donotexist")     -- typo mistake will be silently ignored
human:addLandingUnit("konstrukt", 33)  -- choose cargo to add
human:addLandingUnit("pionier", 22)
human:addLandingUnit("infantery")
human:addLandingUnit("tank")
human:addLandingUnit("apc")
human:addLandingUnit("commando")
human:addLandingUnit("missel")
human:addLandingUnit("corvet")
human:addLandingUnit("bomber")
human:addUnit("scout", 45, 45)

-- Map selection
mapName = "Iron Cross.wrl"

-- May also choose random in available maps
availMaps = game:getAvailableMaps()
math.randomseed(os.time())
randIndex = math.random(#availMaps)
mapName = availMaps[randIndex]
game:loadMap(mapName)


-- Add AI players
billy = game:addPlayer("Billy")
theo = game:addPlayer("Theo")
raul = game:addPlayer("Raul")
billy:setClan(5)

-- AI player landing positions
billy:setLandingPosition(36, 53)
theoPosition = LuaPosition(55, 36)
theo:setLandingPosition(theoPosition)
raul:setLandingPosition(60, 65)

-- AI landing units will be dropped around landing position
billy:addLandingUnit("scout")
theo:addLandingUnit("scout")
raul:addLandingUnit("scout")

-- AI other units can be placed precisely on the map, be sure the unit can be placed on the map !
billy:addUnit("scout", 44, 52)

-- Start the game
game:start()


-- Victory condition, should return -1 defeat, 0 continue, 1 victory
-- The function is called from game engine at each turn
function victoryCondition()
   if billy:getVehicleCount() == 0 and theo:getVehicleCount() == 0 and raul:getVehicleCount() == 0 then return 1 end
   if turnCount >= 7 then return -1 end
   return 0
end