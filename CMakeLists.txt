cmake_minimum_required(VERSION 3.15)
project(SummerIDE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. External Dependencies (GLFW)
add_subdirectory(vendor/glfw)

# 2. Source Files (Explicitly listed to avoid CMake caching bugs)
set(SOURCES
    src/main.cpp
    src/globals.cpp
    src/MainMenu.cpp
    src/WelcomeScreen.cpp
    src/sidebar.cpp
    src/Terminal.cpp
    src/CodeEditor.cpp
)

# Vendor Sources (Globbing here is fine since these third-party libraries rarely change)
file(GLOB IMGUI_SOURCES "vendor/imgui/*.cpp" "vendor/imgui/backends/imgui_impl_glfw.cpp" "vendor/imgui/backends/imgui_impl_opengl3.cpp")
file(GLOB TEXTEDIT_SOURCES "vendor/ImGuiColorTextEdit/*.cpp")

# 3. Create Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${TEXTEDIT_SOURCES})

# 4. Header Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    vendor/imgui
    vendor/imgui/backends
    vendor/glfw/include
    vendor/ImGuiColorTextEdit
)

# 5. Target Link Libraries (Cleaned up and combined)
if(WIN32)
  # On Windows, explicitly link opengl32
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw opengl32)
elseif(APPLE)
  # Mac framework linking
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX AND NOT APPLE)
  # Linux (Arch) OpenGL linking
  find_package(OpenGL REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL m)
endif()
