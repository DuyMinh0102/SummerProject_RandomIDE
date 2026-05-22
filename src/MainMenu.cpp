#include "MainMenu.h"
#include "globals.h"
#include "imgui.h"
#include "portable-file-dialogs.h"
#include <fstream>
using namespace ImGui;

void MainMenuBarFunc(GLFWwindow *window) {
  ImGuiIO &io = GetIO();

  if (BeginMainMenuBar()) {
    if (BeginMenu("File")) {
      if (MenuItem("Open", "Ctrl + O")) {
        auto selection =
            pfd::open_file("Open File", ".",
                           {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"})
                .result();

        if (!selection.empty()) {
          std::filesystem::path objPath(selection[0]);
          bool alreadyOpen = false;
          int existingTabIndex = -1;

          for (size_t i = 0; i < openTabs.size(); i++) {
            if (openTabs[i]->filepath == objPath) {
              alreadyOpen = true;
              existingTabIndex = i;
              break;
            }
          }

          if (alreadyOpen) {
            activeTabIndex = existingTabIndex;
          } else {
            openTabs.push_back(std::make_unique<Tab>(objPath));
            activeTabIndex = openTabs.size() - 1;
          }

          current_path = objPath.parent_path();
          showWelcomeScreen = false;
        }
      }
      if (MenuItem("Save", "Ctrl + S")) {
        if (activeTabIndex >= 0 && activeTabIndex < (int)openTabs.size()) {
          auto &tab = openTabs[activeTabIndex];
          std::string currentText = tab->editor.GetText();

          if (tab->filepath.empty()) {
            auto destination =
                pfd::save_file("Save File", ".", {"All Files", " * "}).result();

            if (!destination.empty()) {
              tab->filepath = destination;
              std::ofstream outFile(destination);
              if (outFile.is_open()) {
                outFile << currentText;
                outFile.close();
                tab->is_modified = false;
              }
            }
          } else {
            std::ofstream outFile(tab->filepath);
            if (outFile.is_open()) {
              outFile << currentText;
              outFile.close();
              tab->is_modified = false;
            }
          }
        }
      }
      if (MenuItem("Save As", "Ctrl + Shift + S")) {
        if (activeTabIndex >= 0 && activeTabIndex < (int)openTabs.size()) {
          auto &tab = openTabs[activeTabIndex];
          std::string currentText = tab->editor.GetText();
          auto destination =
              pfd::save_file("Save File", ".", {"All Files", "*"}).result();

          if (!destination.empty()) {
            tab->filepath = destination;
            std::ofstream outFile(destination);
            if (outFile.is_open()) {
              outFile << currentText;
              outFile.close();
              tab->is_modified = false;
            }
          }
        }
      }
      Separator();
      if (MenuItem("Exit", "Alt+F4")) {
        glfwSetWindowShouldClose(window, true);
      }
      EndMenu();
    }

    if (BeginMenu("View")) {
      Separator();

      if (MenuItem("Zoom In", "Ctrl + =")) {
        if (io.FontGlobalScale <= 10.0f)
          io.FontGlobalScale += 0.1f;
      }
      if (MenuItem("Zoom Out", "Ctrl + -")) {
        if (io.FontGlobalScale >= 0.5f)
          io.FontGlobalScale -= 0.1f;
      }

      EndMenu();
    }

    if (MenuItem("Sidebar")) {
      showSidebar = !showSidebar;
    }
    if (MenuItem("Terminal")) {
      showTerminal = !showTerminal;
    }

    if (io.KeyCtrl) {
      if (IsKeyPressed(ImGuiKey_Equal)) {
        if (io.FontGlobalScale < 10.0f)
          io.FontGlobalScale += 0.1f;
      }
      if (IsKeyPressed(ImGuiKey_Minus)) {
        if (io.FontGlobalScale > 0.5f)
          io.FontGlobalScale -= 0.1f;
      }
      if (IsKeyPressed(ImGuiKey_0))
        io.FontGlobalScale = 1.0f;

      if (IsKeyPressed(ImGuiKey_O)) {
        auto selection =
            pfd::open_file("Open File", ".",
                           {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"})
                .result();

        if (!selection.empty()) {
          std::filesystem::path objPath(selection[0]);
          bool alreadyOpen = false;
          int existingTabIndex = -1;

          for (size_t i = 0; i < openTabs.size(); i++) {
            if (openTabs[i]->filepath == objPath) {
              alreadyOpen = true;
              existingTabIndex = i;
              break;
            }
          }

          if (alreadyOpen) {
            activeTabIndex = existingTabIndex;
          } else {
            openTabs.push_back(std::make_unique<Tab>(objPath));
            activeTabIndex = openTabs.size() - 1;
          }

          current_path = objPath.parent_path();
          showWelcomeScreen = false;
        }
      }
      if (IsKeyPressed(ImGuiKey_S)) {
        if (activeTabIndex >= 0 && activeTabIndex < (int)openTabs.size()) {
          std::string currentText = openTabs[activeTabIndex]->editor.GetText();
          std::ofstream outFile(openTabs[activeTabIndex]->filepath);
          if (outFile.is_open()) {
            outFile << currentText;
            outFile.close();
            openTabs[activeTabIndex]->is_modified = false;
          }
        }
      }
    }
    if (io.KeyAlt) {
      if (IsKeyPressed(ImGuiKey_F4))
        glfwSetWindowShouldClose(window, true);
    }

    EndMainMenuBar();
  }
}
