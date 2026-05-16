cmake_minimum_required(VERSION 3.15)
project(SummerIDE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(vendor/glfw)

# src
file(GLOB SOURCES "src/*.cpp")
file(GLOB IMGUI_SOURCES "vendor/imgui/*.cpp" "vendor/imgui/backends/imgui_impl_glfw.cpp" "vendor/imgui/backends/imgui_impl_opengl3.cpp")
file(GLOB TEXTEDIT_SOURCES "vendor/ImGuiColorTextEdit/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${TEXTEDIT_SOURCES})

# headers
target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    vendor/imgui
    vendor/imgui/backends
    vendor/glfw/include
    vendor/ImGuiColorTextEdit
)

# lib
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL m)
endif()