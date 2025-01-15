#pragma once

#include <steam/steamnetworkingsockets.h>
#include <string>
#include <vector>

class Networking {
public:
    enum class Mode {
        Host,
        Client
    };

    Networking(Mode mode, uint16_t port, const std::string& ip);
    ~Networking();

    bool Initialize();
    void Run();
    /*void SendMessage(const std::string& message);*/
    void Reinitialize(Mode mode, uint16_t port, const std::string& ip);
    void PSendMessage(const std::string&);

private:
    Mode m_Mode;
    uint16_t m_Port;
    std::string m_IP;
    ISteamNetworkingSockets* m_NetworkInterface = nullptr;
    HSteamListenSocket m_ListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup m_PollGroup = k_HSteamNetPollGroup_Invalid;
    HSteamNetConnection m_Connection = k_HSteamNetConnection_Invalid;

    std::vector<HSteamNetConnection> m_Connections;

    void PollIncomingMessages();
    void HandleMessage(ISteamNetworkingMessage* msg);
    uint32_t ConvertIPAddressToUint32(const std::string& ip);

    void Cleanup();
};


