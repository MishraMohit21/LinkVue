#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ImGui.h>


#include <vector>
#include <stack>
#include <array>
#include <iostream>

#include <yaml-cpp/yaml.h>

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
    std::vector<Stroke> m_Strokes;
    std::stack<std::vector<Stroke>> m_UndoStack;
    std::stack<std::vector<Stroke>> m_RedoStack;
    std::array<float, 3> m_CurrentColor = { 0.0f, 0.0f, 0.0f }; // Drawing color
    std::array<float, 3> m_CanvasColor = { 1.0f, 1.0f, 1.0f };  // Canvas background color
    float m_CurrentThickness = 2.0f;
    bool isDrawing = false;
    bool showCanvas = true;
    bool isDragging = false;
    ImVec2 m_Offset = ImVec2(0.0f, 0.0f); // Canvas offset for panning
    ImVec2 m_LastMousePos = ImVec2(0.0f, 0.0f); // Last mouse position for panning
    float m_Zoom = 1.0f; // Zoom level

    // Private helper functions
    void saveState();
    ImVec2 screenToCanvas(const ImVec2& screenPos, const ImVec2& windowPos);
    ImVec2 canvasToScreen(const ImVec2& canvasPos, const ImVec2& windowPos);
    std::string serialize() const;
    void deserialize(const std::string& node);

public:

    Whiteboard() = default;
    ~Whiteboard() = default;


    // Public member functions
    void Undo();
    void redo();
    void init();

    void renderCanvas();
    void drawToolWindow();

    void handleNetworkMessage(const std::string& message);

    // Get data that needs to be sent over network
    std::string getUpdateData(); 

    //Getters and Setters for the Networking class
    std::vector<Stroke> getStrokes() const { return m_Strokes; }
    std::stack<std::vector<Stroke>> getUndoStack() const { return m_UndoStack; }
    std::stack<std::vector<Stroke>> getRedoStack() const { return m_RedoStack; }

    // Getter and Setter declarations

    float getZoom() const { return m_Zoom; }
    void setZoom(float newZoom) { m_Zoom = newZoom; }

    const std::array<float, 3>& getCurrentColor() const { return m_CurrentColor; }
    void setCurrentColor(const std::array<float, 3>& newColor) { m_CurrentColor = newColor; }

    const std::array<float, 3>& getCanvasColor() const { return m_CanvasColor; }
    void setCanvasColor(const std::array<float, 3>& newColor) { m_CanvasColor = newColor; }

    float getCurrentThickness() const { return m_CurrentThickness; }
    void setCurrentThickness(float newThickness) { m_CurrentThickness = newThickness; }


};


