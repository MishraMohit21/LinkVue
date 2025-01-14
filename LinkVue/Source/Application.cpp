//[Application.cpp]


#include "Application.h"
#include <iostream>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <algorithm>

Application::Application()
    : m_Window(nullptr)
    , m_Networking(nullptr)
    , m_NetworkingThreadRunning(false)
    , m_ShowModeSelection(true)
    , m_SelectedMode(Networking::Mode::Host)
    , m_Port(5000) // Default port
{
    std::memset(m_IP, 0, sizeof(m_IP));
    strcpy(m_IP, "127.0.0.1"); // Default IP
}

Application::~Application()
{
    cleanup();
}

void Application::cleanup()
{
   /* if (m_NetworkingThreadRunning) {
        m_NetworkingThreadRunning = false;
        if (m_NetworkingThread.joinable()) {
            m_NetworkingThread.join();
        }
    }*/

    /*if (m_Networking) {
        delete m_Networking;
        m_Networking = nullptr;
    }*/

    if (m_Window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        m_Window = nullptr;
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
        /*if (!m_ShowModeSelection && !m_NetworkingThreadRunning) {
            startNetworkingThread();
        }*/

        // Render frame
        renderFrame();
    }

    // Cleanup when main loop ends
    cleanup();
}

void Application::startNetworkingThread() {
    m_NetworkingThread = std::thread([this]() {
        try {
            m_Networking = new Networking(m_SelectedMode, m_Port, m_IP);
            if (m_Networking->Initialize()) {
                m_NetworkingThreadRunning = true;
            }
            else {
                std::cerr << "Failed to initialize Networking" << std::endl;
                delete m_Networking;
                m_Networking = nullptr;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Networking thread error: " << e.what() << std::endl;
            if (m_Networking) {
                delete m_Networking;
                m_Networking = nullptr;
            }
        }
        });
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

void Application::renderModeSelectionWindow() {
    ImGui::Begin("Networking Setup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Select Networking Mode:");
    bool isHost = (m_SelectedMode == Networking::Mode::Host);
    bool isClient = (m_SelectedMode == Networking::Mode::Client);

    if (ImGui::RadioButton("Host", isHost)) {
        m_SelectedMode = Networking::Mode::Host;
    }
    if (ImGui::RadioButton("Client", isClient)) {
        m_SelectedMode = Networking::Mode::Client;
    }

    ImGui::Separator();

    if (m_SelectedMode == Networking::Mode::Client) {
        ImGui::InputText("Server IP", m_IP, sizeof(m_IP));
    }

    ImGui::InputInt("Port", &m_Port);
    m_Port = std::clamp(m_Port, 1024, 65535); // Ensure valid port range

    if (ImGui::Button("Start")) {
        m_ShowModeSelection = false;
    }

    ImGui::End();
}

void Application::renderMainApplication() {
    /*if (m_Networking) {
        m_Networking->Run();
    }*/

    m_Whiteboard.renderCanvas();
    m_Whiteboard.drawToolWindow();
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
