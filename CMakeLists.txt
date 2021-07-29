
cmake_minimum_required(VERSION 3.1)

project(maxr)


# ============================================
# === Options
# ============================================

option(MAXR_BUILD_DEDICATED_SERVER "Build the dedicated server instead of the normal game" OFF)

# ============================================
# === Global sdditional includes directories
# ============================================

include_directories(src)

# ============================================
# === Global defines
# ============================================

if(PACKAGE_VERSION)
  add_definitions( "-DPACKAGE_VERSION=\"${PACKAGE_VERSION}\"")
endif()

if(MAXR_BUILD_DEDICATED_SERVER)
  add_definitions( -DDEDICATED_SERVER_APPLICATION )
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

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /bigobj")
  set(RESSOURCE_FILE src/maxr.rc)
  
  # use WinXP compatible platformtoolset
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v120)
    set(CMAKE_GENERATOR_TOOLSET "v120_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
    set(CMAKE_VS_PLATFORM_TOOLSET "v120_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
  endif()
  if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL v140)
    set(CMAKE_GENERATOR_TOOLSET "v140_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET" FORCE)
    set(CMAKE_VS_PLATFORM_TOOLSET "v140_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET" FORCE)
  endif()
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

file(GLOB_RECURSE maxr_SRCS "src/*.cpp" "src/*.c" "src/*.h")
 
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

add_executable(maxr ${maxr_SRCS} ${RESSOURCE_FILE})

# set linker flags for target
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(maxr PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
  target_compile_definitions(maxr PRIVATE "NOMINMAX" "_USE_MATH_DEFINES")
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
# Intl (translation)
find_package(Intl REQUIRED)

include_directories(${Intl_INCLUDE_DIRS})
target_link_libraries(maxr ${Intl_LIBRARIES})

# ---------------------------------------
# SDL2
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(maxr ${SDL2_LIBRARY})

# ---------------------------------------
# SDL2_mixer
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_MIXER_INCLUDE_DIR})
target_link_libraries(maxr ${SDL2_MIXER_LIBRARY})

# ---------------------------------------
# SDL2_net
find_package(SDL2_net REQUIRED)

include_directories(${SDL2_NET_INCLUDE_DIR})
target_link_libraries(maxr ${SDL2_NET_LIBRARY})


# ---------------------------------------
# add crash reporting library
if(CRASH_RPT_INCLUDE_DIR)
  include_directories(${CRASH_RPT_INCLUDE_DIR})
  target_link_libraries(maxr ${CRASH_RPT_LIB})
endif()
