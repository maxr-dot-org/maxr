-- Keep access to standard library modules
local io = io
local math = math
local pairs = pairs
local assert = assert

-- Keep access to some global MAXR modules or variables
local P = require "position"
local game = game
local ai = ai

local M = {}
_ENV = M

-- Private: table to store path
local paths = {}

-- Private: table to store units on a path
local unitsPath = {}

-- END private



-- Register a path (list of successive positions) in the module, to be used by the setUnitOnPath function for example
function addPath(name, path)
   if #path == 0 then
      io.write("WARNING: Add empty path to move module. Path discarded.\n")
   else 
      paths[name] = path
   end 
end

-- Set the unit to follow the given path, 
--   * optionnal loop is for cycling to first position when end is reached. Default to false
--   * optionnal startPosition is to start the path from given position, default to zero
function setUnitOnPath(iID, pathName, loop, startPosition)
   loop = loop or false
   startPosition = startPosition or 1
   
   unitsPath[iID] = { pathName = pathName,
                      nextPositionIndex = startPosition,
                      loop = loop,
                      pause = false
                      }
end

-- Remove the unit from its path moves
function removeUnitFromPath(iID)
   unitsPath[iID] = nil
end

-- Set unit on pause for the path moves (call again with false extra arg to restart)
function pauseUnitFromPath(iID, pause)
   pause = pause or true
   if unitsPath[iID] == nil then return end
   unitsPath[iID].pause = pause
end

-- Move units next steps on their paths, call this function each time you wanna check and move units on path
function nextSteps()
   for i, v in pairs(unitsPath) do
      if (v.pause == false and paths[v.pathName]) then
         vehicle = ai:getVehicleById(i)
         if (vehicle.speed > 0) then      -- check if it has remaining movements
            -- check if next step is reached
            nextPosition = paths[v.pathName][v.nextPositionIndex]
            assert(nextPosition, "Next Steps position is nil")
            if (P.equals(nextPosition, vehicle.pos)) then
               -- Check if we reached the end of the path
               if #(paths[v.pathName]) == v.nextPositionIndex then
                  if v.loop == true then v.nextPositionIndex = 1
                  else unitsPath[i] = nil    -- end of the path, remove unit from list
                  end
               else v.nextPositionIndex = v.nextPositionIndex + 1
               end
               nextPosition = paths[v.pathName][v.nextPositionIndex]
               assert(nextPosition, "Next Steps position incremented is nil")
            end
            
            -- TODO : check if next position is busy on map, then choose a free neighbour !
           
            -- move to next step
            local success, errstr = game:move(i, nextPosition)
            if success then 
               io.write("Move unit ", i, " from ", P.toString(vehicle.pos), " on path to ", P.toString(nextPosition), "\n")
            else 
               io.write("Move error: ", errstr, "\n") 
            end
         end         
      end
   end
end

return M