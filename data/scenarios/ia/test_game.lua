local io = io
local math = math
local pairs = pairs
local P = require "position"

module(...)

-- Private
local aiVehicles = {}
-- End private

-- This module emulates MAXR game context
game = {}

function game:move(id, position)
   v = aiVehicles[id]
   if v == nil then return end
   
   old = v.pos
   v.pos = P.waypoint(v.pos, position, v.speed)
   d = P.dist(v.pos, old)
   v.speed = v.speed - math.ceil(d)

   io.write("Move unit ", id, " to the position ", v.pos.x, "-", v.pos.y, "\n")
end

-- Simulates a new turn, refill units with their max speed
function game:newTurn()
   io.write("**************** NEW TURN *********************\n")
   for i, v in pairs(aiVehicles) do
      v.speed = v.speedMax
   end
end

-- This module emulates AI player units
ai = {}

function ai:addVehicle(id, position)
   --io.write("Add AI new vehicle ", id, "\n")
   io.write("Add AI new vehicle ", id, " at position ", position.x, "-", position.y, "\n")
   aiVehicles[id] = { pos=position, speed=12, speedMax=12 }
end

function ai:getVehicleById(id) return aiVehicles[id] end     

