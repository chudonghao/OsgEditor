
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ui)
add_subdirectory(scene)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_ui)
