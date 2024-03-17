#include "SocketClient.h"
#include <iostream>
#include <ws2tcpip.h>
#include <thread>
#include <functional>


SocketClient::SocketClient(const std::string& serverAddr, int serverPort)
	: serverAddr(serverAddr), serverPort(serverPort), sock(INVALID_SOCKET), connected(false) {
	initializeWinsock();
}

SocketClient::~SocketClient() {
	disconnect();
	cleanupWinsock();
}

void SocketClient::initializeWinsock() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		exit(EXIT_FAILURE);
	}
}

void SocketClient::cleanupWinsock() {
	WSACleanup();
}


void SocketClient::connectToServer() {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);

	// Convert IP address from string to binary form
	if (inet_pton(AF_INET, serverAddr.c_str(), &server.sin_addr) <= 0) {
		std::cerr << "Invalid address/ Address not supported: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}


	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	connected = true;
}

void SocketClient::disconnect() {
	std::cout << "normal disconnect\n";
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
	connected = false;
}

void SocketClient::joinRoom(const std::string& roomName) {
	currentRoom = roomName;
	std::string joinCommand = "JOIN " + roomName + "\n";
	send(sock, joinCommand.c_str(), joinCommand.length(), 0);
}

void SocketClient::leaveRoom() {
	std::string leaveCommand = "LEAVE\n";
	send(sock, leaveCommand.c_str(), leaveCommand.length(), 0);
	currentRoom.clear();
}

bool SocketClient::sendMessage(const std::string& message) {
	int bytesSent = send(sock, message.c_str(), message.length(), 0);
	if (bytesSent == SOCKET_ERROR) {
		int error = WSAGetLastError();
		std::cerr << "send failed with error: " << error << std::endl;

		// Specific handling based on error code, if necessary
		switch (error) {
			// Handle specific errors if needed
		default:
			break;
		}

		return false;
	}
	return true;
}



void SocketClient::receiveMessages(std::function<void(const std::string&)> callback) {
	char buffer[1024];

	while (connected) {
		ZeroMemory(buffer, sizeof(buffer));
		int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			// Pass the received message to the callback
			std::string message(buffer, 0, bytesReceived);
			callback(message);
		}
		else if (bytesReceived == 0) {
			std::cout << "Server closed the connection\n";
			break; // Break the loop if connection is gracefully closed
		}
		else {
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK) {
				std::cerr << "recv failed with error: " << error << std::endl;
				break; // Break on errors that are not "would block"
			}
		}
	}
	disconnect(); // Ensure the client is properly disconnected
}