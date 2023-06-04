local root = path.getabsolute(".")

if (_ACTION == nil) then
	return
end

local locationDir = path.join(root, "../solution", _ACTION, "resinstaller")

local nugetPackages = {
	"sdl2.nuget:2.26.5", "sdl2.nuget.redist:2.26.5",
	-- "vorbis-msvc14-x64:1.3.5.7785", "ogg-msvc-x64:1.3.2.8787", -- x64
	"vorbis-msvc14-x86:1.3.5.7785", "ogg-msvc-x86:1.3.2.8787",  -- x86
}

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
	flags { "FatalWarnings"}

	files { "src/**.cpp", "src/**.c", "src/**.h" } -- source files
	files { ".clang-format", "premake5.lua", "CMakeList.txt", "ABOUT", "AUTHORS", "COPYING", "Readme.md" } -- extra files
	includedirs { "resinstaller/src" }

	filter "action:vs*"
		nuget(nugetPackages)
	filter "action:not vs*"
		links { "ogg", "vorbis", "vorbisfile", "vorbisenc" }
	filter {}
