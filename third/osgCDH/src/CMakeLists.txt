
set(
    SRCS
    Outline.cpp
)

if (OSGCDH_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC ${SRCS})
else ()
    add_library(${PROJECT_NAME} ${SRCS})
endif ()

target_link_libraries(
    ${PROJECT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)
