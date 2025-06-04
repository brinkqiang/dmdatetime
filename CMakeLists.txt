cmake_minimum_required(VERSION 3.21)

PROJECT(dmdatetime)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(cmake/ModuleImport.cmake)
include(cmake/ModuleCompileOptions.cmake)
include(cmake/ModuleCompilerDefenses.cmake)

ModuleSetCompileOptions()

set(DMDATETIME_VERSION_MAJOR "1")
set(DMDATETIME_VERSION_MINOR "0")
set(DMDATETIME_VERSION_PATCH "1")
set(DMDATETIME_VERSION "${DMDATETIME_VERSION_MAJOR}.${DMDATETIME_VERSION_MINOR}.${DMDATETIME_VERSION_PATCH}")

message(STATUS "VERSION: ${DMDATETIME_VERSION}")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

ModuleImportAll("thirdparty")

InterfaceImport("dmdatetime" "include" "")
if(PROJECT_IS_TOP_LEVEL)
    ExeImport("test" "dmtest;dmdatetime")
endif()
