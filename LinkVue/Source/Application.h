#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <string>
#include <mutex>
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
    void stopNetworkingThread();
    void handleNetworkMessage(const std::string& message);
    void handleClientConnection();
    void handleClientDisconnection();

    // Window and rendering
    GLFWwindow* m_Window;

    // Networking members
    std::mutex m_NetworkingMutex;
    std::unique_ptr<NetworkManager> m_Networking;
    std::thread m_NetworkingThread;
    std::atomic<bool> m_NetworkingThreadRunning{ false };
    std::atomic<bool> m_NetworkingInitialized{ false };
    bool m_ShowModeSelection = true;
    bool m_IsHost = true;  // Replaced Mode enum with boolean
    char m_IP[16];  // Buffer for IP address
    int m_Port;

    // Message queue for network messages
    std::mutex m_MessageQueueMutex;
    std::vector<std::string> m_MessageQueue;

    // Application components
    Whiteboard m_Whiteboard;
};