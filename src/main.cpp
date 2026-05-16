#include <iostream>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "TextEditor.h"
#include "portable-file-dialogs.h"
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <array>
#include <memory>

// Helper function to read file content
std::string ReadFileContent(const std::filesystem::path& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper function to get language definition based on file extension
TextEditor::LanguageDefinition GetLanguageFromExtension(const std::string& ext) {
    if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cxx" || ext == ".cc") {
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

// Tab structure to hold file information
struct Tab {
    std::filesystem::path filepath;
    std::string filename;
    TextEditor editor;
    bool is_modified;
    
    Tab(const std::filesystem::path& path) : filepath(path), is_modified(false) {
        filename = path.filename().string();
        std::string content = ReadFileContent(path);
        editor.SetText(content);
        editor.SetLanguageDefinition(GetLanguageFromExtension(path.extension().string()));
    }
};

std::string ExecSystemCommand(const std::string& cmd){
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

    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(fullCmd.c_str(), "r"), PCLOSE);
    if(!pipe){
        return "Error: failed to open system pipe.";
    }

    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

        
    return result;
}

int main() {

    // 1. Initialize Windowing (GLFW) & Graphics Context
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "My ImGui IDE", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 2. Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 3. Init editor component
    TextEditor editor;
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetText("// Write your C++ code here!\n\nint main() {\n    return 0;\n}");

    // Tab management
    std::vector<std::unique_ptr<Tab>> openTabs;
    int active_tab_index = -1;

    // File explorer state
    std::filesystem::path current_path = std::filesystem::current_path();
    std::vector<std::filesystem::path> selected_files;

    // Separator state (initial sidebar width)
    float separatorPos = 250.0f;
    bool is_dragging_separator = false;

    // Terminal state
    std::vector<std::string> terminalLog;
    char terminalInput[256] = "";
    terminalLog.push_back("IDE Terminal initialized...");
    terminalLog.push_back("Type 'help' for a list of commands.");
    // New file inline creation state
    bool isCreatingNewFile = false;
    char newFileNameBuffer[256] = "";

    // New folder inline creation state
    bool isCreatingNewFolder = false;
    char newFolderNameBuffer[256] = "";

    // Context menu state
    bool showEmptySpaceContextMenu = false;
    bool showItemContextMenu = false;
    std::filesystem::path selectedItemPath;
    bool selectedItemIsDir = false;

    // Welcome screen state
    bool showWelcomeScreen = true;

    // 4. The Application / Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Catch keypresses and mouse movement

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        // 5. Build the UI

        // Main Menu Bar
        if (ImGui::BeginMainMenuBar()){
            // Open/Save file
            if (ImGui::BeginMenu("File")){
                if (ImGui::MenuItem("Open", "Ctrl + O")) {
                    auto selection = pfd::open_file("Open File", ".", {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"}).result();

                    if (!selection.empty()){
                        std::string filepath = selection[0];
                        std::filesystem::path objPath(filepath);
                        
                        // Check if file is already open in tabs
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
                            // Switch to existing tab
                            active_tab_index = existingTabIndex;
                        } else {
                            // Create new tab
                            openTabs.push_back(std::make_unique<Tab>(objPath));
                            active_tab_index = openTabs.size() - 1;
                        }
                        
                        current_path = objPath.parent_path();
                        showWelcomeScreen = false;
                    }
                }
                if (ImGui::MenuItem("Save", "Ctrl + S")) {
                    if (active_tab_index >= 0 && active_tab_index < (int)openTabs.size()) {
                        std::string currentText = openTabs[active_tab_index]->editor.GetText();
                        std::ofstream outFile(openTabs[active_tab_index]->filepath);
                        if (outFile.is_open()) {
                            outFile << currentText;
                            outFile.close();
                            openTabs[active_tab_index]->is_modified = false;
                        }
                    }
                }
                ImGui::Separator();
                if(ImGui::MenuItem("Exit", "Alt+F4")) {glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }

            // Zoom In/Out
            if (ImGui::BeginMenu("View")){
                if (ImGui::MenuItem("Zoom In", "Ctrl + =")) { 
                    if (io.FontGlobalScale <= 10.0f) io.FontGlobalScale += 0.1f; 
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl + -")) { 
                    if (io.FontGlobalScale >= 0.5f) io.FontGlobalScale -= 0.1f;
                }
                ImGui::EndMenu();
            }

            if (io.KeyCtrl) {
                if (ImGui::IsKeyPressed(ImGuiKey_Equal)) { 
                    if (io.FontGlobalScale < 10.0f) io.FontGlobalScale += 0.1f;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Minus)) {
                    if (io.FontGlobalScale > 0.5f) io.FontGlobalScale -= 0.1f;
                }
                if(ImGui::IsKeyPressed(ImGuiKey_0)) io.FontGlobalScale = 1.0f;
                if(ImGui::IsKeyPressed(ImGuiKey_O)){
                    auto selection = pfd::open_file("Open File", ".", {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"}).result();

                    if (!selection.empty()){
                        std::string filepath = selection[0];
                        std::filesystem::path objPath(filepath);
                        
                        // Check if file is already open in tabs
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
                            // Switch to existing tab
                            active_tab_index = existingTabIndex;
                        } else {
                            // Create new tab
                            openTabs.push_back(std::make_unique<Tab>(objPath));
                            active_tab_index = openTabs.size() - 1;
                        }
                        
                        current_path = objPath.parent_path();
                        showWelcomeScreen = false;
                    }
                }
                if(ImGui::IsKeyPressed(ImGuiKey_S)){
                    if (active_tab_index >= 0 && active_tab_index < (int)openTabs.size()) {
                        std::string currentText = openTabs[active_tab_index]->editor.GetText();
                        std::ofstream outFile(openTabs[active_tab_index]->filepath);
                        if (outFile.is_open()) {
                            outFile << currentText;
                            outFile.close();
                            openTabs[active_tab_index]->is_modified = false;
                        }
                    }
                }
            }
            if (io.KeyAlt){
                if (ImGui::IsKeyPressed(ImGuiKey_F4)) glfwSetWindowShouldClose(window, true);
            }

            ImGui::EndMainMenuBar();
        }

        // Welcome Screen (VSCode-like)
        if (showWelcomeScreen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separatorPos, viewport->WorkPos.y));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separatorPos, viewport->WorkSize.y - 200.0f));
            ImGuiWindowFlags welcome_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
            
            ImGui::Begin("Welcome Screen", nullptr, welcome_flags);
            
            // Center content
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 center_pos = ImVec2(window_size.x * 0.5f, window_size.y * 0.5f);
            
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            
            // Title
            ImGui::SetCursorPosX(center_pos.x - ImGui::CalcTextSize("Welcome").x * 0.5f);
            ImGui::SetCursorPosY(center_pos.y - 150);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::Text("Welcome");
            ImGui::PopStyleColor();
            
            // Subtitle
            ImGui::SetCursorPosX(center_pos.x - ImGui::CalcTextSize("RandomIDE - Start").x * 0.5f);
            ImGui::SetCursorPosY(center_pos.y - 120);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::Text("RandomIDE - Start");
            ImGui::PopStyleColor();
            
            // Button styling
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            
            // New File Button
            float button_width = 200;
            ImGui::SetCursorPosX(center_pos.x - button_width * 0.5f);
            ImGui::SetCursorPosY(center_pos.y - 50);
            if (ImGui::Button("New File", ImVec2(button_width, 40))) {
                isCreatingNewFile = true;
                newFileNameBuffer[0] = '\0';
                showWelcomeScreen = false;
            }
            
            // Open File Button
            ImGui::SetCursorPosX(center_pos.x - button_width * 0.5f);
            ImGui::SetCursorPosY(center_pos.y + 10);
            if (ImGui::Button("Open File", ImVec2(button_width, 40))) {
                auto selection = pfd::open_file("Open File", ".", {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"}).result();
                if (!selection.empty()) {
                    std::string filepath = selection[0];
                    std::filesystem::path objPath(filepath);
                    
                    // Check if file is already open in tabs
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
                        // Switch to existing tab
                        active_tab_index = existingTabIndex;
                    } else {
                        // Create new tab
                        openTabs.push_back(std::make_unique<Tab>(objPath));
                        active_tab_index = openTabs.size() - 1;
                    }
                    
                    current_path = objPath.parent_path();
                }
                showWelcomeScreen = false;
            }
            
            // Open Folder Button
            ImGui::SetCursorPosX(center_pos.x - button_width * 0.5f);
            ImGui::SetCursorPosY(center_pos.y + 70);
            if (ImGui::Button("Open Folder", ImVec2(button_width, 40))) {
                auto folder = pfd::select_folder("Open Folder", ".").result();
                if (!folder.empty()) {
                    current_path = folder;
                }
                showWelcomeScreen = false;
            }
            
            ImGui::PopStyleColor(3); // Pop button colors
            ImGui::PopStyleColor(); // Pop window bg
            
            ImGui::End();
        }

        // Sidebar Explorer
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(separatorPos, viewport->WorkSize.y));
        ImGuiWindowFlags sidebar_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        
        ImGui::Begin("File Explorer", nullptr, sidebar_flags);
        
        if (ImGui::Button("Up")) {
            if (current_path.has_parent_path()) {
                current_path = current_path.parent_path();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("New File")) {
            isCreatingNewFile = true;
            newFileNameBuffer[0] = '\0';
        }
        ImGui::SameLine();
        ImGui::Text(current_path.string().c_str());
        
        ImGui::Separator();

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow("##empty_space_context", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("New File")) {
                isCreatingNewFile = true;
                newFileNameBuffer[0] = '\0';
            }
            if (ImGui::MenuItem("New Folder")) {
                isCreatingNewFolder = true;
                newFolderNameBuffer[0] = '\0';
            }
            ImGui::EndPopup();
        }
        
        // List directories and files
        try {
            if (isCreatingNewFile) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                ImGui::Text("[FILE] ");
                ImGui::SameLine();
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##newfilename", newFileNameBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (strlen(newFileNameBuffer) > 0) {
                        std::filesystem::path new_file_path = current_path / newFileNameBuffer;
                        
                        // Create the file
                        std::ofstream new_file(new_file_path);
                        if (new_file.is_open()) {
                            new_file.close();
                            
                            // Open the new file in a tab
                            openTabs.push_back(std::make_unique<Tab>(new_file_path));
                            active_tab_index = openTabs.size() - 1;
                            showWelcomeScreen = false;
                        }
                    }
                    isCreatingNewFile = false;
                }
                ImGui::PopStyleColor();
                
                // Cancel on Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    isCreatingNewFile = false;
                }
                
                // Cancel on click outside the input box
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemActive()) {
                    isCreatingNewFile = false;
                }
            }

            // Show inline new folder input if creating new folder
            if (isCreatingNewFolder) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::Text("[DIR] ");
                ImGui::SameLine();
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##newfoldername", newFolderNameBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (strlen(newFolderNameBuffer) > 0) {
                        std::filesystem::path new_folder_path = current_path / newFolderNameBuffer;
                        
                        // Create the folder
                        std::filesystem::create_directory(new_folder_path);
                    }
                    isCreatingNewFolder = false;
                }
                ImGui::PopStyleColor();
                
                // Cancel on Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    isCreatingNewFolder = false;
                }
                
                // Cancel on click outside the input box
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemActive()) {
                    isCreatingNewFolder = false;
                }
            }
            
            for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
                std::string filename = entry.path().filename().string();
                bool is_directory = entry.is_directory();
                
                if (is_directory) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                    if (ImGui::Selectable(("[DIR] " + filename).c_str())) {
                        current_path = entry.path();
                    }
                    // Right-click context menu for directory
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete")) {
                            try {
                                std::filesystem::remove_all(entry.path());
                            } catch (const std::filesystem::filesystem_error& e) {
                                // Handle error silently
                            }
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                    if (ImGui::Selectable(("[FILE] " + filename).c_str())) {
                        // Check if file is already open in tabs
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
                            // Switch to existing tab
                            active_tab_index = existingTabIndex;
                        } else {
                            // Create new tab
                            openTabs.push_back(std::make_unique<Tab>(entry.path()));
                            active_tab_index = openTabs.size() - 1;
                        }
                        
                        // Hide welcome screen when opening a file
                        showWelcomeScreen = false;
                    }
                    // Right-click context menu for file
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete")) {
                            try {
                                std::filesystem::remove(entry.path());
                            } catch (const std::filesystem::filesystem_error& e) {
                                // Handle error silently
                            }
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor();
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            ImGui::Text("Error reading directory");
        }
        
        ImGui::End();

        // Handle separator dragging (check if mouse is near separator)
        float separator_x = viewport->WorkPos.x + separatorPos;
        bool is_near_separator = (io.MousePos.x >= separator_x - 4 && io.MousePos.x <= separator_x + 4);
        
        if (is_near_separator || is_dragging_separator) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                is_dragging_separator = true;
                separatorPos = io.MousePos.x - viewport->WorkPos.x;

                if (separatorPos < 100.0f) separatorPos = 100.0f;
                if (separatorPos > viewport->WorkSize.x - 100.0f) separatorPos = viewport->WorkSize.x - 100.0f;
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            is_dragging_separator = false;
        }

        // Terminal
        float terminalHeight = 200.0f;

        float terminalYPos = viewport->WorkPos.y + viewport->WorkSize.y - terminalHeight;

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separatorPos, terminalYPos));

        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separatorPos, terminalHeight));

        // --- C. Set strict flags to lock the window ---
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus; 
        window_flags |= ImGuiWindowFlags_NoNavFocus;     

        // Draw terminal
        ImGui::Begin("Terminal", nullptr, window_flags);
        
        const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar)){
            for (const std::string& line : terminalLog){
                if (line.rfind("> ", 0) == 0){
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                    ImGui::TextUnformatted(line.c_str());
                    ImGui::PopStyleColor();
                }
                else{
                    ImGui::TextUnformatted(line.c_str());
                }
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
        ImGui::Separator();

        bool reclaimFocus = false;
        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

        ImGui::PushItemWidth(-1);

        if(ImGui::InputText("##terminalInput", terminalInput, IM_ARRAYSIZE(terminalInput), inputFlags)){
            std::string command = terminalInput;

            if (!command.empty()){
                terminalLog.push_back("> " + command);

                if (command == "clear"){
                    terminalLog.clear();
                }
                else if(command.rfind("cd ", 0) == 0){
                    std::string newDir = command.substr(3);

                    try{
                        std::filesystem::current_path(newDir);

                        current_path= std::filesystem::current_path();
                        terminalLog.push_back(current_path.string());  
                    }
                    catch (const std::exception& e){
                        terminalLog.push_back(std::string("cd error: ") + e.what());
                    }
                }
                else{
                    std::string output = ExecSystemCommand(command);

                    std::istringstream stream(output);
                    std::string line;
                    while(std::getline(stream, line)){
                        if (!line.empty() && line.back() == '\n') line.pop_back();
                        terminalLog.push_back(line); 
                    }
                }
            }

            strcpy(terminalInput, "");
            reclaimFocus = true;
        }
        ImGui::PopItemWidth();

        ImGui::SetItemDefaultFocus();
        if (reclaimFocus){
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::End();

        // Tab bar and Editor window (takes remaining space)
        float tab_bar_height = 30.0f;
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separatorPos, viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separatorPos, viewport->WorkSize.y - terminalHeight));

        // --- D. Draw the locked Code Editor window ---
        ImGui::Begin("Code Editor Background", nullptr, window_flags);
        
        // Draw tab bar if there are open tabs
        if (!openTabs.empty()) {
            if (ImGui::BeginTabBar("##TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
                for (size_t i = 0; i < openTabs.size(); i++) {
                    bool is_open = true;
                    std::string tab_label = openTabs[i]->filename;
                    
                    // Add modified indicator
                    if (openTabs[i]->is_modified) {
                        tab_label += " *";
                    }
                    
                    // Add close button to tab label
                    tab_label += "  ";
                    
                    if (ImGui::BeginTabItem(tab_label.c_str(), &is_open, ImGuiTabItemFlags_None)) {
                        active_tab_index = i;
                        ImGui::EndTabItem();
                    }
                    
                    // Handle tab close
                    if (!is_open) {
                        openTabs.erase(openTabs.begin() + i);
                        if (active_tab_index >= (int)openTabs.size()) {
                            active_tab_index = openTabs.size() - 1;
                        }
                        if (openTabs.empty()) {
                            active_tab_index = -1;
                            showWelcomeScreen = true;
                        }
                        i--; // Adjust index after erasing
                    }
                }
                ImGui::EndTabBar();
            }
            
            ImGui::Separator();
            
            // Render the active tab's editor
            if (active_tab_index >= 0 && active_tab_index < (int)openTabs.size()) {
                openTabs[active_tab_index]->editor.Render("TextEditor");
            }
        }
        
        ImGui::End();
        if (!showWelcomeScreen){
            // Editor window (takes remaining space)
            ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separatorPos, viewport->WorkPos.y));
            ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separatorPos, viewport->WorkSize.y - terminalHeight));

            // --- D. Draw the locked Code Editor window ---
            ImGui::Begin("Code Editor Background", nullptr, window_flags);
            
            editor.Render("TextEditor"); 
            
            ImGui::End();
        }

        // 6. Render the frame to the screen
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}