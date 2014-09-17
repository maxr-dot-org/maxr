scenarioName = "Step by step tutorial, this demonstrate how to write a tutorial to explain things step by step"
description = "Learn maxr basic: move, attack, defend and build factory and new units."

settings = LuaSettings()
settings:setBridgeHeadDefinite(true)
game:setSettings(settings)

human = game:getHumanPlayer()
human:setLandingPosition( {x=20, y=45} )
human:addLandingUnit("scout")
human:addLandingUnit("pionier", 40)

game:loadMap("Lava.wrl")

billy = game:addPlayer("Billy")
billy:setLandingPosition( {x=40, y=20} )
billy:addUnit("tank", 32, 32)
billy:setIaScript("ia/Tutorial_ai.lua")

-- Start the game
game:start()

game:message("Welcome general, we are here to learn a few things togheter !\n" ..
             "First I want you to move your scout around the base to find the ennemy!\n\n" ..
             "Good luck general !")
             
step = 0             
io.output("scenarios/log/Tutorial.log")
                                  
-- Victory
function victoryCondition()
   if step == 0 then
       for i, v in pairs(human:getVehicleIdList()) do
           vehicle = human:getVehicleById(i)
           if (vehicle.pos.x > 26 and vehicle.pos.y < 40) then
               step = 1
               io.write("step 1 completed")
               game:message("Caution general, an ennemi tank has been detected !\n" ..
                            "You should prepare to defend your base, build a defense turret between your base and the ennemi.\n")
           end 
       end
   end 
   if step == 1 then
       for i, v in pairs(human:getBuildingIdList()) do
           if (v == "gun_turret" or v == "gun_ari" or v == "gun_missel") then
               io.write("step 2 completed")
               step = 2
               game:message("Great general, prepare to defend and kill that tank !.\n")
           end 
       end
   end  
   if (billy:getVehicleCount() == 0) then return 1 end
    
   return 0
end