#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <iostream>
#include <cstring>

int main() {
    SteamDatagramErrMsg errMsg;

    // Initialize Steam Networking
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        std::cerr << "Failed to initialize Steam Networking: " << errMsg << std::endl;
        return 1;
    }

    // Create Interface
    ISteamNetworkingSockets* pNetworking = SteamNetworkingSockets();
    if (!pNetworking) {
        std::cerr << "Failed to get networking sockets interface!" << std::endl;
        GameNetworkingSockets_Kill();
        return 1;
    }

    // Create Listen Socket
    SteamNetworkingIPAddr serverAddress;
    serverAddress.Clear();
    serverAddress.m_port = 27015;

    HSteamListenSocket listenSocket = pNetworking->CreateListenSocketIP(serverAddress, 0, nullptr);
    if (listenSocket == k_HSteamListenSocket_Invalid) {
        std::cerr << "Failed to create listen socket!" << std::endl;
        GameNetworkingSockets_Kill();
        return 1;
    }

    std::cout << "Host is listening on port 27015..." << std::endl;

    // Accept incoming connections
    HSteamNetConnection incomingConnection;
    while (true) {
        incomingConnection = pNetworking->AcceptConnection(listenSocket);
        if (incomingConnection != k_HSteamNetConnection_Invalid) {
            std::cout << "Client connected: " << incomingConnection << std::endl;

            // Send a welcome message
            const char* message = "Hello, Client!";
            int messageLength = strlen(message) + 1; // +1 for null terminator

            pNetworking->SendMessageToConnection(
                incomingConnection,
                message,
                messageLength,
                k_nSteamNetworkingSend_Reliable,
                nullptr
            );
            break;
        }
    }

    // Close connection and cleanup
    pNetworking->CloseConnection(incomingConnection, 0, "Disconnected", true);
    pNetworking->CloseListenSocket(listenSocket);
    GameNetworkingSockets_Kill();

    return 0;
}