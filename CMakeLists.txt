cmake_minimum_required(VERSION 3.21)
project(OsgEditor)

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# osg::osg
find_package(OpenSceneGraph 3.6 REQUIRED osg osgDB osgViewer osgUtil osgParticle osgText osgGA osgFX osgQOpenGL)
if(NOT TARGET osg::osg)
  add_library(osg__osg INTERFACE)
  target_include_directories(osg__osg INTERFACE ${OPENSCENEGRAPH_INCLUDE_DIRS})
  target_link_libraries(osg__osg INTERFACE ${OPENSCENEGRAPH_LIBRARIES})
  add_library(osg::osg ALIAS osg__osg)
endif()

# Qt5::Widgets
find_package(Qt5 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

################################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/lib>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin>)

add_subdirectory(src)
