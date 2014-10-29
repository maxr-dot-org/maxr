
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

#add_definitions( -DTIXML_USE_STL )

if(MAXR_BUILD_DEDICATED_SERVER)
  add_definitions( -DDEDICATED_SERVER_APPLICATION )
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
  add_definitions( -DNOMINMAX )
  add_definitions(/ZI)
  add_definitions(/MP)
else()
  message(FATAL_ERROR "You are using an unsupported C++ compiler.")
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

add_executable(maxr ${maxr_SRCS})

# set linker flags for target
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_target_properties(maxr PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()



# ============================================
# === Additional Packages
# ============================================
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/mk/cmake)

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
