cmake_minimum_required(VERSION 3.28)

project(Cprime LANGUAGES C CXX)
enable_testing()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(cmake/IPOSupport.cmake)


# TODO: add script to simplify loading deps
add_subdirectory(vendor/Catch2 EXCLUDE_FROM_ALL SYSTEM)
include(Catch)
add_subdirectory(vendor/utfcpp EXCLUDE_FROM_ALL SYSTEM)
add_library(utf8cpp::utf8cpp ALIAS utf8cpp)
add_subdirectory(vendor/magic_enum EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(vendor/cxxopts EXCLUDE_FROM_ALL SYSTEM)

set(BOOST_SUPERPROJECT_VERSION 1.91.0)
foreach(module IN ITEMS config assert move intrusive)
  add_subdirectory(
    "vendor/boost/${module}"
    EXCLUDE_FROM_ALL
    SYSTEM
  )
endforeach()


add_subdirectory(src/)
add_subdirectory(test/)
