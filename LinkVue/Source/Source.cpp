//#include "Application.h"
//
//int main() {
//    Application LinkVue;
//
//
//    LinkVue.init();
//    LinkVue.Run();
//
//
//    return 0; 
//}

#include <iostream>
#include "Networking.h"


int main() {
    try {
        NetworkManager network(12345);

        std::string mode;
        std::cout << "Enter mode (host/client): ";
        std::cin >> mode;

        // Set up message callback
        network.setOnMessageReceived([](const std::string& msg) {
            std::cout << "Received: " << msg << std::endl;
            });

        // Initialize based on mode
        bool initialized = false;
        if (mode == "host") {
            initialized = network.initializeHost();
            network.setOnClientConnected([]() {
                std::cout << "Client connected!" << std::endl;
                });
        }
        else if (mode == "client") {
            initialized = network.initializeClient("127.0.0.1");
        }

        if (!initialized) {
            std::cerr << "Failed to initialize networking" << std::endl;
            return 1;
        }

        // Start networking
        network.start();

        // Main message loop
        std::string message;
        while (true) {
            std::getline(std::cin, message);
            if (network.isHost()) {
                network.broadcastMessage(message);
            }
            else {
                network.sendMessage(message);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}