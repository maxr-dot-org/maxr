# Build settings for Unit Test of spiritless_po.
# Copyright © 2022 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.
cmake_minimum_required(VERSION 3.5)

project(test_spiritless_po CXX)

find_package(Catch2 REQUIRED)
add_executable(test_spiritless_po main.cpp Catalog.cpp MetadataParser.cpp PluralParser.cpp PoParser.cpp)
target_link_libraries(test_spiritless_po Catch2::Catch2)
target_include_directories(test_spiritless_po PRIVATE ../include)
target_compile_features(test_spiritless_po PRIVATE cxx_std_11)
target_compile_definitions(test_spiritless_po PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)
if (MSVC)
  set(CMAKE_CXX_FLAGS "/permissive- /EHsc /W4 /O2")
else()
  set(CMAKE_CXX_FLAGS "-Wall -pedantic -O3")
endif()

include(CTest)
include(Catch)
catch_discover_tests(test_spiritless_po)
