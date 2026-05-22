#include "globals.h"
#include <array>
#include <fstream>
#include <sstream>

Tab::Tab(const std::filesystem::path &path)
    : filepath(path), is_modified(false) {
  filename = path.filename().string();
  std::string content = ReadFileContent(path);
  editor.SetText(content);
  editor.SetLanguageDefinition(
      GetLanguageFromExtension(path.extension().string()));
}

std::vector<std::unique_ptr<Tab>> openTabs;
int activeTabIndex = -1;

std::filesystem::path current_path = std::filesystem::current_path();
std::vector<std::filesystem::path> selected_files;

float separatorPos = 250.0f;
bool isDraggingSeparator = false;
bool isDraggingTerminalSeparator = false;

std::vector<std::string> terminalLog;
char terminalInput[256] = "";

bool isCreatingNewFile = false;
char newFileNameBuffer[256] = "";
bool isCreatingNewFolder = false;
char newFolderNameBuffer[256] = "";

bool showWelcomeScreen = true;
bool showSidebar = true;
bool showTerminal = true;

float terminalHeight = 200.0f;
ImGuiWindowFlags shared_window_flags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

// System IO Utility Functions
std::string ReadFileContent(const std::filesystem::path &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open())
    return "";
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

TextEditor::LanguageDefinition
GetLanguageFromExtension(const std::string &ext) {
  if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cxx" ||
      ext == ".cc") {
    return TextEditor::LanguageDefinition::CPlusPlus();
  } else if (ext == ".c") {
    return TextEditor::LanguageDefinition::C();
  } else if (ext == ".glsl" || ext == ".frag" || ext == ".vert") {
    return TextEditor::LanguageDefinition::GLSL();
  } else if (ext == ".hlsl") {
    return TextEditor::LanguageDefinition::HLSL();
  } else if (ext == ".sql") {
    return TextEditor::LanguageDefinition::SQL();
  } else if (ext == ".lua") {
    return TextEditor::LanguageDefinition::Lua();
  } else if (ext == ".as") {
    return TextEditor::LanguageDefinition::AngelScript();
  } else {
    return TextEditor::LanguageDefinition::CPlusPlus();
  }
}

std::string ExecSystemCommand(const std::string &cmd) {
  std::array<char, 128> buffer;
  std::string result;

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

  std::string fullCmd = cmd + " 2>&1";
  std::unique_ptr<FILE, int (*)(FILE *)> pipe(POPEN(fullCmd.c_str(), "r"),
                                              PCLOSE);

  if (!pipe)
    return "Error: failed to open system pipe.";

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    result += buffer.data();

  return result;
}
