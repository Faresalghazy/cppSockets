#include <iostream>
#include <cstdlib> // For std::rand and std::srand
#include <ctime> // For std::time
#include <string>
#include "SocketServer.h"
#include "SocketClient.h"
#include <chrono>
#include <thread>


// Mock function to generate a price feed string
std::string getPriceFeed() {
	int r1 = std::rand() % 100 + 1;
	int r2 = std::rand() % 100 + 1;
	int r3 = std::rand() % 100 + 1;
	return "XAUUSD:" + std::to_string(r1) + ";EURUSD:" + std::to_string(r2) + ";NAS100:" + std::to_string(r3) + "\n";
}

void setupPumpClient(std::string host, int port, std::string room) {
	SocketClient pumpClient(host, port);
	pumpClient.connectToServer();
	pumpClient.joinRoom(room);

	// Wait a bit after joining the room, ensuring the join message is processed
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for 1 second

	while (true) {
		std::string priceFeed = getPriceFeed();
		pumpClient.sendMessage(priceFeed);
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Send message every 500ms to prevent flooding
	}
}


void setupReadClient(std::string host, int port, std::string room) {
	std::cout << "Setting up read client..." << std::endl;
	SocketClient readClient(host, port);
	std::cout << "Attempting to connect to server..." << std::endl;
	readClient.connectToServer();
	std::cout << "Connected to server, joining room: " << room << std::endl;
	readClient.joinRoom(room);

	// Added logging to confirm read client's status
	std::cout << "Read client connected and joined room: " << room << std::endl;

	// Use a lambda function to process incoming messages
	std::thread receiveThread([&readClient]() {
		readClient.receiveMessages([](const std::string& message) {
			std::cout << "Received: " << message << std::endl;
			});
		std::cout << "Exiting receive thread..." << std::endl; // Log when exiting thread
		});

	if (receiveThread.joinable()) {
		receiveThread.join(); // Ensure the receive thread completes before exiting
	}

}



void setupServer(int port) {
	SocketServer server(port);
	server.start();
}
int main() {
	std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed the random number generator

	const std::string host = "127.0.0.1";
	const std::string room = "prices";
	const int port = 50001;
	std::cout << "Select an option:\n1. Setup Pump Client\n2. Setup Read Client\n3. Setup Server\n";
	int choice;
	std::cin >> choice;

	switch (choice) {
	case 1:
		setupPumpClient(host, port, room);
		break;
	case 2:
		setupReadClient(host, port, room);
		break;
	case 3:
		setupServer(port);
		break;
	default:
		std::cout << "Invalid choice. Exiting." << std::endl;
		return 1;
	}

	return 0;
}
