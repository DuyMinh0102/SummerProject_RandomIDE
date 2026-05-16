#include <iostream>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "TextEditor.h"

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

    // 4. The Application / Render Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Catch keypresses and mouse movement

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 5. Build the UI

        // Main Menu Bar
        if (ImGui::BeginMainMenuBar()){
            if (ImGui::BeginMenu("File")){
                if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                ImGui::Separator();
                if(ImGui::MenuItem("Exit", "Alt+F4")) {glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

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