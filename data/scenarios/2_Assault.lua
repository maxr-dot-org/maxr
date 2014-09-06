scenarioName = "Assault : destroy ennemy base"
description = "Watchout for missile launcher"

settings = LuaSettings()
settings:setBridgeHeadDefinite(true)  -- default to mobile (no landing mining station)
game:setSettings(settings)

human = game:getHumanPlayer()
human:setLandingPosition( {x=35, y=60} )
human:setClan(4)
for i=1,6 do human:addLandingUnit("assault") end
for i=1,6 do human:addUnit("tank", 33 + i, 50 + i) end
human:addLandingUnit("scanner")

mapName = "Iron Cross.wrl"
game:loadMap(mapName)

billy = game:addPlayer("Billy")
billy:setClan(6)
billy:setLandingPosition( {x=50, y=37} )
billy:addLandingUnit("missel")
billy:addLandingUnit("missel")
billy:addLandingUnit("missel")
for i=1,10 do billy:addBuilding("block", 43 + i, 33) end
for i=1,10 do billy:addBuilding("block", 43 + i, 43) end
for i=1,10 do billy:addBuilding("block", 44, 33 + i) end
for i=1,10 do billy:addBuilding("block", 54, 33 + i) end
billy:addBuilding("radar", 49, 39)
billy:addBuilding("gun_missel", 45, 34)
billy:addBuilding("gun_missel", 53, 34)
billy:addBuilding("gun_missel", 45, 42)
billy:addBuilding("gun_missel", 53, 42)

-- Start the game
game:start()