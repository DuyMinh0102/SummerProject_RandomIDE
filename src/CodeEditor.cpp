#include "CodeEditor.h"
#include "globals.h"
#include "imgui.h"
using namespace ImGui;

void CodeEditorFunc() {
  if (showWelcomeScreen)
    return;

  const ImGuiViewport *viewport = GetMainViewport();

  float currentSidebarWidth = showSidebar ? separatorPos : 0.0f;
  float currentTerminalHeight = showTerminal ? terminalHeight : 0.0f;

  SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + currentSidebarWidth, viewport->WorkPos.y));
  SetNextWindowSize(ImVec2(viewport->WorkSize.x - currentSidebarWidth,
                           viewport->WorkSize.y - currentTerminalHeight));

  Begin("Code Editor Background", nullptr, shared_window_flags);

  if (!openTabs.empty()) {
    if (BeginTabBar("##TabBar", ImGuiTabBarFlags_Reorderable |
                                    ImGuiTabBarFlags_AutoSelectNewTabs)) {
      for (size_t i = 0; i < openTabs.size(); i++) {
        bool is_open = true;
        std::string tab_label = openTabs[i]->filename;

        if (openTabs[i]->is_modified) {
          tab_label += " *";
        }
        tab_label += "  ";

        if (BeginTabItem(tab_label.c_str(), &is_open, ImGuiTabItemFlags_None)) {
          activeTabIndex = i;
          EndTabItem();
        }

        if (!is_open) {
          openTabs.erase(openTabs.begin() + i);
          if (activeTabIndex >= (int)openTabs.size()) {
            activeTabIndex = openTabs.size() - 1;
          }
          if (openTabs.empty()) {
            activeTabIndex = -1;
            showWelcomeScreen = true;
          }
          i--;
        }
      }
      EndTabBar();
    }

    Separator();

    if (activeTabIndex >= 0 && activeTabIndex < (int)openTabs.size()) {
      openTabs[activeTabIndex]->editor.Render("TextEditor");
    }
  }
  End();
}
