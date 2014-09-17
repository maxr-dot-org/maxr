local P = require("position")
local M = require("move")

-- Patrol path and Attack path
M.addPath("Patrol", {{x=32, y=37}, {x=32, y=32}})
M.addPath("Attack", {{x=32, y=37}, {x=23, y=44}})

step = 0

-- Turn begin callback
function newTurn()
  io.write("********* NEW TURN : ", turnCount, " ****************\n")
 
  -- Initialize all units on the patrol path 
  if step == 0 then
    for i, v in pairs(ai:getVehicleIdList()) do
       M.setUnitOnPath(i, "Patrol", true)
    end
    step = 1
  end
  
  -- Time to move units on the attack path
  if step == 1 and turnCount > 5 then
     for i, v in pairs(ai:getVehicleIdList()) do
        M.removeUnitFromPath(i)
        M.setUnitOnPath(i, "Attack", true)
     end
    step = 2
  end
 
  -- Makes unit on path moving to their next position
  M.nextSteps()
end

-- Moves end callback
function movesFinished()
  -- Makes unit on path moving to their next position
  M.nextSteps()
end