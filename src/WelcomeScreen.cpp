#include "WelcomeScreen.h"
#include "globals.h"
#include "imgui.h"
#include "portable-file-dialogs.h"
using namespace ImGui;

void WelcomeScreenFunc() {
  if (!showWelcomeScreen)
    return;

  ImGuiIO &io = GetIO();
  const ImGuiViewport *viewport = GetMainViewport();

  float currentSidebarWidth = showSidebar ? separatorPos : 0.0f;
  float currentTerminalHeight = showTerminal ? terminalHeight : 0.0f;

  SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + currentSidebarWidth, viewport->WorkPos.y));
  SetNextWindowSize(ImVec2(viewport->WorkSize.x - currentSidebarWidth,
                           viewport->WorkSize.y - currentTerminalHeight));

  ImGuiWindowFlags welcome_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

  Begin("Welcome Screen", nullptr, welcome_flags);

  ImVec2 windowSize = GetWindowSize();
  ImVec2 centerPos = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);

  PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

  SetNextItemWidth(centerPos.x);
  SetCursorPosX(centerPos.x - ImGui::CalcTextSize("Welcome").x * 0.5f);
  SetCursorPosY(centerPos.y - 150);
  PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
  Text("Welcome");
  PopStyleColor();

  SetCursorPosX(centerPos.x - CalcTextSize("RandomIDE - Start").x * 0.5f);
  SetCursorPosY(centerPos.y - 120);
  PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
  Text("RandomIDE - Start");
  PopStyleColor();

  PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
  PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
  PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

  float button_width = 200;
  SetCursorPosX(centerPos.x - button_width * 0.5f);
  SetCursorPosY(centerPos.y - 50);
  if (Button("New File", ImVec2(button_width, 40))) {
    isCreatingNewFile = true;
    newFileNameBuffer[0] = '\0';
    showWelcomeScreen = false;
  }

  SetCursorPosX(centerPos.x - button_width * 0.5f);
  SetCursorPosY(centerPos.y + 10);
  if (Button("Open File", ImVec2(button_width, 40))) {
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

  SetCursorPosX(centerPos.x - button_width * 0.5f);
  SetCursorPosY(centerPos.y + 70);
  if (Button("Open Folder", ImVec2(button_width, 40))) {
    auto folder = pfd::select_folder("Open Folder", ".").result();
    if (!folder.empty()) {
      current_path = folder;
      showWelcomeScreen = false;
    }
  }

  PopStyleColor(3);
  PopStyleColor();
  End();
}
