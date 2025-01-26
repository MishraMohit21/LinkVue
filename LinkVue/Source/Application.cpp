//[Application.cpp]


#include "Application.h"
#include <iostream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <algorithm>


Application::Application()
    : m_Window(nullptr)
    , m_NetworkingThreadRunning(false)
    , m_NetworkingInitialized(false)
    , m_ShowModeSelection(true)
    , m_IsHost(true)
    , m_Port(5000)
{
    std::memset(m_IP, 0, sizeof(m_IP));
    strcpy(m_IP, "127.0.0.1");
}

Application::~Application()
{
    cleanup();
}

void Application::handleNetworkMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
    m_MessageQueue.push_back(message);
    // Forward the message to whiteboard
    m_Whiteboard.handleNetworkMessage(message);
}

void Application::handleClientConnection() {
    std::cout << "Client connected to application" << std::endl;
    // Add any specific client connection handling here
}

void Application::handleClientDisconnection() {
    std::cout << "Client disconnected from application" << std::endl;
    // Add any specific client disconnection handling here
}

void Application::cleanup()
{
    stopNetworkingThread();

    if (m_Window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        m_Window = nullptr;
    }
}

void Application::stopNetworkingThread()
{
    m_NetworkingThreadRunning = false;
    m_NetworkingInitialized = false;

    if (m_NetworkingThread.joinable()) {
        m_NetworkingThread.join();
    }

    std::lock_guard<std::mutex> lock(m_NetworkingMutex);
    m_Networking.reset();
}

void Application::startNetworkingThread()
{
    stopNetworkingThread();

    m_NetworkingThread = std::thread([this]() {
        try {
            auto newNetworking = std::make_unique<NetworkManager>(m_Port);

            // Set up callbacks
            newNetworking->setOnMessageReceived([this](const std::string& msg) {
                handleNetworkMessage(msg);
                });

            newNetworking->setOnClientConnected([this]() {
                handleClientConnection();
                });

            newNetworking->setOnClientDisconnected([this]() {
                handleClientDisconnection();
                });

            bool initialized = false;
            if (m_IsHost) {
                initialized = newNetworking->initializeHost();
            }
            else {
                initialized = newNetworking->initializeClient(m_IP);
            }

            if (initialized) {
                {
                    std::lock_guard<std::mutex> lock(m_NetworkingMutex);
                    m_Networking = std::move(newNetworking);
                    m_NetworkingInitialized = true;
                }
                m_NetworkingThreadRunning = true;

                // Start the network manager
                m_Networking->start();

                // Network message loop
                while (m_NetworkingThreadRunning) {
                    // Process any pending messages in the queue
                    std::vector<std::string> messages;
                    {
                        std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
                        messages.swap(m_MessageQueue);
                    }

                    // Process messages
                    for (const auto& msg : messages) {
                        m_Whiteboard.handleNetworkMessage(msg);
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(16));
                }
            }
            else {
                std::cerr << "Failed to initialize NetworkManager" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Networking thread error: " << e.what() << std::endl;
        }

        m_NetworkingThreadRunning = false;
        m_NetworkingInitialized = false;
        });
}

void Application::renderModeSelectionWindow() {
    ImGui::Begin("Networking Setup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Select Networking Mode:");

    if (ImGui::RadioButton("Host", m_IsHost)) {
        m_IsHost = true;
    }
    if (ImGui::RadioButton("Client", !m_IsHost)) {
        m_IsHost = false;
    }

    ImGui::Separator();

    if (!m_IsHost) {
        ImGui::InputText("Server IP", m_IP, sizeof(m_IP));
    }

    ImGui::InputInt("Port", &m_Port);
    m_Port = std::clamp(m_Port, 1024, 65535);

    if (ImGui::Button("Start")) {
        m_ShowModeSelection = false;
        startNetworkingThread();
    }

    ImGui::End();
}

void Application::renderMainApplication() {
    // Check if we have any network messages to process
    std::vector<std::string> messages;
    {
        std::lock_guard<std::mutex> lock(m_MessageQueueMutex);
        messages.swap(m_MessageQueue);
    }

    // Process any pending messages
    for (const auto& msg : messages) {
        m_Whiteboard.handleNetworkMessage(msg);
    }

    // Render whiteboard components
    m_Whiteboard.renderCanvas();
    m_Whiteboard.drawToolWindow();

    // If you need to send whiteboard updates over the network
    if (m_NetworkingInitialized && m_Networking) {
        std::string update = m_Whiteboard.getUpdateData();
        if (!update.empty()) {
            if (m_IsHost) {
                m_Networking->broadcastMessage(update);
            }
            else {
                m_Networking->sendMessage(update);
            }
        }
    }
}


bool Application::init()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set error callback
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_Window = glfwCreateWindow(1280, 720, "LinkVue", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(0); // Enable vsync

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        cleanup();
        return false;
    }
    glViewport(0, 0, 1280, 720);
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = false;

    if (!ImGui_ImplGlfw_InitForOpenGL(m_Window, true) ||
        !ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
        cleanup();
        return false;
    }

    SetDarkThemeColors();
    return true;
}

void Application::Run() {
    if (!m_Window) {
        std::cerr << "Cannot run application: Window not initialized" << std::endl;
        return;
    }

    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();

        // Start networking thread if needed
        if (!m_ShowModeSelection && !m_NetworkingThreadRunning) {
            startNetworkingThread();
        }

        // Render frame
        renderFrame();
    }

    // Cleanup when main loop ends
    cleanup();
}


void Application::renderFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    }

    if (m_ShowModeSelection) {
        renderModeSelectionWindow();
    }
    else {
        renderMainApplication();
    }

    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}


void Application::SetDarkThemeColors()
{
    ImGuiStyle& style = ImGui::GetStyle();
    

    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.32f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.32f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.10f, 0.15f, 0.75f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.15f, 0.20f, 0.23f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.90f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.78f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.85f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.32f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.25f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.32f, 0.36f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.27f, 0.32f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.33f, 0.90f, 0.33f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.33f, 0.90f, 0.33f, 0.75f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.33f, 0.90f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.33f, 0.90f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.20f, 0.75f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.33f, 0.90f, 0.33f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.55f, 0.90f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);


    //auto& style = ImGui::GetStyle();
    style.TabRounding = 4;
    style.ScrollbarRounding = 9;
    style.WindowRounding = 7;
    style.GrabRounding = 3;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ChildRounding = 4;
}
