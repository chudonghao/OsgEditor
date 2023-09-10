add_subdirectory(ui)
add_subdirectory(test)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_ui)
