cmake_minimum_required(VERSION 3.11)
project(MyRaylibGame C CXX)

# Tell CMake to look in our local 'raylib' folder and build it
add_subdirectory(raylib)

# Create the executable
add_executable(${PROJECT_NAME} src/main.c)

# Link Raylib to your project
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Add this after your target_link_libraries
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    XCODE_ATTRIBUTE_CUSTOM_EXECUTABLE_ARGUMENTS "--working-dir ${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Setup includes so the compiler can find your files
target_include_directories(${PROJECT_NAME} PRIVATE src)