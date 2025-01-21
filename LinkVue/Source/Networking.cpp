#include "Networking.h"
#include <iostream>
#include <thread>

NetworkManager::NetworkManager(int port)
    : port(port), running(false), hostMode(false),
    listenSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

NetworkManager::~NetworkManager() {
    stop();
    WSACleanup();
}

bool NetworkManager::initializeHost() {
    hostMode = true;
    struct sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        return false;
    }

    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    return true;
}

bool NetworkManager::initializeClient(const std::string& hostAddress) {
    hostMode = false;
    struct sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, hostAddress.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        return false;
    }

    if (connect(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        return false;
    }

    clientSockets.push_back(listenSocket);
    return true;
}

void NetworkManager::handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (running) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            if (onMessageReceived) {
                onMessageReceived(std::string(buffer));
            }
        }
        else {
            if (onClientDisconnected) {
                onClientDisconnected();
            }
            break;
        }
    }
    closesocket(clientSocket);
}

void NetworkManager::clientListenerThread() {
    while (running) {
        if (hostMode) {
            SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                clientSockets.push_back(clientSocket);
                if (onClientConnected) {
                    onClientConnected();
                }
                std::thread(&NetworkManager::handleClient, this, clientSocket).detach();
            }
        }
        else {
            handleClient(listenSocket);
            break;
        }
    }
}

bool NetworkManager::sendMessage(const std::string& message) {
    if (!running || clientSockets.empty()) {
        return false;
    }
    return send(clientSockets[0], message.c_str(), static_cast<int>(message.size()), 0) != SOCKET_ERROR;
}

bool NetworkManager::broadcastMessage(const std::string& message) {
    if (!running || !hostMode) {
        return false;
    }
    bool success = true;
    for (auto& socket : clientSockets) {
        if (send(socket, message.c_str(), static_cast<int>(message.size()), 0) == SOCKET_ERROR) {
            success = false;
        }
    }
    return success;
}

void NetworkManager::setOnMessageReceived(std::function<void(const std::string&)> callback) {
    onMessageReceived = callback;
}

void NetworkManager::setOnClientConnected(std::function<void()> callback) {
    onClientConnected = callback;
}

void NetworkManager::setOnClientDisconnected(std::function<void()> callback) {
    onClientDisconnected = callback;
}

void NetworkManager::start() {
    if (!running) {
        running = true;
        std::thread(&NetworkManager::clientListenerThread, this).detach();
    }
}

void NetworkManager::stop() {
    running = false;
    cleanup();
}

void NetworkManager::cleanup() {
    for (auto& socket : clientSockets) {
        closesocket(socket);
    }
    clientSockets.clear();
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
}

bool NetworkManager::isRunning() const {
    return running;
}

bool NetworkManager::isHost() const {
    return hostMode;
}