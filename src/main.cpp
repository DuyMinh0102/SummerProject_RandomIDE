#include <iostream>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "TextEditor.h"
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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
        return TextEditor::LanguageDefinition::CPlusPlus(); // Default
    }
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

    // 4. The Application / Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Catch keypresses and mouse movement

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        // Ctrl
        if (io.KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_Equal)) { 
                if (io.FontGlobalScale < 10.0f) io.FontGlobalScale += 0.1f;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Minus)) {
                if (io.FontGlobalScale > 0.5f) io.FontGlobalScale -= 0.1f;
            }
        }

        // 5. Build the UI

        // Main Menu Bar
        if (ImGui::BeginMainMenuBar()){
            if (ImGui::BeginMenu("File")){
                if (ImGui::MenuItem("Open", "Ctrl + O")) {}
                if (ImGui::MenuItem("Save", "Ctrl + S")) {}
                ImGui::Separator();
                if(ImGui::MenuItem("Exit", "Alt+F4")) {glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")){
                if (ImGui::MenuItem("Zoom In", "Ctrl + =")) { 
                    if (io.FontGlobalScale <= 10.0f) io.FontGlobalScale += 0.1f; 
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl + -")) { 
                    if (io.FontGlobalScale >= 0.5f) io.FontGlobalScale -= 0.1f;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Sidebar Explorer
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // Set up sidebar window
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(250, viewport->WorkSize.y));
        ImGuiWindowFlags sidebar_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        
        ImGui::Begin("File Explorer", nullptr, sidebar_flags);
        
        if (ImGui::Button("Up")) {
            if (current_path.has_parent_path()) {
                current_path = current_path.parent_path();
            }
        }
        ImGui::SameLine();
        ImGui::Text(current_path.string().c_str());
        
        ImGui::Separator();
        
        // List directories and files
        try {
            for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
                std::string filename = entry.path().filename().string();
                bool is_directory = entry.is_directory();
                
                if (is_directory) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                    if (ImGui::Selectable(("📁 " + filename).c_str())) {
                        current_path = entry.path();
                    }
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                    if (ImGui::Selectable(("📄 " + filename).c_str())) {
                        std::string content = ReadFileContent(entry.path());
                        if (!content.empty()) {
                            editor.SetText(content);
                            std::string ext = entry.path().extension().string();
                            editor.SetLanguageDefinition(GetLanguageFromExtension(ext));
                        }
                    }
                    ImGui::PopStyleColor();
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            ImGui::Text("Error reading directory");
        }
        
        ImGui::End();

        // Editor window (takes remaining space)
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + 250, viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - 250, viewport->WorkSize.y));

        // --- C. Set strict flags to lock the window ---
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;      // Hide the ImGui title
        window_flags |= ImGuiWindowFlags_NoCollapse;      // Prevent minimizing
        window_flags |= ImGuiWindowFlags_NoResize;        // Prevent user resizing
        window_flags |= ImGuiWindowFlags_NoMove;          // Prevent dragging
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus; 
        window_flags |= ImGuiWindowFlags_NoNavFocus;      

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