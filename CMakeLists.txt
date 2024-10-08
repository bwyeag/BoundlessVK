cmake_minimum_required(VERSION 3.20)
# set(CMAKE_CXX_COMPILER "D:\\msys2\\mingw64\\bin\\g++.exe")
# set(CMAKE_GENERATOR "MinGW Makefiles")
project(Boundless LANGUAGES CXX VERSION 0.1.0)

set(EXPORT_COMPILE_COMMANDS ON)
set(ENV{VULKAN_SDK} "D:\\vulkanSDK")

# aux_source_directory(./src SRC_FILE)
set(SRC_FILE ./src/main.cpp ./src/bl_context.cpp ./src/vma.cpp ./src/bl_log.cpp ./src/bl_render.cpp)
aux_source_directory(./src/imgui IMGUI_FILE)
add_executable(main ${SRC_FILE} ${IMGUI_FILE})

set(ZLIB_ROOT "D:\\c++programs\\zlib-1.3.1\\")
find_package(ZLIB REQUIRED)
target_link_libraries(main ZLIB::ZLIB)

include_directories("D:\\c++programs\\eigen-3.4.0\\Eigen" ".\\inc\\imgui")

find_package(Vulkan REQUIRED)
set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
add_subdirectory("inc\\glfw-3.3.8")

target_link_libraries(main ${Vulkan_LIBRARIES})
target_link_libraries(main glfw)
target_include_directories(main PRIVATE inc/BL PRIVATE inc/ PUBLIC "D:\\vulkanSDK\\Include")

target_compile_features(main PRIVATE cxx_std_20)
add_compile_definitions(BL_DEBUG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Og")