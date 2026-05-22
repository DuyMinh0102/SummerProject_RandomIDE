#include "Terminal.h"
#include "globals.h"
#include "imgui.h"
#include <cstring>
#include <sstream>
using namespace ImGui;

void TerminalFunc() {
  if (!showTerminal)
    return;

  ImGuiIO &io = GetIO();
  const ImGuiViewport *viewport = GetMainViewport();

  float currentSidebarWidth = showSidebar ? separatorPos : 0.0f;
  float terminalYPos =
      viewport->WorkPos.y + viewport->WorkSize.y - terminalHeight;

  SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + currentSidebarWidth, terminalYPos));
  SetNextWindowSize(
      ImVec2(viewport->WorkSize.x - currentSidebarWidth, terminalHeight));

  Begin("Terminal", nullptr, shared_window_flags);

  const float footerHeightToReserve =
      GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

  if (BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false,
                 ImGuiWindowFlags_HorizontalScrollbar)) {
    for (const std::string &line : terminalLog) {
      if (line.rfind("> ", 0) == 0) {
        PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
        TextUnformatted(line.c_str());
        PopStyleColor();
      } else {
        TextUnformatted(line.c_str());
      }
    }

    if (GetScrollY() >= ImGui::GetScrollMaxY()) {
      SetScrollHereY(1.0f);
    }
  }
  EndChild();
  Separator();

  bool reclaimFocus = false;
  PushItemWidth(-1);

  if (InputText("##terminalInput", terminalInput, IM_ARRAYSIZE(terminalInput),
                ImGuiInputTextFlags_EnterReturnsTrue)) {
    std::string command = terminalInput;

    if (!command.empty()) {
      terminalLog.push_back("> " + command);

      if (command == "clear") {
        terminalLog.clear();
      } else if (command.rfind("cd ", 0) == 0) {
        std::string newDir = command.substr(3);
        try {
          std::filesystem::current_path(newDir);
          current_path = std::filesystem::current_path();
          terminalLog.push_back(current_path.string());
        } catch (const std::exception &e) {
          terminalLog.push_back(std::string("cd error: ") + e.what());
        }
      } else {
        std::string output = ExecSystemCommand(command);
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
          if (!line.empty() && line.back() == '\n')
            line.pop_back();
          terminalLog.push_back(line);
        }
      }
    }

    strcpy(terminalInput, "");
    reclaimFocus = true;
  }
  PopItemWidth();

  SetItemDefaultFocus();
  if (reclaimFocus) {
    SetKeyboardFocusHere(-1);
  }
  End();

  bool isNearTerminalSep =
      (io.MousePos.y >= terminalYPos - 4 && io.MousePos.y <= terminalYPos + 4);

  if (isNearTerminalSep || isDraggingTerminalSeparator) {
    SetMouseCursor(ImGuiMouseCursor_ResizeNS);

    if (IsMouseDown(ImGuiMouseButton_Left)) {
      isDraggingTerminalSeparator = true;

      terminalHeight =
          (viewport->WorkPos.y + viewport->WorkSize.y) - io.MousePos.y;

      if (terminalHeight < 50.0f)
        terminalHeight = 50.0f;
      if (terminalHeight > viewport->WorkSize.y - 350.0f)
        terminalHeight = viewport->WorkSize.y - 350.0f;
    }
  }
  if (IsMouseReleased(ImGuiMouseButton_Left)) {
    isDraggingTerminalSeparator = false;
  }
}
