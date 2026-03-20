cmake_minimum_required(VERSION 3.20)
project(DroneTelemetryRenderer)

# 1. Use C++20 for modern math features (required for your 3D renderer)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Configure Python 3.10
set(Python3_ROOT_DIR "C:/Python")
find_package(Python3 3.10 EXACT COMPONENTS Interpreter Development REQUIRED)

# 3. Include Directories
# Includes your project headers, SDL3 headers, and Python headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories("C:/SDL3-3.4.0/x86_64-w64-mingw32/include")
include_directories(${Python3_INCLUDE_DIRS})

# 4. Library Directories (where .a / .lib files are)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
link_directories("C:/SDL3-3.4.0/x86_64-w64-mingw32/lib")

# 5. Create the Executive
# Combining all your source files into one project
add_executable(DroneApp 
    src/main.cpp 
    src/pythonManager.cpp 
    src/render_engine.cpp 
    src/sdl_engine.cpp
)

# 6. Link Libraries
# Link both SDL3 and the Python libraries found by find_package
target_link_libraries(DroneApp PRIVATE SDL3 ${Python3_LIBRARIES})

# 7. Post-Build: Copy DLLs to the build folder
# This ensures both SDL3.dll and python310.dll are next to your .exe
add_custom_command(TARGET DroneApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "C:/SDL3-3.4.0/x86_64-w64-mingw32/bin/SDL3.dll"
    $<TARGET_FILE_DIR:DroneApp>
)

add_custom_command(TARGET DroneApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "C:/Python/python310.dll"
    $<TARGET_FILE_DIR:DroneApp>
)