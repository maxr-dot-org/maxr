scenarioName = "Scout tutorial : eliminate enemy scouting capacity"
description = "You are facing the ennemy scouts, they have same capability as yours, you have to eliminate all of them to blind your ennemy." ..
              "Scouting is very important to know what your ennemy prepares, so killing ennemy scouts is also very important !"

human = game:getHumanPlayer()
human:setLandingPosition(20, 45)
human:setClan(2)
for i=1,6 do human:addLandingUnit("scout") end
human:addUnit("scout", 23, 42)
human:addUnit("scout", 22, 42)

mapName = "Lava.wrl"
game:loadMap(mapName)

billy = game:addPlayer("Billy")
billy:setClan(2)
billy:setLandingPosition(40, 20)
for i=1,3 do billy:addLandingUnit("scout") end
billy:addUnit("scout", 35, 19)
billy:addUnit("scout", 35, 20)
billy:addUnit("scout", 43, 35)
billy:addUnit("scout", 43, 36)

-- Add an IA script to Billy so he will move and attack ennemy approaching
billy:setIaScript("ia/1_Scout.lua")

-- Start the game
game:start()

-- Victory
function victoryCondition()
   if billy:getVehicleCount() == 0 then return 1 end
   return 0
end