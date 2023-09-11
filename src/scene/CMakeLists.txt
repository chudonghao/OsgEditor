
add_library(
  ${PROJECT_NAME}_scene
  Outline.cpp
  Outline.h
)

target_link_libraries(
  ${PROJECT_NAME}_scene
  osg::osg
)
