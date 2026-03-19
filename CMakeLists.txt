cmake_minimum_required(VERSION 3.11)
project(MyRaylibGame C CXX)

# Tell CMake to look in our local 'raylib' folder and build it
add_subdirectory(raylib)

# Create the executable
add_executable(${PROJECT_NAME} src/main.c)

# Link Raylib to your project
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Setup includes so the compiler can find your files
target_include_directories(${PROJECT_NAME} PRIVATE src)