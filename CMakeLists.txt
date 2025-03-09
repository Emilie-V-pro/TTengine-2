cmake_minimum_required(VERSION 3.15)

# Projet
project(TTengine VERSION 1.0 DESCRIPTION "Vulkan Engine")

# Compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver Vulkan et GLFW
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenMP REQUIRED)

# Ajouter les sous-modules nécessaires
add_subdirectory(volk)
add_subdirectory(vk-bootstrap)
add_subdirectory(SPIRV-Cross)

# Trouver tous les fichiers sources
file(GLOB_RECURSE TTENGINE_SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.c
)

# Définition de la bibliothèque statique TTengine
add_library(TTengine STATIC ${TTENGINE_SOURCES})

add_library(imgui STATIC
${PROJECT_SOURCE_DIR}/imgui/imgui.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_demo.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_draw.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_tables.cpp
${PROJECT_SOURCE_DIR}/imgui/imgui_widgets.cpp
${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
${PROJECT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
    ${PROJECT_SOURCE_DIR}/imgui
    ${PROJECT_SOURCE_DIR}/imgui/backends
    ${PROJECT_SOURCE_DIR}/volk
)

# Inclure les headers dans la bibliothèque
target_include_directories(TTengine PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/volk
    ${PROJECT_SOURCE_DIR}/imgui
    ${PROJECT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    Vulkan::Vulkan
    volk
)

# Lier les dépendances à la bibliothèque
target_link_libraries(TTengine PUBLIC
    glfw
    Vulkan::Vulkan
    volk
    vk-bootstrap::vk-bootstrap
    spirv-cross-cpp
    OpenMP::OpenMP_CXX
    imgui
)

# Ajouter un exécutable de test (optionnel)
option(TTENGINE_BUILD_EXAMPLES "Build TTengine example application" ON)

if(TTENGINE_BUILD_EXAMPLES)
    add_executable(TTengineApp ${TTENGINE_SOURCES})

    target_link_libraries(TTengineApp PRIVATE TTengine)

    # Définir un répertoire de travail pour VS Code / Visual Studio
    set_property(TARGET TTengineApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()
