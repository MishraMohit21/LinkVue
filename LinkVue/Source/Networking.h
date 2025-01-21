#include <string>
#include <vector>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class NetworkManager {
public:
    NetworkManager(int port = 12345);
    ~NetworkManager();

    // Initialize as host or client
    bool initializeHost();
    bool initializeClient(const std::string& hostAddress);

    // Send and receive messages
    bool sendMessage(const std::string& message);
    bool broadcastMessage(const std::string& message);

    // Set callbacks
    void setOnMessageReceived(std::function<void(const std::string&)> callback);
    void setOnClientConnected(std::function<void()> callback);
    void setOnClientDisconnected(std::function<void()> callback);

    // Start and stop networking
    void start();
    void stop();

    // Status checks
    bool isRunning() const;
    bool isHost() const;

private:
    static const int BUFFER_SIZE = 512;

    void handleClient(SOCKET clientSocket);
    void clientListenerThread();
    void cleanup();

    SOCKET listenSocket;
    std::vector<SOCKET> clientSockets;
    bool running;
    bool hostMode;
    int port;

    std::function<void(const std::string&)> onMessageReceived;
    std::function<void()> onClientConnected;
    std::function<void()> onClientDisconnected;
};