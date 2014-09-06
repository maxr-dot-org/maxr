require("luaunit")
local P = require("position")

TestPosition = {}
  
function TestPosition:testNew()
   assertEquals(P.new(3, 2), {x=3, y=2})
end

function TestPosition:testDist()
   assertEquals(P.dist({x=3, y=4}, {x=0, y=0}), 5)
end

function TestPosition:testEqual()
   assertTrue(P.equals(P.new(3, 2), {x=3, y=2}))
   assertFalse(P.equals(P.new(3, -2), {x=-3, y=2}))
end

function TestPosition:testShortest()
   dist, pos = P.shortest({x=0, y=0}, {{x=2, y=5}, {x=4, y=3}, {x=7, y=8}, {x=1, y=2}})
   assertEquals(pos, {x=1, y=2})
   dist, pos = P.shortest({x=3, y=3}, {{x=1, y=1}, {x=3, y=3}, {x=5, y=5}, {x=1, y=2}})
   assertEquals(pos, {x=3, y=3})
end

function TestPosition:testVector()
   assertEquals(P.vector({x=3, y=4}, {x=0, y=0}), {x=-3, y=-4}) 
   assertEquals(P.vector({x=3, y=4}, {x=2, y=1}), {x=-1, y=-3}) 
   assertEquals(P.vector({x=0, y=0}, {x=3, y=4}), {x=3, y=4}) 
end

function TestPosition:testMul()
   assertEquals(P.mul({x=3, y=4}, 0), {x=0, y=0}) 
   assertEquals(P.mul({x=3, y=4}, 2), {x=6, y=8}) 
   assertEquals(P.mul({x=3, y=4}, -0.5), {x=-1.5, y=-2}) 
end

function TestPosition:testAdd()
   assertEquals(P.add({x=3, y=4}, {x=0, y=0}), {x=3, y=4}) 
   assertEquals(P.add({x=3, y=4}, {x=2, y=1}), {x=5, y=5}) 
   assertEquals(P.add({x=0, y=0}, {x=-3, y=-4}), {x=-3, y=-4}) 
end

function TestPosition:testFloor()
   assertEquals(P.floor({x=3, y=4}), {x=3, y=4}) 
   assertEquals(P.floor({x=3.2, y=4.7}), {x=3, y=4}) 
   assertEquals(P.floor({x=-3.2, y=-4.7}), {x=-4, y=-5}) 
end

function TestPosition:testCeil()
   assertEquals(P.ceil({x=3, y=4}), {x=3, y=4}) 
   assertEquals(P.ceil({x=3.2, y=4.7}), {x=4, y=5}) 
   assertEquals(P.ceil({x=-3.2, y=-4.7}), {x=-3, y=-4}) 
end

function TestPosition:testRound()
   assertEquals(P.round({x=3, y=4}), {x=3, y=4}) 
   assertEquals(P.round({x=3.2, y=4.7}), {x=3, y=5}) 
   assertEquals(P.round({x=-3.2, y=-4.7}), {x=-3, y=-5}) 
end

function TestPosition:testAbs()
   assertEquals(P.abs({x=3, y=4}), {x=3, y=4}) 
   assertEquals(P.abs({x=-3, y=-4}), {x=3, y=4}) 
end

function TestPosition:testWaypoint()
   assertEquals(P.waypoint({x=0, y=0}, {x=100, y=0}, -25), {x=0, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=100, y=0}, 0), {x=0, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=100, y=0}, 25), {x=25, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=100, y=0}, 125), {x=100, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=-100, y=0}, 25), {x=-25, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=-100, y=0}, 125), {x=-100, y=0})
   assertEquals(P.waypoint({x=50, y=0}, {x=100, y=0}, 25), {x=75, y=0})
   assertEquals(P.waypoint({x=50, y=0}, {x=100, y=0}, 125), {x=100, y=0})
   assertEquals(P.waypoint({x=0, y=0}, {x=0, y=100}, 25), {x=0, y=25})
   assertEquals(P.waypoint({x=0, y=0}, {x=0, y=100}, 125), {x=0, y=100})
   assertEquals(P.waypoint({x=0, y=50}, {x=0, y=100}, 25), {x=0, y=75})
   assertEquals(P.waypoint({x=0, y=50}, {x=0, y=100}, 125), {x=0, y=100})
   
   assertEquals(P.waypoint({x=50, y=50}, {x=53, y=54}, 5), {x=53, y=54})
   assertEquals(P.waypoint({x=50, y=50}, {x=56, y=58}, 5), {x=53, y=54})
   assertEquals(P.waypoint({x=50, y=50}, {x=56, y=58}, 6), {x=53, y=54})
   assertEquals(P.waypoint({x=50, y=50}, {x=44, y=42}, 5), {x=47, y=46})
   assertEquals(P.waypoint({x=50, y=50}, {x=44, y=42}, 6), {x=47, y=46})   
   assertEquals(P.waypoint({x=50, y=50}, {x=56, y=42}, 6), {x=53, y=46})  
end

os.exit(LuaUnit.run())