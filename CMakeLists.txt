cmake_minimum_required(VERSION 3.20)
project(Boundless LANGUAGES CXX VERSION 0.0.1)

set(EXPORT_COMPILE_COMMANDS ON)
set(ENV{VULKAN_SDK} "D:\\vulkanSDK")

aux_source_directory(./src SRC_FILE)
add_executable(main ${SRC_FILE})

find_package(Vulkan REQUIRED)
add_subdirectory("inc\\glfw-3.3.8")
target_link_libraries(main ${Vulkan_LIBRARIES})
target_link_libraries(main glfw)
target_include_directories(main PRIVATE inc/BL PRIVATE inc/ PUBLIC "D:\\vulkanSDK\\Include")

target_compile_features(main PRIVATE cxx_std_20)