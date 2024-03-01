newoption {
	trigger = "to",
	value   = "path",
	description = "Set the output location for the generated files",
	default = "solution/%{_ACTION}/maxr"
}

newoption {
	trigger = "crashRpt_root",
	value   = "path",
	description = "Set the location of crashRpt_root",
}

if (_ACTION == nil) then
	return
end

local locationDir = _OPTIONS["to"]

local nugetPackages = {
	"sdl2.nuget:2.28.0", "sdl2.nuget.redist:2.28.0",
	"sdl2_mixer.nuget:2.8.0", "sdl2_mixer.nuget.redist:2.8.0",
	"sdl2_net.nuget:2.2.0", "sdl2_net.nuget.redist:2.2.0",
	-- "CrashRpt2.CPP:1.5.3", "CrashRpt2.CPP.redist:1.5.3",
	"vorbis-msvc14-x86:1.3.5.7785", "ogg-msvc-x86:1.3.2.8787",  -- x86
	-- "vorbis-msvc14-x64:1.3.5.7785", "ogg-msvc-x64:1.3.2.8787", -- x64
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

local function linksToCrashRpt()
	if _OPTIONS["crashRpt_root"] ~= nil then
		files {
			path.join(_OPTIONS["crashRpt_root"], "bin/CrashRpt1403.dll"),
			path.join(_OPTIONS["crashRpt_root"], "bin/CrashSender1403.exe"),
			path.join(_OPTIONS["crashRpt_root"], "bin/crashrpt_lang.ini"),
		}
		libdirs { path.join(_OPTIONS["crashRpt_root"], "lib") }
		links { "CrashRpt1403" }
		filter { "files:" .. path.join(_OPTIONS["crashRpt_root"], "bin/*.*") }
			buildaction "Copy"
		filter {}
	end
end

workspace "Maxr"
	location(locationDir)
	configurations {"Debug", "Release"}

	language "C++"
	cppdialect "C++17"

	objdir(path.join(locationDir, "obj")) -- premake adds $(configName)/$(AppName)
	startproject "maxr"

	externalwarnings "Off"

if _OPTIONS["crashRpt_root"] ~= nil then
	defines { "USE_CRASH_RPT" }
end

	filter { "action:vs*" }
		nuget(nugetPackages)
	filter { "toolset:msc*" }
		--defines { "USE_CRASH_RPT" }
		defines { "_CRT_SECURE_NO_WARNINGS" } -- 4996: '$func': This function or variable may be unsafe. Consider using $func2 instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
		buildoptions { "/Zc:__cplusplus" } -- else __cplusplus would be 199711L
		disablewarnings {
			"4100", -- '%var': unreferenced formal parameter
			"4013", -- '$func' undefined, assuming extern return int
			"4244", -- '=': conversion from '$type1' to '$type2', possible loss of data
			"4245", -- '=': conversion from '$type1' to '$type2', signed/unsigned mismatch
			"4389", -- '==': signed/unsigned mismatch
			"4456", -- declaration of '$var' hides previous local declaration
			"4457", -- declaration of '$var' hides function parameter
			"4458", -- declaration of '$var' hides class member
			"4459", -- declaration of '$var' hides global declaration
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

project "resinstaller"
	kind "ConsoleApp"
	uuid "9DEEC088-8951-502D-32D7-88E31E191CB0"
	
	targetdir "data"
	debugdir "data"
	filter { "configurations:Release" }
		targetname "resinstaller"
	filter { "configurations:Debug" }
		targetname "resinstaller_%{_ACTION}_%{cfg.buildcfg}"
	filter {}

	warnings "Extra"
	-- flags { "FatalWarnings"} -- We still have warnings :-(

	files { "resinstaller/src/**.cpp", "resinstaller/src/**.c", "resinstaller/src/**.h" } -- source files
	files { "resinstaller/.clang-format", "resinstaller/ABOUT", "resinstaller/AUTHORS", "resinstaller/Readme.md" } -- extra files
	includedirs { "src", "resinstaller/src" }

	links "SDL_flic"

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

project "maxr"
	kind "WindowedApp"
	targetdir "data"

	filter { "configurations:Release" }
		targetname "maxr"
	filter { "configurations:Debug" }
		targetname "maxr_%{_ACTION}_%{cfg.buildcfg}"
	filter {}

	warnings "Extra"
	flags { "FatalWarnings"}

	files { "src/ui/**.cpp", "src/ui/**.h", "src/maxr.rc" }
	includedirs { "src", "src/lib" }
	links { "maxr_lib", "SDL_flic", "mveplayer" }
	linksToCrashRpt()

	debugdir "data"

project "dedicated_server"
	kind "ConsoleApp"
	targetdir "data"
	filter { "configurations:Release" }
		targetname "dedicatedserver"
	filter { "configurations:Debug" }
		targetname "dedicatedserver_%{_ACTION}_%{cfg.buildcfg}"
	filter {}

	warnings "Extra"
	flags { "FatalWarnings"}

	files { "src/dedicatedserver/**.cpp", "src/dedicated_server/**.h", "src/maxr.rc" }
	includedirs { "src", "src/lib" }
	links { "maxr_lib", "SDL_flic", "mveplayer" }
	linksToCrashRpt()

	debugdir "data"

project "tests"
	kind "ConsoleApp"
	targetdir(path.join(locationDir, "bin/%{cfg.buildcfg}"))
	targetname "maxr_tests"

	warnings "Extra"
	flags { "FatalWarnings"}

	files { "tests/**.cpp", "tests/**.h" }
	vpaths { ["tests/*"] = "tests" }

	includedirs { "src", "src/lib" }
	links { "maxr_lib" }
	linksToCrashRpt()

project "maxr_lib"
	kind "StaticLib"

	targetdir(path.join(locationDir, "obj/%{cfg.buildcfg}"))
	targetname "maxr_lib"

	warnings "Extra"
	flags { "FatalWarnings"}

	files { "src/lib/**.cpp", "src/lib/**.h", "src/**.in", "src/.clang-format", "CMakeList.txt" }
	includedirs { "src", "src/lib" }
	externalincludedirs { "src/3rd/spiritless_po/include" }

if _OPTIONS["crashRpt_root"] ~= nil then
	externalincludedirs { path.join(_OPTIONS["crashRpt_root"], "include") }
end

group "3rd"
project "mveplayer"
	kind "StaticLib"

	targetdir(path.join(locationDir, "obj/%{cfg.buildcfg}"))
	targetname "mveplayer"

	files { "src/3rd/mveplayer/**.cpp", "src/3rd/mveplayer/**.h" }
	vpaths { ["mveplayer/*"] = "src/3rd/mveplayer" }
	includedirs { "src", "src/lib" }

project "SDL_flic"
	kind "StaticLib"

	targetdir(path.join(locationDir, "obj/%{cfg.buildcfg}"))
	targetname "SDL_flic"

	files { "src/3rd/SDL_flic/**.c", "src/3rd/SDL_flic/**.h" }
	vpaths { ["SDL_flic/*"] = "src/3rd/SDL_flic" }

if premake.action.supports("None") then
project "doctest" -- header only
	kind "None"

	files { "src/3rd/doctest/**.*" }
	vpaths { ["doctest/*"] = "src/3rd/doctest" }

project "nlohmann" -- header only
	kind "None"

	files { "src/3rd/nlohmann/**.*" }
	vpaths { ["nlohmann/*"] = "src/3rd/nlohmann" }

project "spiritless_po" -- header only
	kind "None"

	files { "src/3rd/spiritless_po/**.*" }
	vpaths { ["spiritless_po/*"] = "src/3rd/spiritless_po" }

group ""
project "data" -- data
	kind "None"

	files { "data/**.*" }
	vpaths { ["data/*"] = "data" }
	removefiles { "data/**.dll", "data/**.exe" }
end
