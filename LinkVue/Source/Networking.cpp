#include "Networking.h"
#include <iostream>
#include <sstream>
#include <vector>
//#include <arpa/inet.h> // For htonl and htons (or use Winsock on Windows)
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

Networking::Networking(Mode mode, uint16_t port, const std::string& ip)
    : m_Mode(mode), m_Port(port), m_IP(ip) {
}

Networking::~Networking() {
    Cleanup();
}

bool Networking::Initialize() {
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        std::cerr << "GameNetworkingSockets_Init failed: " << errMsg << std::endl;
        return false;
    }

    m_NetworkInterface = SteamNetworkingSockets();

    if (m_Mode == Mode::Host) {
        SteamNetworkingIPAddr addr;
        addr.SetIPv4(INADDR_ANY, htons(m_Port)); // INADDR_ANY for host
        m_ListenSocket = m_NetworkInterface->CreateListenSocketIP(addr, 0, nullptr);
        m_PollGroup = m_NetworkInterface->CreatePollGroup();

        if (m_ListenSocket == k_HSteamListenSocket_Invalid || m_PollGroup == k_HSteamNetPollGroup_Invalid) {
            std::cerr << "Failed to initialize host." << std::endl;
            return false;
        }

        std::cout << "Host initialized on port " << m_Port << std::endl;
    }
    else {
        SteamNetworkingIPAddr addr;
        addr.SetIPv4(ConvertIPAddressToUint32(m_IP), htons(m_Port));
        m_Connection = m_NetworkInterface->ConnectByIPAddress(addr, 0, nullptr);

        if (m_Connection == k_HSteamNetConnection_Invalid) {
            std::cerr << "Failed to initialize client." << std::endl;
            return false;
        }

        std::cout << "Client initialized and connecting to " << m_IP << " on port " << m_Port << std::endl;
    }

    return true;
}

void Networking::Reinitialize(Mode mode, uint16_t port, const std::string& ip) {
    Cleanup(); // Clean up the existing connection
    m_Mode = mode;
    m_Port = port;
    m_IP = ip;
    if (!Initialize()) {
        std::cerr << "Failed to reinitialize networking." << std::endl;
    }
}

void Networking::Run() {
    while (true) {
        PollIncomingMessages();
    }
}

void Networking::PSendMessage(const std::string& message)
{
    if (m_Mode == Mode::Host) {
        for (auto conn : m_Connections) {
            m_NetworkInterface->SendMessageToConnection(conn, message.data(), message.size(), k_nSteamNetworkingSend_Reliable, nullptr);
        }
    }
    else if (m_Connection != k_HSteamNetConnection_Invalid) {
        m_NetworkInterface->SendMessageToConnection(m_Connection, message.data(), message.size(), k_nSteamNetworkingSend_Reliable, nullptr);
    }
}


void Networking::PollIncomingMessages() {
    ISteamNetworkingMessage* msgs[16];
    int msgCount = m_NetworkInterface->ReceiveMessagesOnPollGroup(m_PollGroup, msgs, 16);

    for (int i = 0; i < msgCount; ++i) {
        HandleMessage(msgs[i]);
        msgs[i]->Release();
    }
}

void Networking::HandleMessage(ISteamNetworkingMessage* msg) {
    std::string receivedMsg(static_cast<const char*>(msg->m_pData), msg->m_cbSize);
    std::cout << "Received: " << receivedMsg << std::endl;
    
    if (m_Mode == Mode::Host) {
        PSendMessage(receivedMsg); // Broadcast message to all clients
    }
}

void Networking::Cleanup() {
    if (m_NetworkInterface) {
        if (m_Mode == Mode::Host) {
            if (m_ListenSocket != k_HSteamListenSocket_Invalid) {
                m_NetworkInterface->CloseListenSocket(m_ListenSocket);
            }
            if (m_PollGroup != k_HSteamNetPollGroup_Invalid) {
                m_NetworkInterface->DestroyPollGroup(m_PollGroup);
            }
        }
        else if (m_Connection != k_HSteamNetConnection_Invalid) {
            m_NetworkInterface->CloseConnection(m_Connection, 0, nullptr, false);
        }

        GameNetworkingSockets_Kill();
    }
}

uint32_t Networking::ConvertIPAddressToUint32(const std::string& ip) {
    in_addr addr;
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        std::cerr << "Invalid IP address format: " << ip << std::endl;
        return INADDR_NONE;
    }
    return ntohl(addr.s_addr);
}
