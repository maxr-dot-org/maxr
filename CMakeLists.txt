
cmake_minimum_required(VERSION 2.8)

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


if(MAXR_BUILD_DEDICATED_SERVER)
  add_definitions( -DDEDICATED_SERVER_APPLICATION )
endif()

# ============================================
# === Checks
# ============================================

# ---------------------------------------
# Check C++11/C++14 compatible compiler
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION_STRING)

  if (NOT (GCC_VERSION_STRING VERSION_GREATER 4.6 OR GCC_VERSION_STRING VERSION_EQUAL 4.6))
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.6 or greater (is ${GCC_VERSION_STRING}).")
  endif ()
  
  if(GCC_VERSION_STRING VERSION_GREATER 4.8 OR GCC_VERSION_STRING VERSION_EQUAL 4.8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  endif()
  
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CLANG_FULL_VERSION_STRING)

  if("${CLANG_FULL_VERSION_STRING}" MATCHES "Apple")
  
    string(REGEX REPLACE ".*Apple LLVM version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${CLANG_FULL_VERSION_STRING})
    
    if (NOT (CLANG_VERSION_STRING VERSION_GREATER 3.1 OR CLANG_VERSION_STRING VERSION_EQUAL 3.1))
      message(FATAL_ERROR "${PROJECT_NAME} requires AppleClang 3.1 or greater (is ${CLANG_VERSION_STRING}).")
    endif ()
  
    if (CLANG_VERSION_STRING VERSION_GREATER 5.1 OR CLANG_VERSION_STRING VERSION_EQUAL 5.1)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
    endif()
  else()
    string(REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${CLANG_FULL_VERSION_STRING})
    
    if (NOT (CLANG_VERSION_STRING VERSION_GREATER 3.1 OR CLANG_VERSION_STRING VERSION_EQUAL 3.1))
      message(FATAL_ERROR "${PROJECT_NAME} requires clang 3.1 or greater (is ${CLANG_VERSION_STRING}).")
    endif ()
  
    if (CLANG_VERSION_STRING VERSION_GREATER 3.4 OR CLANG_VERSION_STRING VERSION_EQUAL 3.4)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -stdlib=libc++")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
    endif()
  endif()
 
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")

  if (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 12.1 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 12.1))
    message(FATAL_ERROR "${PROJECT_NAME} requires ICC 12.1 or greater (is ${CMAKE_CXX_COMPILER_VERSION}).")
  endif ()
    
  if(NOT WIN32)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 15.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 15.0)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()
  endif()
  
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/DNOMINMAX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
  set(RESSOURCE_FILE src/maxr.rc)
else()
  message(FATAL_ERROR "You are using an unsupported C++ compiler.")
endif()

# ============================================
# === Add GIT Revision
# ============================================


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)
include(GetGitRevisionDescription)
git_describe(GIT_DESC --always)

if (GIT_DESC)
  message("Setting cpp define PACKAGE_REV to ${GIT_DESC}")
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
endif()



# ============================================
# === Additional Packages
# ============================================

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
