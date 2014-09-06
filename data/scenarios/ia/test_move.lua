require("luaunit")

-- Setup the context to emulate MAXR
local MAXR = require("test_game")
_G["game"] = MAXR.game
_G["ai"] = MAXR.ai

local P = require("position")
local M = require("move")
                      
TestMove = {}
  
function TestMove:setUp()
   -- Add some units to AI (id, position)
   ai:addVehicle(1, {x=10, y=10})
   ai:addVehicle(2, {x=20, y=20})
   ai:addVehicle(3, {x=30, y=30})
   ai:addVehicle(4, {x=40, y=40})
   
   M.addPath("TestPath", {{x=0, y=0}, {x=0, y=50}, {x=50, y=50}, {x=50, y=0}})
   M.addPath("Empty", {})
   M.addPath("One", {{x=45, y=45}})
   
   M.setUnitOnPath(1, "TestPath")
   M.setUnitOnPath(2, "TestPath", true, 4)
   M.setUnitOnPath(3, "Empty")
   M.setUnitOnPath(4, "One")
   
end

function TestMove:testMoveOnPath()
   assertNotNil(ai:getVehicleById(1))
   assertNotNil(ai:getVehicleById(2))
   assertNotNil(ai:getVehicleById(3))
   assertNotNil(ai:getVehicleById(4))
   
   assertEquals(ai:getVehicleById(1).pos, P.new(10, 10))
   assertEquals(ai:getVehicleById(2).pos, P.new(20, 20))
   assertEquals(ai:getVehicleById(3).pos, P.new(30, 30))
   assertEquals(ai:getVehicleById(4).pos, P.new(40, 40))

   M.nextSteps()
   assertNotEquals(ai:getVehicleById(1).pos, P.new(10, 10))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(20, 20))
   assertEquals(ai:getVehicleById(3).pos, P.new(30, 30))
   assertNotEquals(ai:getVehicleById(4).pos, P.new(40, 40))
   
   -- All moves finished, simulate a new turn
   game:newTurn()
   
   M.nextSteps()
   assertEquals(ai:getVehicleById(1).pos, P.new(0, 0))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(20, 20))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(50, 0))
   assertEquals(ai:getVehicleById(4).pos, P.new(45, 45))

   M.nextSteps()
   assertNotEquals(ai:getVehicleById(1).pos, P.new(0, 0))
   assertNotEquals(ai:getVehicleById(1).pos, P.new(0, 50))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(20, 20))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(50, 0))
   assertEquals(ai:getVehicleById(4).pos, P.new(45, 45))
    
    -- All moves finished, simulate a new turn
   game:newTurn()
   
   M.nextSteps()
   assertNotEquals(ai:getVehicleById(2).pos, P.new(20, 20))
   assertNotEquals(ai:getVehicleById(2).pos, P.new(50, 0))
   assertEquals(ai:getVehicleById(4).pos, P.new(45, 45))
 
     -- All moves finished, simulate a new turn
   game:newTurn()
   
   M.nextSteps()
   assertEquals(ai:getVehicleById(2).pos, P.new(50, 0))
   M.nextSteps()
   assertNotEquals(ai:getVehicleById(2).pos, P.new(50, 0))
 
end

function TestMove:tearDown()
end

os.exit(LuaUnit.run())