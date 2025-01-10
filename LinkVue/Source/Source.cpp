#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <vector>
#include <array>
#include <iostream>

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
    std::array<float, 3> currentColor = { 0.0f, 0.0f, 0.0f };    // Drawing color
    std::array<float, 3> canvasColor = { 1.0f, 1.0f, 1.0f };     // Canvas background color
    float currentThickness = 2.0f;
    bool isDrawing = false;
    bool showCanvas = true;
    bool isDragging = false;
    ImVec2 offset = ImVec2(0.0f, 0.0f);      // Canvas offset for panning
    ImVec2 lastMousePos = ImVec2(0.0f, 0.0f); // Last mouse position for panning
    float zoom = 1.0f;                        // Zoom level

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
        ImGui::StyleColorsDark();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

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
                        // Handle zooming with mouse wheel
                        if (ImGui::GetIO().MouseWheel != 0.0f) {
                            zoom *= (1.0f + ImGui::GetIO().MouseWheel * 0.1f);
                            if (zoom < 0.1f) zoom = 0.1f;
                            if (zoom > 5.0f) zoom = 5.0f;
                        }

                        ImVec2 mousePos = ImGui::GetMousePos();

                        // Middle mouse button for panning
                        if (ImGui::IsMouseDown(2)) { // Middle mouse button
                            if (!isDragging) {
                                isDragging = true;
                                lastMousePos = mousePos;
                            }
                            offset.x += mousePos.x - lastMousePos.x;
                            offset.y += mousePos.y - lastMousePos.y;
                            lastMousePos = mousePos;
                        }
                        else {
                            isDragging = false;
                        }

                        // Left mouse button for drawing
                        if (ImGui::IsMouseDown(0) && !isDragging) {
                            ImVec2 canvasPos = screenToCanvas(mousePos, windowPos);

                            if (!isDrawing) {
                                strokes.push_back(Stroke());
                                isDrawing = true;
                            }

                            Point newPoint = {
                                canvasPos.x,
                                canvasPos.y,
                                currentColor,
                                currentThickness / zoom  // Adjust thickness based on zoom
                            };
                            strokes.back().points.push_back(newPoint);
                        }
                        else {
                            isDrawing = false;
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

                    // Draw grid (optional visual reference)
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

            // Tools window
            ImGui::Begin("Tools");

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
                strokes.clear();
            }

            if (ImGui::Button(showCanvas ? "Hide Canvas" : "Show Canvas")) {
                showCanvas = !showCanvas;
            }

            ImGui::Text("\nControls:");
            ImGui::Text("- Left Click: Draw");
            ImGui::Text("- Middle Click: Pan");
            ImGui::Text("- Mouse Wheel: Zoom");

            ImGui::End();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
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