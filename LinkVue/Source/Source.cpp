#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <vector>
#include <array>
#include <iostream>
#include <stack>


void SetDarkThemeColors()
{
    auto& colors = ImGui::GetStyle().Colors;

    // --- Dark Mode ---

    // Base: Deep space black with a hint of blue for depth
    colors[ImGuiCol_WindowBg] = ImVec4{ 0.03f, 0.03f, 0.04f, 1.0f };

    // Headers: Subtle blue undertone for a futuristic feel
    colors[ImGuiCol_Header] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f };
    colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.12f, 0.12f, 0.15f, 1.0f };
    colors[ImGuiCol_HeaderActive] = ImVec4{ 0.05f, 0.05f, 0.07f, 1.0f };

    // Buttons: Introducing a vibrant blue accent
    colors[ImGuiCol_Button] = ImVec4{ 0.1f, 0.1f, 0.12f, 1.0f };
    colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.0f, 0.2f, 0.4f, 1.0f }; // Electric blue
    colors[ImGuiCol_ButtonActive] = ImVec4{ 0.0f, 0.15f, 0.3f, 1.0f }; // Deeper blue

    // Frame BG: Blends with the background
    colors[ImGuiCol_FrameBg] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f };
    colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.15f, 0.15f, 0.2f, 1.0f };
    colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.05f, 0.05f, 0.07f, 1.0f };

    // Tabs:  Using a contrasting purple accent
    colors[ImGuiCol_Tab] = ImVec4{ 0.07f, 0.07f, 0.09f, 1.0f };
    colors[ImGuiCol_TabHovered] = ImVec4{ 0.3f, 0.1f, 0.4f, 1.0f }; // Vivid purple
    colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.05f, 0.3f, 1.0f }; // Deeper purple
    colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.04f, 0.04f, 0.05f, 1.0f };
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f };

    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

    // Title: Subtle and unobtrusive
    colors[ImGuiCol_TitleBg] = ImVec4{ 0.07f, 0.07f, 0.09f, 1.0f };
    colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.07f, 0.07f, 0.09f, 1.0f };
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.04f, 0.04f, 0.05f, 1.0f };

    // --- Accent Colors ---
    colors[ImGuiCol_Text] = ImVec4{ 0.8f, 0.8f, 0.82f, 1.0f }; // Slightly off-white for readability
    colors[ImGuiCol_CheckMark] = ImVec4{ 0.0f, 0.6f, 1.0f, 1.0f }; // Bright cyan for checkmarks
    colors[ImGuiCol_SliderGrab] = ImVec4{ 0.0f, 0.4f, 0.8f, 1.0f }; // Blue slider grab
    colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.0f, 0.3f, 0.7f, 1.0f }; // Slightly darker when active

    // Assuming your menu bar uses these ImGui elements:
    colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.13f, 0.13f, 0.14f, 1.0f }; // Match the main window background
    //colors[/*ImGuiCol_MenuBarItem*/] = ImVec4{ 0.8f, 0.8f, 0.82f, 1.0f }; // Slightly off-white for menu items

    // If you have a border around your menu bar:
    colors[ImGuiCol_Border] = ImVec4{ 0.1f, 0.1f, 0.12f, 1.0f }; // Subtle blue tint



   


}

struct Point {
    float x, y;
    std::array<float, 3> color;
    float thickness;
};

struct Stroke {
    std::vector<Point> points;
};

class Whiteboard {
private:
    std::vector<Stroke> strokes;
    std::stack<std::vector<Stroke>> UndoStack;
    std::stack<std::vector<Stroke>> redoStack;
    std::array<float, 3> currentColor = { 0.0f, 0.0f, 0.0f };    // Drawing color
    std::array<float, 3> canvasColor = { 1.0f, 1.0f, 1.0f };     // Canvas background color
    float currentThickness = 2.0f;
    bool isDrawing = false;
    bool showCanvas = true;
    bool isDragging = false;
    ImVec2 offset = ImVec2(0.0f, 0.0f);      // Canvas offset for panning
    ImVec2 lastMousePos = ImVec2(0.0f, 0.0f); // Last mouse position for panning
    float zoom = 1.0f;                        // Zoom level

    // Save current state before making changes
    void saveState() {
        UndoStack.push(strokes);
        // Clear redo stack when new action is performed
        while (!redoStack.empty()) {
            redoStack.pop();
        }
    }

    ImVec2 screenToCanvas(const ImVec2& screenPos, const ImVec2& windowPos) {
        return ImVec2(
            (screenPos.x - windowPos.x - offset.x) / zoom,
            (screenPos.y - windowPos.y - offset.y) / zoom
        );
    }

    ImVec2 canvasToScreen(const ImVec2& canvasPos, const ImVec2& windowPos) {
        return ImVec2(
            canvasPos.x * zoom + windowPos.x + offset.x,
            canvasPos.y * zoom + windowPos.y + offset.y
        );
    }

public:
    void Undo() {
        if (!UndoStack.empty()) {
            redoStack.push(strokes);
            strokes = UndoStack.top();
            UndoStack.pop();
        }
    }

    void redo() {
        if (!redoStack.empty()) {
            UndoStack.push(strokes);
            strokes = redoStack.top();
            redoStack.pop();
        }
    }

    void init() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(1280, 720, "Whiteboard", NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigDockingWithShift = false;

       

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");


        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.AntiAliasedFill = true;
            style.AntiAliasedLines = true;
            style.AntiAliasedLinesUseTex = true;   style.WindowPadding = ImVec2(12, 12);
            style.FramePadding = ImVec2(6, 6);
            //style.P = ImVec2(12, 6);
            style.ItemSpacing = ImVec2(6, 6);
            style.ItemInnerSpacing = ImVec2(6, 6);
            style.TouchExtraPadding = ImVec2(0, 0);
            style.IndentSpacing = 25;
            style.ScrollbarSize = 12;
            style.GrabMinSize = 10;

            // Borders
            style.WindowBorderSize = 1;
            style.ChildBorderSize = 1;
            style.PopupBorderSize = 1;
            style.FrameBorderSize = 1;
            style.TabBorderSize = 1;

            // Rounding
            style.WindowRounding =    25;
            style.ChildRounding =     25;
            style.FrameRounding =     25;
            style.PopupRounding =     25;
            style.ScrollbarRounding = 25;
            style.GrabRounding =      25;
            style.TabRounding =       25;
            //style.WindowMinSize.x = 10.0f;
            // Alignment
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Right;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            // Anti-aliasing
            style.AntiAliasedLines = true;
            style.AntiAliasedFill = true;
            //style.Colors[ImGuiCol_WindowBg].w = 0.1f;
        }

        SetDarkThemeColors();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            }
            // Canvas window
            if (showCanvas) {
                if (ImGui::Begin("Canvas", &showCanvas)) {
                    ImVec2 windowPos = ImGui::GetWindowPos();
                    ImVec2 windowSize = ImGui::GetWindowSize();
                    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
                    // Draw background
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(
                        windowPos,
                        ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                        ImColor(canvasColor[0], canvasColor[1], canvasColor[2])
                    );

                    // Handle input
                    if (ImGui::IsWindowHovered()) {
                        if (ImGui::GetIO().MouseWheel != 0.0f) {
                            zoom *= (1.0f + ImGui::GetIO().MouseWheel * 0.1f);
                            if (zoom < 0.1f) zoom = 0.1f;
                            if (zoom > 5.0f) zoom = 5.0f;
                        }

                        ImVec2 mousePos = ImGui::GetMousePos();

                        if (ImGui::IsMouseDown(2)) {
                            if (!isDragging) {
                                isDragging = true;
                                lastMousePos = mousePos;
                            }
                            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                            offset.x += mousePos.x - lastMousePos.x;
                            offset.y += mousePos.y - lastMousePos.y;
                            lastMousePos = mousePos;
                        }
                        else {
                            isDragging = false;
                        }

                        if (ImGui::IsMouseDown(0) && !isDragging) {
                            ImVec2 canvasPos = screenToCanvas(mousePos, windowPos);

                            if (!isDrawing) {
                                saveState(); // Save state before starting new stroke
                                strokes.push_back(Stroke());
                                isDrawing = true;
                            }

                            Point newPoint = {
                                canvasPos.x,
                                canvasPos.y,
                                currentColor,
                                currentThickness / zoom
                            };
                            strokes.back().points.push_back(newPoint);
                        }
                        else {
                            isDrawing = false;
                        }


                        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
                        {
                            if (ImGui::GetIO().KeyCtrl)
                            {
                                std::cout << "Redo Done pressed\n";
                                redo();
                            }
                            else
                            {
                                std::cout << "Undo Done pressed\n";
                                Undo();
                            }
                        }


                    }

                    // Draw all strokes
                    for (const auto& stroke : strokes) {
                        for (size_t i = 1; i < stroke.points.size(); i++) {
                            const auto& p1 = stroke.points[i - 1];
                            const auto& p2 = stroke.points[i];

                            ImVec2 screenP1 = canvasToScreen(ImVec2(p1.x, p1.y), windowPos);
                            ImVec2 screenP2 = canvasToScreen(ImVec2(p2.x, p2.y), windowPos);

                            drawList->AddLine(
                                screenP1,
                                screenP2,
                                ImColor(p1.color[0], p1.color[1], p1.color[2]),
                                p1.thickness * zoom
                            );
                        }
                    }

                    // Draw grid
                    const float gridSize = 50.0f * zoom;
                    const ImU32 gridColor = ImColor(0.8f, 0.8f, 0.8f, 0.2f);
                    for (float x = fmod(offset.x, gridSize); x < windowSize.x; x += gridSize) {
                        drawList->AddLine(
                            ImVec2(windowPos.x + x, windowPos.y),
                            ImVec2(windowPos.x + x, windowPos.y + windowSize.y),
                            gridColor
                        );
                    }
                    for (float y = fmod(offset.y, gridSize); y < windowSize.y; y += gridSize) {
                        drawList->AddLine(
                            ImVec2(windowPos.x, windowPos.y + y),
                            ImVec2(windowPos.x + windowSize.x, windowPos.y + y),
                            gridColor
                        );
                    }
                }
                ImGui::End();
            }

            ImGui::SetNextWindowBgAlpha(0.35);
            // Tools window
            ImGui::Begin("Tools");

            // Add Undo/redo buttons at the top
            if (ImGui::Button("Undo") && !UndoStack.empty()) {
                Undo();
            }
            ImGui::SameLine();
            if (ImGui::Button("Redo") && !redoStack.empty()) {
                redo();
            }

            ImGui::Separator();

            ImGui::Text("Drawing Color");
            ImGui::ColorEdit3("##DrawingColor", currentColor.data());

            ImGui::Text("Canvas Color");
            ImGui::ColorEdit3("##CanvasColor", canvasColor.data());

            ImGui::Text("Brush Size");
            ImGui::SliderFloat("##Thickness", &currentThickness, 1.0f, 20.0f);

            ImGui::Text("Zoom: %.1fx", zoom);
            if (ImGui::Button("Reset Zoom")) {
                zoom = 1.0f;
            }

            if (ImGui::Button("Reset Pan")) {
                offset = ImVec2(0.0f, 0.0f);
            }

            if (ImGui::Button("Clear Canvas")) {
                saveState(); // Save state before clearing
                strokes.clear();
            }

            if (ImGui::Button(showCanvas ? "Hide Canvas" : "Show Canvas")) {
                showCanvas = !showCanvas;
            }

            ImGui::Text("\nControls:");
            ImGui::Text("- Left Click: Draw");
            ImGui::Text("- Middle Click: Pan");
            ImGui::Text("- Mouse Wheel: Zoom");
            ImGui::Text("- Backspace: Undo");
            ImGui::Text("- Ctrl+Backspace: Redo");

            ImGui::End();

            
            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    Whiteboard whiteboard;
    whiteboard.init();
    return 0;
}