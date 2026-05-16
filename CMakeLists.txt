cmake_minimum_required(VERSION 3.15)
project(SummerIDE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(vendor/glfw)

# 1. Grab only the core ImGui files (Notice we use GLOB, not GLOB_RECURSE!)
file(GLOB IMGUI_CORE CONFIGURE_DEPENDS "vendor/imgui/*.cpp" "vendor/imgui/*.h")

# 2. Explicitly grab ONLY the two backends you actually use
set(IMGUI_BACKENDS
    "vendor/imgui/backends/imgui_impl_glfw.cpp"
    "vendor/imgui/backends/imgui_impl_glfw.h"
    "vendor/imgui/backends/imgui_impl_opengl3.cpp"
    "vendor/imgui/backends/imgui_impl_opengl3.h"
)

# 3. Combine them
set(IMGUI_SOURCES ${IMGUI_CORE} ${IMGUI_BACKENDS})
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS "vendor/imgui/*.cpp" "vendor/imgui/*.h")
file(GLOB_RECURSE TEXTEDIT_SOURCES CONFIGURE_DEPENDS "vendor/ImGuiColorTextEdit/*.cpp" "vendor/ImGuiColorTextEdit/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${TEXTEDIT_SOURCES})

# headers
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    vendor/imgui
    vendor/imgui/backends
    vendor/glfw/include
    vendor/ImGuiColorTextEdit
)

# Link the libraries and handle OS-specific OpenGL setup
if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw opengl32)
elseif(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX AND NOT APPLE)
  find_package(OpenGL REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL m)
endif()
