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

    std::string fullCmd = cmd + "2>&1";

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

    // File explorer state
    std::filesystem::path current_path = std::filesystem::current_path();
    std::vector<std::filesystem::path> selected_files;

    // Separator state (initial sidebar width)
    float separator_pos = 250.0f;
    bool is_dragging_separator = false;

    // Terminal state
    std::vector<std::string> terminal_log;
    char terminal_input[256] = "";
    terminal_log.push_back("IDE Terminal initialized...");
    terminal_log.push_back("Type 'help' for a list of commands.");
    // New file inline creation state
    bool is_creating_new_file = false;
    char new_file_name_buffer[256] = "";

    // New folder inline creation state
    bool is_creating_new_folder = false;
    char new_folder_name_buffer[256] = "";

    // Context menu state
    bool show_empty_space_context_menu = false;
    bool show_item_context_menu = false;
    std::filesystem::path selected_item_path;
    bool selected_item_is_directory = false;

    // Welcome screen state
    bool show_welcome_screen = true;

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
                        std::string content = ReadFileContent(filepath);
                        editor.SetText(content);

                        std::filesystem::path path_obj(filepath);
                        editor.SetLanguageDefinition(GetLanguageFromExtension(path_obj.extension().string()));
                        current_path = path_obj.parent_path();
                    }
                }
                if (ImGui::MenuItem("Save", "Ctrl + S")) {
                    auto destination = pfd::save_file("Save File As", ".", { "C++ Files", "*.cpp *.h", "All Files", "*" }).result();
                        
                    if (!destination.empty()) {
                        std::string current_text = editor.GetText();
                        
                        std::ofstream out_file(destination);
                        if (out_file.is_open()) {
                            out_file << current_text;
                            out_file.close();
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
                        std::string content = ReadFileContent(filepath);
                        editor.SetText(content);

                        std::filesystem::path path_obj(filepath);
                        editor.SetLanguageDefinition(GetLanguageFromExtension(path_obj.extension().string()));
                        current_path = path_obj.parent_path();
                    }
                }
                if(ImGui::IsKeyPressed(ImGuiKey_S)){
                    auto destination = pfd::save_file("Save File As", ".", { "C++ Files", "*.cpp *.h", "All Files", "*" }).result();
                        
                    if (!destination.empty()) {
                        std::string current_text = editor.GetText();
                        
                        std::ofstream out_file(destination);
                        if (out_file.is_open()) {
                            out_file << current_text;
                            out_file.close();
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
        if (show_welcome_screen) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
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
                is_creating_new_file = true;
                new_file_name_buffer[0] = '\0';
                show_welcome_screen = false;
            }
            
            // Open File Button
            ImGui::SetCursorPosX(center_pos.x - button_width * 0.5f);
            ImGui::SetCursorPosY(center_pos.y + 10);
            if (ImGui::Button("Open File", ImVec2(button_width, 40))) {
                auto selection = pfd::open_file("Open File", ".", {"C++ Files", "*.cpp *.h *.hpp", "All Files", "*"}).result();
                if (!selection.empty()) {
                    std::string filepath = selection[0];
                    std::string content = ReadFileContent(filepath);
                    editor.SetText(content);
                    std::filesystem::path path_obj(filepath);
                    editor.SetLanguageDefinition(GetLanguageFromExtension(path_obj.extension().string()));
                    current_path = path_obj.parent_path();
                }
                show_welcome_screen = false;
            }
            
            // Open Folder Button
            ImGui::SetCursorPosX(center_pos.x - button_width * 0.5f);
            ImGui::SetCursorPosY(center_pos.y + 70);
            if (ImGui::Button("Open Folder", ImVec2(button_width, 40))) {
                auto folder = pfd::select_folder("Open Folder", ".").result();
                if (!folder.empty()) {
                    current_path = folder;
                }
                show_welcome_screen = false;
            }
            
            ImGui::PopStyleColor(3); // Pop button colors
            ImGui::PopStyleColor(); // Pop window bg
            
            ImGui::End();
        }

        // Sidebar Explorer
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // Set up sidebar window
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(separator_pos, viewport->WorkSize.y));
        ImGuiWindowFlags sidebar_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        
        ImGui::Begin("File Explorer", nullptr, sidebar_flags);
        
        if (ImGui::Button("Up")) {
            if (current_path.has_parent_path()) {
                current_path = current_path.parent_path();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("New File")) {
            is_creating_new_file = true;
            new_file_name_buffer[0] = '\0';
        }
        ImGui::SameLine();
        ImGui::Text(current_path.string().c_str());
        
        ImGui::Separator();

        // Context menu for empty space
        if (ImGui::BeginPopupContextWindow("##empty_space_context", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("New File")) {
                is_creating_new_file = true;
                new_file_name_buffer[0] = '\0';
            }
            if (ImGui::MenuItem("New Folder")) {
                is_creating_new_folder = true;
                new_folder_name_buffer[0] = '\0';
            }
            ImGui::EndPopup();
        }
        
        // List directories and files
        try {
            // Show inline new file input if creating new file
            if (is_creating_new_file) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                ImGui::Text("[FILE] ");
                ImGui::SameLine();
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##newfilename", new_file_name_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (strlen(new_file_name_buffer) > 0) {
                        std::filesystem::path new_file_path = current_path / new_file_name_buffer;
                        
                        // Create the file
                        std::ofstream new_file(new_file_path);
                        if (new_file.is_open()) {
                            new_file.close();
                            
                            // Open the new file in the editor
                            std::string content = ReadFileContent(new_file_path);
                            editor.SetText(content);
                            std::string ext = new_file_path.extension().string();
                            editor.SetLanguageDefinition(GetLanguageFromExtension(ext));
                        }
                    }
                    is_creating_new_file = false;
                }
                ImGui::PopStyleColor();
                
                // Cancel on Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    is_creating_new_file = false;
                }
                
                // Cancel on click outside the input box
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemActive()) {
                    is_creating_new_file = false;
                }
            }

            // Show inline new folder input if creating new folder
            if (is_creating_new_folder) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::Text("[DIR] ");
                ImGui::SameLine();
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##newfoldername", new_folder_name_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    if (strlen(new_folder_name_buffer) > 0) {
                        std::filesystem::path new_folder_path = current_path / new_folder_name_buffer;
                        
                        // Create the folder
                        std::filesystem::create_directory(new_folder_path);
                    }
                    is_creating_new_folder = false;
                }
                ImGui::PopStyleColor();
                
                // Cancel on Escape
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    is_creating_new_folder = false;
                }
                
                // Cancel on click outside the input box
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemActive()) {
                    is_creating_new_folder = false;
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
                        std::string content = ReadFileContent(entry.path());
                        if (!content.empty()) {
                            editor.SetText(content);
                            std::string ext = entry.path().extension().string();
                            editor.SetLanguageDefinition(GetLanguageFromExtension(ext));
                        }
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
        float separator_x = viewport->WorkPos.x + separator_pos;
        bool is_near_separator = (io.MousePos.x >= separator_x - 4 && io.MousePos.x <= separator_x + 4);
        
        if (is_near_separator || is_dragging_separator) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                is_dragging_separator = true;
                separator_pos = io.MousePos.x - viewport->WorkPos.x;
                // Clamp separator position
                if (separator_pos < 100.0f) separator_pos = 100.0f;
                if (separator_pos > viewport->WorkSize.x - 100.0f) separator_pos = viewport->WorkSize.x - 100.0f;
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            is_dragging_separator = false;
        }

        // Terminal
        float terminalHeight = 200.0f;

        float terminalYPos = viewport->WorkPos.y + viewport->WorkSize.y - terminalHeight;

        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separator_pos, terminalYPos));

        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separator_pos, terminalHeight));

               // --- C. Set strict flags to lock the window ---
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;      // Hide the ImGui title
        window_flags |= ImGuiWindowFlags_NoCollapse;      // Prevent minimizing
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus; 
        window_flags |= ImGuiWindowFlags_NoNavFocus;     

        // Draw terminal
        ImGui::Begin("Terminal", nullptr, window_flags);
        
        const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar)){
            for (const std::string& line : terminal_log){
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

        if(ImGui::InputText("##terminal_input", terminal_input, IM_ARRAYSIZE(terminal_input), inputFlags)){
            std::string command = terminal_input;

            if (!command.empty()){
                terminal_log.push_back("> " + command);

                if (command == "clear"){
                    terminal_log.clear();
                }
                else if(command.rfind("cd ", 0) == 0){
                    std::string newDir = command.substr(3);

                    try{
                        std::filesystem::current_path(newDir);

                        current_path= std::filesystem::current_path();
                        terminal_log.push_back(current_path.string());  
                    }
                    catch (const std::exception& e){
                        terminal_log.push_back(std::string("cd error: ") + e.what());
                    }
                }
                else{
                    std::string output = ExecSystemCommand(command);

                    std::istringstream stream(output);
                    std::string line;
                    while(std::getline(stream, line)){
                        if (!line.empty() && line.back() == '\n') line.pop_back();
                        terminal_log.push_back(line); 
                    }
                }
            }

            strcpy(terminal_input, "");
            reclaimFocus = true;
        }
        ImGui::PopItemWidth();

        ImGui::SetItemDefaultFocus();
        if (reclaimFocus){
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::End();

        // Editor window (takes remaining space)
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + separator_pos, viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - separator_pos, viewport->WorkSize.y - terminalHeight));

        // --- D. Draw the locked Code Editor window ---
        ImGui::Begin("Code Editor Background", nullptr, window_flags);
        
        // Render the actual text editing widget inside this locked space
        editor.Render("TextEditor"); 
        
        ImGui::End();

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