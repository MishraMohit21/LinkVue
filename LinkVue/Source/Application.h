// [Application.h]

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <string>
#include "Networking.h"
#include "Whiteboard.h"

class Application {
public:
    // Constructor & Destructor
    Application();
    ~Application();

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Core functions
    bool init();
    void Run();

private:
    // Initialization and cleanup
    void cleanup();
    void SetDarkThemeColors();

    // Rendering functions
    void renderFrame();
    void renderModeSelectionWindow();
    void renderMainApplication();

    // Networking
    void startNetworkingThread();

    // Window and rendering
    GLFWwindow* m_Window;

    // Networking members
    Networking* m_Networking;
    std::thread m_NetworkingThread;
    bool m_NetworkingThreadRunning;
    bool m_ShowModeSelection;
    Networking::Mode m_SelectedMode;
    char m_IP[16];  // Buffer for IP address
    int m_Port;

    // Application components
    Whiteboard m_Whiteboard;
};