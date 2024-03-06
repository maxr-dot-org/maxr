cmake_minimum_required(VERSION 3.1)

project(maxr)

# ============================================
# === Global additional includes directories
# ============================================

include_directories(src)
include_directories(src/lib)

# ============================================
# === Global defines
# ============================================

if(RESINSTALLER_VERSION)
  MESSAGE (STATUS "  Resinstaller version: ${RESINSTALLER_VERSION}")
  add_definitions( "-DVERSION=\"${RESINSTALLER_VERSION}\"")
endif()

if(PACKAGE_VERSION)
  add_definitions( "-DPACKAGE_VERSION=\"${PACKAGE_VERSION}\"")
endif()

if(CRASH_RPT_INCLUDE_DIR)
  add_definitions( -DUSE_CRASH_RPT )
endif()

if(NIGHTLY_BUILD)
  add_definitions( -DNIGHTLY_BUILD )
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
# === Add GIT Revision
# ============================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)
include(GetGitRevisionDescription)
git_describe(GIT_DESC --always)

if (GIT_DESC)
  message(STATUS "Setting cpp define PACKAGE_REV to ${GIT_DESC}")
  add_definitions("-DHAVE_AUTOVERSION_H")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/autoversion.h.in" "${CMAKE_CURRENT_BINARY_DIR}/autoversion.h")
  include_directories(${CMAKE_CURRENT_BINARY_DIR})

else()
  message("Could not automativally set the maxr PACKAGE_REV from git. Set cmake variable GIT_EXECUTABLE to use this feature.")
endif()

# ============================================
# === Target
# ============================================

add_library(SDL_flic src/3rd/SDL_flic/SDL_flic.c src/3rd/SDL_flic/SDL_flic.h)
add_library(mve_player src/3rd/mveplayer/mveplayer.cpp src/3rd/mveplayer/mveplayer.h)
# nlohman is library header library for json
# spiritless_po is library header for gettext po translation
# doctest is library header for test framework

file(GLOB_RECURSE maxr_LIB_SRCS "src/lib/*.cpp" "src/lib/*.h")
file(GLOB_RECURSE maxr_SRCS "src/ui/*.cpp" "src/ui/*.h")
file(GLOB_RECURSE maxr_dedicatedserver_SRCS "src/dedicatedserver/*.cpp" "src/dedicatedserver/*.h")
file(GLOB_RECURSE maxr_tests_SRCS "tests/*.cpp" "tests/*.h")

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

add_executable(resinstaller
	resinstaller/src/converter.cpp
	resinstaller/src/file.cpp
	resinstaller/src/ogg_encode.cpp
	resinstaller/src/pcx.cpp
	resinstaller/src/resinstaller.cpp
	resinstaller/src/wave.cpp
)

add_library(maxr_lib ${maxr_LIB_SRCS})

add_executable(maxr_dedicatedserver ${maxr_dedicatedserver_SRCS} ${RESOURCE_FILE})
add_executable(maxr ${maxr_SRCS} ${RESOURCE_FILE})
add_executable(maxr_tests ${maxr_tests_SRCS})

target_include_directories(maxr_lib SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/spiritless_po/include")
target_include_directories(maxr_tests SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/doctest/doctest")

target_link_libraries(resinstaller SDL_flic)
target_link_libraries(maxr maxr_lib SDL_flic mve_player)
target_link_libraries(maxr_dedicatedserver maxr_lib)
target_link_libraries(maxr_tests maxr_lib)

# set linker flags for target
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(maxr PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  target_compile_definitions(maxr_lib PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
  target_compile_definitions(maxr PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
  target_compile_definitions(maxr_dedicatedserver PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
endif()

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

# ============================================
# === Additional Packages
# ============================================

# ---------------------------------------
# SDL2
find_package(SDL2 REQUIRED)

target_include_directories(maxr_lib PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(maxr_lib PUBLIC ${SDL2_LIBRARY})
target_include_directories(mve_player PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(mve_player PUBLIC ${SDL2_LIBRARY})
target_include_directories(SDL_flic PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(SDL_flic PUBLIC ${SDL2_LIBRARY})
target_include_directories(resinstaller PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(resinstaller ${SDL2_LIBRARY})

# ---------------------------------------
# SDL2_mixer
find_package(SDL2_mixer REQUIRED)

target_include_directories(maxr_lib PUBLIC ${SDL2_MIXER_INCLUDE_DIR})
target_link_libraries(maxr_lib PUBLIC ${SDL2_MIXER_LIBRARY})

# ---------------------------------------
# SDL2_net
find_package(SDL2_net REQUIRED)

target_include_directories(maxr_lib PUBLIC ${SDL2_NET_INCLUDE_DIR})
target_link_libraries(maxr_lib PUBLIC ${SDL2_NET_LIBRARY})

# ---------------------------------------
# OggVorbis

find_package(OggVorbis REQUIRED)

target_include_directories(resinstaller PUBLIC ${OGG_INCLUDE_DIR})
target_include_directories(resinstaller PUBLIC ${VORBIS_INCLUDE_DIR})
target_link_libraries(resinstaller ${OGG_LIBRARY})
target_link_libraries(resinstaller ${VORBIS_LIBRARY})
if(WIN32)
else()
	target_link_libraries(resinstaller ${VORBISFILE_LIBRARY} ${VORBISENC_LIBRARY})
endif()

# ---------------------------------------
# add crash reporting library
if(CRASH_RPT_INCLUDE_DIR)
  target_include_directories(maxr_lib PUBLIC ${CRASH_RPT_INCLUDE_DIR})
  target_link_libraries(maxr_lib PUBLIC ${CRASH_RPT_LIB})
endif()
