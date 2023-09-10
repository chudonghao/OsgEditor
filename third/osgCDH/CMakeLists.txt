
cmake_minimum_required(VERSION 3.10)
project(osgCDH)

set(OSGCDH_BUILD_STATIC OFF CACHE BOOL "Build osgCDH static")

if (OSGCDH_BUILD_STATIC)
    message(STATUS "Build osgCDH static")
endif ()
################################################################################
# OSG
find_package(OpenSceneGraph REQUIRED osg osgUtil)

message(STATUS "OPENSCENEGRAPH_INCLUDE_DIRS:${OPENSCENEGRAPH_INCLUDE_DIRS}")
message(STATUS "OPENSCENEGRAPH_LIBRARIES:${OPENSCENEGRAPH_LIBRARIES}")
message(STATUS "OPENSCENEGRAPH_VERSION:${OPENSCENEGRAPH_VERSION}")

add_subdirectory(src)
