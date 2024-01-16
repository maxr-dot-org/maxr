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
	"sdl2_mixer.nuget:2.6.3", "sdl2_mixer.nuget.redist:2.6.3",
	"sdl2_net.nuget:2.2.0", "sdl2_net.nuget.redist:2.2.0",
	-- "CrashRpt2.CPP:1.5.3", "CrashRpt2.CPP.redist:1.5.3",
}

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
			"4244", -- '=': conversion from '$type1' to '$type2', possible loss of data
			"4245", -- '=': conversion from '$type1' to '$type2', signed/unsigned mismatch
			"4389", -- '==': signed/unsigned mismatch
			"4456", -- declaration of '$var' hides previous local declaration
			"4457", -- declaration of '$var' hides function parameter
			"4458", -- declaration of '$var' hides class member
			"4459", -- declaration of '$var' hides global declaration
			"4701", -- potentially uninitialized local variable '$var' used
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

if os.isdir(path.join(locationDir, "../resinstaller")) then
group "resinstaller"
externalproject "resinstaller"
	kind "ConsoleApp"
	location(path.join(locationDir, "../resinstaller"))
	uuid "9DEEC088-8951-502D-32D7-88E31E191CB0"
end
