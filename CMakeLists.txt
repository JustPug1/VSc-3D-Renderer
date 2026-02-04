cmake_minimum_required(VERSION 3.20)
project("VSC 3D RENDERER")

# Use C++20 for modern math features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Include paths (where the .h files are)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. Library paths (where libSDL3.dll.a is)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 3. Create the executable
add_executable(VSC_Renderer src/main.cpp src/render_engine.cpp src/sdl_engine.cpp)

# 4. Link SDL3
target_link_libraries(VSC_Renderer PRIVATE SDL3)

# 5. Copy the DLL to the build folder so it runs
add_custom_command(TARGET VSC_Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/SDL3.dll"
    $<TARGET_FILE_DIR:VSC_Renderer>
)