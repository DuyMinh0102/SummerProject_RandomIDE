#include "sidebar.h"
#include "globals.h"
#include "imgui.h"
#include <fstream>
using namespace ImGui;
void SidebarFunc() {
  if (!showSidebar)
    return;

  ImGuiIO &io = GetIO();
  const ImGuiViewport *viewport = GetMainViewport();

  float separatorX = viewport->WorkPos.x + separatorPos;
  bool isNearSeparator =
      (io.MousePos.x >= separatorX - 4 && io.MousePos.x <= separatorX + 4);

  if (isNearSeparator || isDraggingSeparator) {
    SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    if (IsMouseDown(ImGuiMouseButton_Left)) {
      isDraggingSeparator = true;
      separatorPos = io.MousePos.x - viewport->WorkPos.x;

      if (separatorPos < 100.0f)
        separatorPos = 100.0f;
      if (separatorPos > viewport->WorkSize.x - 175.0f)
        separatorPos = viewport->WorkSize.x - 175.0f;
    }
  }
  if (IsMouseReleased(ImGuiMouseButton_Left)) {
    isDraggingSeparator = false;
  }

  SetNextWindowPos(viewport->WorkPos);
  SetNextWindowSize(ImVec2(separatorPos, viewport->WorkSize.y));

  ImGuiWindowFlags sidebar_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

  Begin("File Explorer", nullptr, sidebar_flags);

  if (Button("Up")) {
    if (current_path.has_parent_path()) {
      current_path = current_path.parent_path();
    }
  }
  SameLine();
  if (Button("New File")) {
    isCreatingNewFile = true;
    newFileNameBuffer[0] = '\0';
  }
  SameLine();
  Text(current_path.string().c_str());

  Separator();

  if (BeginPopupContextWindow("##empty_space_context",
                              ImGuiPopupFlags_NoOpenOverItems |
                                  ImGuiPopupFlags_MouseButtonRight)) {
    if (MenuItem("New File")) {
      isCreatingNewFile = true;
      newFileNameBuffer[0] = '\0';
    }
    if (MenuItem("New Folder")) {
      isCreatingNewFolder = true;
      newFolderNameBuffer[0] = '\0';
    }
    EndPopup();
  }

  try {
    if (isCreatingNewFile) {
      PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
      Text("[FILE] ");
      SameLine();
      SetKeyboardFocusHere();
      if (InputText("##newfilename", newFileNameBuffer, 256,
                    ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (strlen(newFileNameBuffer) > 0) {
          std::filesystem::path new_file_path =
              current_path / newFileNameBuffer;
          std::ofstream new_file(new_file_path);
          if (new_file.is_open()) {
            new_file.close();
            openTabs.push_back(std::make_unique<Tab>(new_file_path));
            activeTabIndex = openTabs.size() - 1;
            showWelcomeScreen = false;
          }
        }
        isCreatingNewFile = false;
      }
      PopStyleColor();

      if (IsKeyPressed(ImGuiKey_Escape) ||
          (IsMouseClicked(ImGuiMouseButton_Left) && !IsItemActive())) {
        isCreatingNewFile = false;
        if (openTabs.empty())
          showWelcomeScreen = true;
      }
    }

    if (isCreatingNewFolder) {
      PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
      Text("[DIR] ");
      SameLine();
      SetKeyboardFocusHere();
      if (InputText("##newfoldername", newFolderNameBuffer, 256,
                    ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (strlen(newFolderNameBuffer) > 0) {
          std::filesystem::path new_folder_path =
              current_path / newFolderNameBuffer;
          std::filesystem::create_directory(new_folder_path);
        }
        isCreatingNewFolder = false;
      }
      PopStyleColor();

      if (IsKeyPressed(ImGuiKey_Escape) ||
          (IsMouseClicked(ImGuiMouseButton_Left) && !IsItemActive())) {
        isCreatingNewFolder = false;
      }
    }

    for (const auto &entry :
         std::filesystem::directory_iterator(current_path)) {
      std::string filename = entry.path().filename().string();
      bool is_directory = entry.is_directory();

      if (is_directory) {
        PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        if (Selectable(("[DIR] " + filename).c_str())) {
          current_path = entry.path();
        }
        if (BeginPopupContextItem()) {
          if (MenuItem("Delete")) {
            try {
              std::filesystem::remove_all(entry.path());
            } catch (...) {
            }
          }
          EndPopup();
        }
        PopStyleColor();
      } else {
        PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        if (Selectable(("[FILE] " + filename).c_str())) {
          bool alreadyOpen = false;
          int existingTabIndex = -1;

          for (size_t i = 0; i < openTabs.size(); i++) {
            if (openTabs[i]->filepath == entry.path()) {
              alreadyOpen = true;
              existingTabIndex = i;
              break;
            }
          }

          if (alreadyOpen) {
            activeTabIndex = existingTabIndex;
          } else {
            openTabs.push_back(std::make_unique<Tab>(entry.path()));
            activeTabIndex = openTabs.size() - 1;
          }
          showWelcomeScreen = false;
        }
        if (BeginPopupContextItem()) {
          if (MenuItem("Delete")) {
            try {
              std::filesystem::remove(entry.path());
            } catch (...) {
            }
          }
          EndPopup();
        }
        PopStyleColor();
      }
    }
  } catch (...) {
    Text("Error reading directory");
  }

  End();
}
