#pragma once
#include "TextEditor.h"
#include "imgui.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct Tab {
  std::filesystem::path filepath;
  std::string filename;
  TextEditor editor;
  bool is_modified;

  Tab(const std::filesystem::path &path);
};

extern std::vector<std::unique_ptr<Tab>> openTabs;
extern int activeTabIndex;

extern std::filesystem::path current_path;
extern std::vector<std::filesystem::path> selected_files;

extern float separatorPos;
extern bool isDraggingSeparator;
extern bool isDraggingTerminalSeparator;

extern std::vector<std::string> terminalLog;
extern char terminalInput[256];

extern bool isCreatingNewFile;
extern char newFileNameBuffer[256];
extern bool isCreatingNewFolder;
extern char newFolderNameBuffer[256];

extern bool showWelcomeScreen;
extern bool showTerminal;
extern bool showSidebar;

extern float terminalHeight;
extern ImGuiWindowFlags shared_window_flags;

std::string ReadFileContent(const std::filesystem::path &filepath);
TextEditor::LanguageDefinition GetLanguageFromExtension(const std::string &ext);
std::string ExecSystemCommand(const std::string &cmd);
