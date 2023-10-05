cmake_minimum_required(VERSION 3.1)

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
# === Target
# ============================================
add_executable(resinstaller src/converter.cpp src/file.cpp src/ogg_encode.cpp src/pcx.cpp src/resinstaller.cpp src/3rd/SDL_flic/SDL_flic.c src/wave.cpp)

find_package(SDL2 REQUIRED)
target_include_directories(resinstaller PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(resinstaller ${SDL2_LIBRARY})

find_package(OggVorbis REQUIRED)
target_include_directories(resinstaller PUBLIC ${OGG_INCLUDE_DIR})
target_include_directories(resinstaller PUBLIC ${VORBIS_INCLUDE_DIR})
target_link_libraries(resinstaller ${OGG_LIBRARY})
target_link_libraries(resinstaller ${VORBIS_LIBRARY})
if(WIN32)
else()
	target_link_libraries(resinstaller ${VORBISFILE_LIBRARY} ${VORBISENC_LIBRARY})
endif()
