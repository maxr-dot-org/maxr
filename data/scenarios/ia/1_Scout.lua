-- IA file : this file will run in the context of the AI player, will have access to data limited to the data the the user can see.
io.output("1_Scout_intel.log")
io.write("Loading AI for scout scenario.\n")
io.flush()

gameSettings = game:getSettings()
io.write("Players starting credits : ", gameSettings:getStartingCredits(), "\n")
if gameSettings:getClansEnabled() then 
    io.write("Clan enabled!\n")
else
    io.write("Clan disabled!\n")
end
if gameSettings:getBridgeHeadDefinite() then
    io.write("Bridge head ready\n")
else
    io.write("No bridge head !\n")
end
io.flush()

io.write("My name is: ", ai:getName(), "\n")
io.write("...and my ennemies are: ")
for i,v in pairs(ennemies) do
    io.write(i, ",")
end
io.write("\n")
io.flush()