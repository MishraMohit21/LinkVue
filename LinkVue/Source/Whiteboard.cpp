



//[Implmentation]

#include "Whiteboard.h"


namespace YAML {
    template<>
    struct convert<Point> {
        static Node encode(const Point& point) {
            Node node;
            node["x"] = point.x;
            node["y"] = point.y;
            node["color"] = point.color;
            node["thickness"] = point.thickness;
            return node;
        }

        static bool decode(const Node& node, Point& point) {
            if (!node.IsMap() || !node["x"] || !node["y"] || !node["color"] || !node["thickness"])
                return false;

            point.x = node["x"].as<float>();
            point.y = node["y"].as<float>();
            point.color = node["color"].as<std::array<float, 3>>();
            point.thickness = node["thickness"].as<float>();
            return true;
        }
    };

    template<>
    struct convert<Stroke> {
        static Node encode(const Stroke& stroke) {
            Node node;
            for (const auto& point : stroke.points) {
                node.push_back(point);
            }
            return node;
        }

        static bool decode(const Node& node, Stroke& stroke) {
            if (!node.IsSequence())
                return false;

            for (const auto& item : node) {
                stroke.points.push_back(item.as<Point>());
            }
            return true;
        }
    };
}



void Whiteboard::saveState()
{
    m_UndoStack.push(m_Strokes);
    // Clear redo stack when new action is performed
    while (!m_RedoStack.empty()) {
        m_RedoStack.pop();
    }
}

ImVec2 Whiteboard::screenToCanvas(const ImVec2& screenPos, const ImVec2& windowPos)
{
    return ImVec2(
        (screenPos.x - windowPos.x - m_Offset.x) / m_Zoom,
        (screenPos.y - windowPos.y - m_Offset.y) / m_Zoom
    );
}

ImVec2 Whiteboard::canvasToScreen(const ImVec2& canvasPos, const ImVec2& windowPos)
{
    return ImVec2(
        canvasPos.x * m_Zoom + windowPos.x + m_Offset.x,
        canvasPos.y * m_Zoom + windowPos.y + m_Offset.y
    );
}

void Whiteboard::Undo()
{
    if (!m_UndoStack.empty()) {
        m_RedoStack.push(m_Strokes);
        m_Strokes = m_UndoStack.top();
        m_UndoStack.pop();
    }
}

void Whiteboard::redo()
{
    if (!m_RedoStack.empty()) {
        m_UndoStack.push(m_Strokes);
        m_Strokes = m_RedoStack.top();
        m_RedoStack.pop();
    }
}

void Whiteboard::init()
{

}

void Whiteboard::renderCanvas()
{
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
                ImColor(m_CanvasColor[0], m_CanvasColor[1], m_CanvasColor[2])
            );

            // Handle input
            if (ImGui::IsWindowHovered()) {
                if (ImGui::GetIO().MouseWheel != 0.0f) {
                    m_Zoom *= (1.0f + ImGui::GetIO().MouseWheel * 0.1f);
                    if (m_Zoom < 0.1f) m_Zoom = 0.1f;
                    if (m_Zoom > 5.0f) m_Zoom = 5.0f;
                }

                ImVec2 mousePos = ImGui::GetMousePos();

                if (ImGui::IsMouseDown(2)) {
                    if (!isDragging) {
                        isDragging = true;
                        m_LastMousePos = mousePos;
                    }
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                    m_Offset.x += mousePos.x - m_LastMousePos.x;
                    m_Offset.y += mousePos.y - m_LastMousePos.y;
                    m_LastMousePos = mousePos;
                }
                else {
                    isDragging = false;
                }

                if (ImGui::IsMouseDown(0) && !isDragging) {
                    ImVec2 canvasPos = screenToCanvas(mousePos, windowPos);

                    if (!isDrawing) {
                        saveState(); // Save state before starting new stroke
                        m_Strokes.push_back(Stroke());
                        isDrawing = true;
                    }

                    Point newPoint = {
                        canvasPos.x,
                        canvasPos.y,
                        m_CurrentColor,
                        m_CurrentThickness / m_Zoom
                    };
                    m_Strokes.back().points.push_back(newPoint);
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

            // Draw all m_Strokes
            for (const auto& stroke : m_Strokes) {
                for (size_t i = 1; i < stroke.points.size(); i++) {
                    const auto& p1 = stroke.points[i - 1];
                    const auto& p2 = stroke.points[i];

                    ImVec2 screenP1 = canvasToScreen(ImVec2(p1.x, p1.y), windowPos);
                    ImVec2 screenP2 = canvasToScreen(ImVec2(p2.x, p2.y), windowPos);

                    drawList->AddLine(
                        screenP1,
                        screenP2,
                        ImColor(p1.color[0], p1.color[1], p1.color[2]),
                        p1.thickness * m_Zoom
                    );
                }
            }

            // Draw grid
            const float gridSize = 50.0f * m_Zoom;
            const ImU32 gridColor = ImColor(0.8f, 0.8f, 0.8f, 0.2f);
            for (float x = fmod(m_Offset.x, gridSize); x < windowSize.x; x += gridSize) {
                drawList->AddLine(
                    ImVec2(windowPos.x + x, windowPos.y),
                    ImVec2(windowPos.x + x, windowPos.y + windowSize.y),
                    gridColor
                );
            }
            for (float y = fmod(m_Offset.y, gridSize); y < windowSize.y; y += gridSize) {
                drawList->AddLine(
                    ImVec2(windowPos.x, windowPos.y + y),
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + y),
                    gridColor
                );
            }
        }
        ImGui::End();
    }

}

void Whiteboard::drawToolWindow()
{
    ImGui::SetNextWindowBgAlpha(0.35);
    // Tools window
    ImGui::Begin("Tools");

    // Add Undo/redo buttons at the top
    if (ImGui::Button("Undo") && !m_UndoStack.empty()) {
        Undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo") && !m_RedoStack.empty()) {
        redo();
    }

    ImGui::Separator();

    ImGui::Text("Drawing Color");
    ImGui::ColorEdit3("##DrawingColor", m_CurrentColor.data());

    ImGui::Text("Canvas Color");
    ImGui::ColorEdit3("##CanvasColor", 
        m_CanvasColor.data());

    ImGui::Text("Brush Size");
    ImGui::SliderFloat("##Thickness", &m_CurrentThickness, 1.0f, 20.0f);

    ImGui::Text("Zoom: %.1fx", m_Zoom);
    if (ImGui::Button("Reset Zoom")) {
        m_Zoom = 1.0f;
    }

    if (ImGui::Button("Reset Pan")) {
        m_Offset = ImVec2(0.0f, 0.0f);
    }

    if (ImGui::Button("Clear Canvas")) {
        saveState(); // Save state before clearing
        m_Strokes.clear();
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
}

void Whiteboard::handleNetworkMessage(const std::string& message)
{
    deserialize(message);
}

std::string Whiteboard::getUpdateData()
{
    return serialize();
}

std::string Whiteboard::serialize() const 
{
    YAML::Node node;

    // Serialize strokes
    for (const auto& stroke : m_Strokes) {
        node["strokes"].push_back(stroke);
    }

    // Serialize undo stack
    auto temp = m_UndoStack;
    while (!temp.empty())
    {
        const auto stackItem = temp.top();
        YAML::Node stackNode;
        for (const auto& stroke : stackItem) {
            stackNode.push_back(stroke);
        }
        node["undoStack"].push_back(stackNode);
        temp.pop();
    }

    // Serialize redo stack
    temp = m_RedoStack;
    while (!temp.empty())
    {
        const auto stackItem = temp.top();
        YAML::Node stackNode;
        for (const auto& stroke : stackItem) {
            stackNode.push_back(stroke);
        }
        node["redoStack"].push_back(stackNode);
        temp.pop();
    }

    // Serialize other properties
    node["currentColor"] = m_CurrentColor;
    node["canvasColor"] = m_CanvasColor;
    node["currentThickness"] = m_CurrentThickness;
    node["zoom"] = m_Zoom;

    return YAML::Dump(node);
}

void Whiteboard::deserialize(const std::string& nodeString)
{
    YAML::Node node = YAML::Load(nodeString);
    // Deserialize strokes
    if (node["strokes"]) {
        m_Strokes.clear();
        for (const auto& strokeNode : node["strokes"]) {
            m_Strokes.push_back(strokeNode.as<Stroke>());
        }
    }

    // Deserialize undo stack
    if (node["undoStack"]) {
        while (!m_UndoStack.empty()) m_UndoStack.pop(); // Clear current stack
        for (const auto& stackNode : node["undoStack"]) {
            std::vector<Stroke> stackItem;
            for (const auto& strokeNode : stackNode) {
                stackItem.push_back(strokeNode.as<Stroke>());
            }
            m_UndoStack.push(stackItem);
        }
    }

    // Deserialize redo stack
    if (node["redoStack"]) {
        while (!m_RedoStack.empty()) m_RedoStack.pop(); // Clear current stack
        for (const auto& stackNode : node["redoStack"]) {
            std::vector<Stroke> stackItem;
            for (const auto& strokeNode : stackNode) {
                stackItem.push_back(strokeNode.as<Stroke>());
            }
            m_RedoStack.push(stackItem);
        }
    }

    // Deserialize other properties
    if (node["currentColor"]) m_CurrentColor = node["currentColor"].as<std::array<float, 3>>();
    if (node["canvasColor"]) m_CanvasColor = node["canvasColor"].as<std::array<float, 3>>();
    if (node["currentThickness"]) m_CurrentThickness = node["currentThickness"].as<float>();
    if (node["zoom"]) m_Zoom = node["zoom"].as<float>();
}

