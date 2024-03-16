cmake_minimum_required(VERSION 3.1)

project(maxr)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)

# ============================================
# === Additional Packages
# ============================================
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)
find_package(OggVorbis REQUIRED)

# ============================================
# === Global defines
# ============================================

if(RESINSTALLER_VERSION)
  MESSAGE (STATUS "  Resinstaller version: ${RESINSTALLER_VERSION}")
  add_definitions( "-DVERSION=\"${RESINSTALLER_VERSION}\"")
endif()

# ============================================
# === Checks
# ============================================

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /bigobj")
  set(RESOURCE_FILE src/maxr.rc)

  # use WinXP compatible platformtoolset
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v120)
    set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
    set(CMAKE_VS_PLATFORM_TOOLSET "v120_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
  endif()
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v140)
    set(CMAKE_GENERATOR_TOOLSET "v140_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
    set(CMAKE_VS_PLATFORM_TOOLSET "v140_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes") # remove noises about C++17 attributes used
endif()


# ============================================
# === Targets
# ============================================

# nlohmann is a library header for json
add_library(nlohmann INTERFACE IMPORTED)
target_include_directories(nlohmann SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/nlohmann/single_include")

# spiritless_po is a library header for gettext po translation
add_library(spiritless_po INTERFACE IMPORTED)
target_include_directories(spiritless_po SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/spiritless_po/include")

# doctest is a library header for test framework
add_library(doctest INTERFACE IMPORTED)
target_include_directories(doctest SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/doctest/doctest")

# SDL_flic
add_library(SDL_flic src/3rd/SDL_flic/SDL_flic.c src/3rd/SDL_flic/SDL_flic.h)
target_include_directories(SDL_flic PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(SDL_flic INTERFACE src)
target_link_libraries(SDL_flic PUBLIC ${SDL2_LIBRARY})

# mve player
add_library(mve_player src/3rd/mveplayer/mveplayer.cpp src/3rd/mveplayer/mveplayer.h)
target_include_directories(mve_player PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(mve_player INTERFACE src)
target_link_libraries(mve_player PUBLIC ${SDL2_LIBRARY})

# maxr library
file(GLOB_RECURSE maxr_LIB_SRCS "src/lib/*.cpp" "src/lib/*.h")
add_library(maxr_lib ${maxr_LIB_SRCS})
target_include_directories(maxr_lib PUBLIC src/lib)
target_include_directories(maxr_lib PUBLIC ${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL2_NET_INCLUDE_DIR})
target_link_libraries(maxr_lib PUBLIC nlohmann ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_NET_LIBRARY})
target_link_libraries(maxr_lib PRIVATE spiritless_po)

## add crash reporting library
if(CRASH_RPT_INCLUDE_DIR)
  target_compile_definitions(maxr_lib PUBLIC USE_CRASH_RPT)
  target_include_directories(maxr_lib PUBLIC ${CRASH_RPT_INCLUDE_DIR})
  target_link_libraries(maxr_lib PUBLIC ${CRASH_RPT_LIB})
endif()
## Add GIT Revision
include(GetGitRevisionDescription)
git_describe(GIT_DESC --always)

if (GIT_DESC)
  message(STATUS "Setting cpp define GIT_DESC to ${GIT_DESC}")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/autoversion.h.in" "${CMAKE_CURRENT_BINARY_DIR}/autoversion.h")
  target_include_directories(maxr_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
else()
  message("Could not automativally set the maxr GIT_DESC from git. Set cmake variable GIT_EXECUTABLE to use this feature.")
endif()

# maxr dedicated server
file(GLOB_RECURSE maxr_dedicatedserver_SRCS "src/dedicatedserver/*.cpp" "src/dedicatedserver/*.h")
add_executable(maxr_dedicatedserver ${maxr_dedicatedserver_SRCS} ${RESOURCE_FILE})
target_include_directories(maxr_lib PUBLIC src)
target_link_libraries(maxr_dedicatedserver maxr_lib)

# maxr
file(GLOB_RECURSE maxr_SRCS "src/ui/*.cpp" "src/ui/*.h")
add_executable(maxr ${maxr_SRCS} ${RESOURCE_FILE})
target_link_libraries(maxr maxr_lib SDL_flic mve_player)

# maxr tests
file(GLOB_RECURSE maxr_tests_SRCS "tests/*.cpp" "tests/*.h")
add_executable(maxr_tests ${maxr_tests_SRCS})
target_link_libraries(maxr_tests maxr_lib doctest)

# resinstaller
add_executable(resinstaller
	resinstaller/src/converter.cpp
	resinstaller/src/converter.h
	resinstaller/src/defines.h
	resinstaller/src/file.cpp
	resinstaller/src/file.h
	resinstaller/src/ogg_encode.cpp
	resinstaller/src/ogg_encode.h
	resinstaller/src/palette.h
	resinstaller/src/pcx.cpp
	resinstaller/src/pcx.h
	resinstaller/src/resinstaller.cpp
	resinstaller/src/resinstaller.h
	resinstaller/src/wave.cpp
	resinstaller/src/wave.h
)
target_include_directories(resinstaller PUBLIC ${SDL2_INCLUDE_DIR} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
target_link_libraries(resinstaller SDL_flic ${SDL2_LIBRARY} ${OGG_LIBRARY} ${VORBIS_LIBRARY})
if(WIN32)
else()
	target_link_libraries(resinstaller ${VORBISFILE_LIBRARY} ${VORBISENC_LIBRARY})
endif()

# set linker flags for target
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(maxr PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  target_compile_definitions(maxr_lib PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
  target_compile_definitions(maxr PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
  target_compile_definitions(maxr_dedicatedserver PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
endif()

# Source group for IDE
macro(GroupSources curdir)
  file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

  foreach(child ${children})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
      GroupSources(${curdir}/${child})
    else()
      string(REPLACE "/" "\\" groupname ${curdir})
      source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
    endif()
  endforeach()
endmacro()

GroupSources(src)
# source_group(TREE . FILES maxr_SRCS)

# ============================================
# === Install
# ============================================

if (UNIX)
  install(TARGETS maxr DESTINATION bin)
  install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/data/"
    DESTINATION share/maxr
    PATTERN ".gitignore" EXCLUDE
  )
endif()
