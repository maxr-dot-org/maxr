local io = io
local math = math
local ipairs = ipairs

local M = {}
_ENV = M

-- A position is defined by a table with an x, y indexes: { x=1, y=2 }
function new(x, y) return { x=x, y=y } end

-- Distance between two positions
function dist(p1, p2) return math.sqrt( (p2.x - p1.x)^2 + (p2.y - p1.y)^2 ) end

-- Equal
function equals(p1, p2) return p1 and p2 and p1.x == p2.x and p1.y == p2.y end

-- Vector going from p1 to p2
function vector(p1, p2) return { x = p2.x - p1.x, y = p2.y - p1.y } end

-- Multiply a vector by a scalar
function mul(p, s) return { x = p.x * s, y = p.y * s } end

-- Add two points or vector
function add(p1, p2) return { x = p1.x + p2.x, y = p1.y + p2.y } end

-- Rounds down coordinates
function floor(p) return { x = math.floor(p.x), y = math.floor(p.y) } end

-- Rounds up coordinates
function ceil(p) return { x = math.ceil(p.x), y = math.ceil(p.y) } end

-- Rounds coordinates
function round(p) return { x = math.floor(p.x + 0.5), y = math.floor(p.y + 0.5) } end

-- Abs value of coordinates
function abs(p) return { x = math.abs(p.x), y = math.abs(p.y) } end

-- Return string value of the position
function toString(p) return "(" .. p.x .. "/" .. p.y .. ")" end

-- Shortest between p and list of positions (that is a table of positions)
function shortest(p, plist)
   min = math.huge
   for i, v in ipairs(plist) do
      d = dist(p, v)
      if (d < min) then
         min = d
         shortPos = v
      end
   end
   return min, shortPos 
end 

-- Position you can get with a move from p1 to p2 and not beyond
function waypoint(from, to, speed)
   if speed < 0 then return from end
   d = dist(from, to)
   if speed > d then return to end
   move = mul(vector(from, to), speed / d)
   if move.x > 0 then move.x = math.floor(move.x) else move.x = math.ceil(move.x) end
   if move.y > 0 then move.y = math.floor(move.y) else move.y = math.ceil(move.y) end   
   return add(from, move)
end


return M