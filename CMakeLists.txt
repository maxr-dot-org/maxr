
cmake_minimum_required(VERSION 2.8)

project(resinstaller)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)

# ============================================
# === Global additional includes directories
# ============================================

include_directories(src)

if(RESINSTALLER_VERSION)
  MESSAGE (STATUS "  Resinstaller version: ${RESINSTALLER_VERSION}")
  add_definitions( "-DVERSION=\"${RESINSTALLER_VERSION}\"")
endif()



# ============================================
# === Checks
# ============================================

# ---------------------------------------
# Check C++11 compatible compiler
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION_STRING)

  if (NOT (GCC_VERSION_STRING VERSION_GREATER 4.6 OR GCC_VERSION_STRING VERSION_EQUAL 4.6))
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.6 or greater (is ${GCC_VERSION_STRING}).")
  endif ()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string)
  string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

  if (NOT (CLANG_VERSION_STRING VERSION_GREATER 3.1 OR CLANG_VERSION_STRING VERSION_EQUAL 3.1))
    message(FATAL_ERROR "${PROJECT_NAME} requires clang 3.1 or greater (is ${CLANG_VERSION_STRING}).")
  endif ()

  if (CLANG_VERSION_STRING VERSION_GREATER 3.4 OR CLANG_VERSION_STRING VERSION_EQUAL 3.4)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")

  if(WIN32)
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")

  # use WinXP compatible platformtoolset
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v120)
    set(CMAKE_VS_PLATFORM_TOOLSET "v120_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
    set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
  endif()
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v140)
    set(CMAKE_VS_PLATFORM_TOOLSET "v140_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
    set(CMAKE_GENERATOR_TOOLSET "v140_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
  endif()

else()
  message(FATAL_ERROR "You are using an unsupported C++ compiler.")
endif()

# ============================================
# === Target
# ============================================
add_executable(resinstaller src/converter.cpp src/file.cpp src/ogg_encode.cpp src/pcx.cpp src/resinstaller.cpp src/3rd/SDL_flic/SDL_flic.c src/wave.cpp)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(resinstaller ${SDL2_LIBRARY})

find_package(OggVorbis REQUIRED)
include_directories(${VORBIS_INCLUDE_DIR})
target_link_libraries(resinstaller ${VORBIS_LIBRARY})
if(WIN32)
elseif()
	target_link_libraries(resinstaller ${VORBISENC_LIBRARY})
endif()
include_directories(${OGG_INCLUDE_DIR})
target_link_libraries(resinstaller ${OGG_LIBRARY})
