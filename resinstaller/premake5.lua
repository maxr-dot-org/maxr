newoption {
	trigger = "to",
	value   = "path",
	description = "Set the output location for the generated files",
	default = "solution/%{_ACTION}/resinstaller"
}

if (_ACTION == nil) then
	return
end

local locationDir = _OPTIONS["to"]

local nugetPackages = {
	"sdl2.nuget:2.28.0", "sdl2.nuget.redist:2.28.0",
	-- "vorbis-msvc14-x64:1.3.5.7785", "ogg-msvc-x64:1.3.2.8787", -- x64
	"vorbis-msvc14-x86:1.3.5.7785", "ogg-msvc-x86:1.3.2.8787",  -- x86
}

local SDL2_DIR = os.getenv("SDL2_DIR")
local SDL2_headerPath = os.findheader("SDL2/SDL.h", SDL2_DIR)
local SDL2_libraryPath = os.findlib("SDL2", SDL2_DIR)

local vorbis_headerPath = os.findheader("vorbis/vorbisenc.h")
local ogg_headerPath = os.findheader("ogg/ogg.h")

print("SDL2_DIR: ", SDL2_DIR)
print("SDL2 header path: ", SDL2_headerPath)
print("SDL2 library path: ", SDL2_libraryPath)
print("vorbis header path: ", vorbis_headerPath)
print("ogg header path: ", ogg_headerPath)

workspace "Resinstaller"
	location(locationDir)
	configurations {"Debug", "Release"}

	language "C++"
	cppdialect "C++17"

	objdir(path.join(locationDir, "obj")) -- premake adds $(configName)/$(AppName)
	startproject "resinstaller"

	externalwarnings "Off"
	filter { "toolset:msc*" }
		defines { "_CRT_SECURE_NO_WARNINGS" } -- 4996: '$func': This function or variable may be unsafe. Consider using $func2 instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
		disablewarnings {
			"4013", -- '$func' undefined, assuming extern return int
			"4244", -- '=': conversion from '$type1' to '$type2', possible loss of data
			"4389", -- '==': signed/unsigned mismatch
			"4456", -- declaration of '$var' hides previous local declaration
			"4701", -- potentially uninitialized local variable '$var' used
			"4703", -- potentially uninitialized local pointer variable '$var' used
			"4996", -- '$func': The POSIX name of this item is deprecated. Instead, use the ISO C and C++ conformant name: $func2. See online help for details.

		}
	filter { "configurations:Debug" }
		symbols "On"
		defines { "DEBUG" }
		optimize "Off"

	filter { "configurations:Release" }
		symbols "Off"
		defines { "NDEBUG" }
		optimize "On"

	filter "system:windows"
		defines { "WIN32" }
	filter {}

project "resinstaller"
	kind "ConsoleApp"
	-- targetdir "maxr/data"
	uuid "9DEEC088-8951-502D-32D7-88E31E191CB0"
	debugdir "../maxr/data"
	
	targetname "resinstaller"

	warnings "Extra"
	-- flags { "FatalWarnings"} -- We still have warnings :-(

	files { "src/**.cpp", "src/**.c", "src/**.h" } -- source files
	files { ".clang-format", "premake5.lua", "CMakeList.txt", "ABOUT", "AUTHORS", "COPYING", "Readme.md" } -- extra files
	includedirs { "resinstaller/src" }

	filter "action:vs*"
		nuget(nugetPackages)
	filter "action:not vs*"

if vorbis_headerPath then
		includedirsafter { vorbis_headerPath }
end
if ogg_headerPath then
		includedirsafter { ogg_headerPath }
end
if SDL2_headerPath then
		externalincludedirs { path.join(SDL2_headerPath, "SDL2") }
end
if SDL2_libraryPath then
		libdirs { SDL2_libraryPath }
end
		links { "SDL2", "ogg", "vorbis", "vorbisfile", "vorbisenc" }
	filter {}
