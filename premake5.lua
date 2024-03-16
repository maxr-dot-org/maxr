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

local function autoversion_h()
	local git_tag, errorCode = os.outputof("git describe --tag")
	if errorCode == 0 then
		print("git description: ", git_tag)
		local content = io.readfile("src/autoversion.h.in")
		content = content:gsub("${GIT_DESC}", git_tag)
		
		local f, err = os.writefile_ifnotequal(content, path.join(locationDir, "autoversion.h"))
		
		if (f == 0) then
			-- file not modified
		elseif (f < 0) then
			error(err, 0)
			return false
		elseif (f > 0) then
			print("Generated autoversion.h...")
		end

		return true
	else
		print("`git describe --tag` failed with error code", errorCode)
		return false
	end
end

local have_autoversion_h = autoversion_h()

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
		defines { "_CRT_SECURE_NO_WARNINGS" } -- 4996: '$func': This function or variable may be unsafe. Consider using $func2 instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
		buildoptions { "/Zc:__cplusplus" } -- else __cplusplus would be 199711L
		disablewarnings {
			"4458", -- declaration of '$var' hides class member
		}

	filter { "configurations:Debug" }
		symbols "On"
		defines { "DEBUG" }
		optimize "Off"

	filter { "configurations:Release" }
		symbols "Off"
		defines { "NDEBUG" }
		optimize "On"

	filter { "toolset:msc*" }
		symbols "On" -- pdb even on release

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

	filter { "toolset:msc*" }
		disablewarnings {
			"4244", -- '=': conversion from '$type1' to '$type2', possible loss of data
			"4456", -- declaration of '$var' hides previous local declaration
			"4701", -- potentially uninitialized local variable '$var' used
			"4703", -- potentially uninitialized local pointer variable '$var' used
			"4996", -- '$func': The POSIX name of this item is deprecated. Instead, use the ISO C and C++ conformant name: $func2. See online help for details.
		}
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

	if have_autoversion_h then
		includedirs { locationDir } -- for generated file (autoversion.h)
	end

	files { "src/ui/**.cpp", "src/ui/**.h", "src/maxr.rc" }
	includedirs { "src", "src/lib" }
	externalincludedirs { "submodules/nlohmann/single_include" }
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

	if have_autoversion_h then
		includedirs { locationDir } -- for generated file (autoversion.h)
	end

	files { "src/dedicatedserver/**.cpp", "src/dedicated_server/**.h", "src/maxr.rc" }
	includedirs { "src", "src/lib" }
	externalincludedirs { "submodules/nlohmann/single_include" }
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

	includedirs { "src/lib" }
	externalincludedirs { "submodules/doctest/doctest", "submodules/nlohmann/single_include" }
	links { "maxr_lib" }
	linksToCrashRpt()

	filter { "toolset:msc*" }
		disablewarnings {
			"4459", -- declaration of '$var' hides global declaration
		}
	filter {}

project "maxr_lib"
	kind "StaticLib"

	targetdir(path.join(locationDir, "obj/%{cfg.buildcfg}"))
	targetname "maxr_lib"

	warnings "Extra"
	flags { "FatalWarnings"}

	if have_autoversion_h then
		files { path.join(locationDir, "autoversion.h") } -- generated file
		includedirs { locationDir } -- for generated file (autoversion.h)
	end

	files { "src/lib/**.cpp", "src/lib/**.h" } -- Source
	files { "src/autoversion.h.in"} -- template to generate autoversion.h
	files { "src/.clang-format" }
	files { "CMakeLists.txt", "mk/cmake/**.*" } --CMake stuff
	includedirs { "src", "src/lib" }
	externalincludedirs { "submodules/nlohmann/single_include", "submodules/spiritless_po/include" }

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

project "SDL_flic"
	kind "StaticLib"

	targetdir(path.join(locationDir, "obj/%{cfg.buildcfg}"))
	targetname "SDL_flic"

	files { "src/3rd/SDL_flic/**.c", "src/3rd/SDL_flic/**.h" }
	vpaths { ["SDL_flic/*"] = "src/3rd/SDL_flic" }

	filter { "toolset:msc*" }
		disablewarnings {
			"4244", -- '=': conversion from '$type1' to '$type2', possible loss of data
		}
	filter {}

if premake.action.supports("None") then
project "doctest" -- header only
	kind "None"

	files { "submodules/doctest/**.*" }
	vpaths { ["doctest/*"] = "submodules/doctest" }

project "nlohmann" -- header only
	kind "None"

	files { "submodules/nlohmann/**.*" }
	vpaths { ["nlohmann/*"] = "submodules/nlohmann" }

project "spiritless_po" -- header only
	kind "None"

	files { "submodules/spiritless_po/**.*" }
	vpaths { ["spiritless_po/*"] = "submodules/spiritless_po" }

group ""
project "data" -- data
	kind "None"

	files { "data/**.*" }
	vpaths { ["data/*"] = "data" }
	removefiles { "data/**.dll", "data/**.exe" }
end
