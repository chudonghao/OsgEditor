
add_executable(osgforest osgforest.cpp)
target_link_libraries(osgforest ${OPENSCENEGRAPH_LIBRARIES})

add_executable(osgoutline osgoutline.cpp)
target_link_libraries(osgoutline ${OPENSCENEGRAPH_LIBRARIES})
