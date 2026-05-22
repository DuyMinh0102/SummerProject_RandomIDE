#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "globals.h"

// Components
#include "CodeEditor.h"
#include "MainMenu.h"
#include "Terminal.h"
#include "WelcomeScreen.h"
#include "sidebar.h"

using namespace ImGui;

int main() {
  if (!glfwInit())
    return -1;

  GLFWwindow *window = glfwCreateWindow(1280, 720, "My ImGui IDE", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Initialize Dear ImGui
  IMGUI_CHECKVERSION();
  CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  // Initialize Terminal Log Defaults
  terminalLog.push_back("IDE Terminal initialized...");
  terminalLog.push_back("Type 'help' for a list of commands.");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    NewFrame();

    // --- Execute Modular Element Render Pipelines ---
    MainMenuBarFunc(window);
    WelcomeScreenFunc();
    SidebarFunc();
    TerminalFunc();
    CodeEditorFunc();

    // Finalize standard pipeline graphics rendering layout commands
    Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

    glfwSwapBuffers(window);
  }

  // Engine Pipeline Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
