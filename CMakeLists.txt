cmake_minimum_required(VERSION 3.15)

# Projection Name and Version
project(TTengine VERSION 1.0
    DESCRIPTION "Vulkan Engine")


# Compiler Options
set(CMAKE_CXX_STANDARD 20)
# add_compile_options(-Wall -Wextra -Wpedantic)
# add_link_options(-Wall -Wextra -Wpedantic)
# set_source_files_properties(
#     ${PROJECT_SOURCE_DIR}/src/structs_vk.hpp
#     ${PROJECT_SOURCE_DIR}/src/vk_mem_alloc.h
#     PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wpedantic"
# )


#find all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/volk/*.h)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
add_subdirectory(volk)
add_subdirectory(vk-bootstrap)
add_subdirectory(SPIRV-Cross)

add_executable(${PROJECT_NAME} ${SOURCES})
#set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC glfw vulkan volk vk-bootstrap::vk-bootstrap spirv-cross-cpp)
target_include_directories(${PROJECT_NAME} PUBLIC volk)


add_library(TTengineApp ${SOURCES})
target_link_libraries(TTengineApp PUBLIC glfw vulkan volk vk-bootstrap::vk-bootstrap spirv-cross-cpp)
target_include_directories(TTengineApp PUBLIC volk)
