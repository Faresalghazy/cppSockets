#include "SocketServer.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

SocketServer::SocketServer(int port) : server_fd(INVALID_SOCKET) {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		log("WSAStartup failed: " + std::to_string(result), true);
		throw std::runtime_error("WSAStartup failed");
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET) {
		log("Socket creation failed with error: " + std::to_string(WSAGetLastError()), true);
		WSACleanup();
		throw std::runtime_error("Socket creation failed");
	}

	sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(static_cast<u_short>(port));

	result = bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));
	if (result == SOCKET_ERROR) {
		log("Bind failed with error: " + std::to_string(WSAGetLastError()), true);
		closesocket(server_fd);
		WSACleanup();
		throw std::runtime_error("Bind failed");
	}

	if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
		log("Listen failed with error: " + std::to_string(WSAGetLastError()), true);
		closesocket(server_fd);
		WSACleanup();
		throw std::runtime_error("Listen failed");
	}

	log("Server initialized successfully on port: " + std::to_string(port));
}

SocketServer::~SocketServer() {
	std::lock_guard<std::mutex> guard(mtx);
	for (const auto& client : clients) {
		closesocket(client.first);
	}
	clients.clear();
	if (server_fd != INVALID_SOCKET) {
		closesocket(server_fd);
		server_fd = INVALID_SOCKET;
	}
	WSACleanup();
}

void SocketServer::start() {
	log("Server started");
	acceptConnections();
}

void SocketServer::stop() {
	std::lock_guard<std::mutex> lock(mtx);
	for (const auto& client : clients) {
		closesocket(client.first);
	}
	clients.clear();
	if (server_fd != INVALID_SOCKET) {
		closesocket(server_fd);
		server_fd = INVALID_SOCKET;
	}
	WSACleanup();
	log("Server stopped");
}

void SocketServer::acceptConnections() {
	sockaddr_in client_addr = {};
	int client_addr_size = sizeof(client_addr);

	while (true) {
		int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_addr_size);
		if (client_fd == INVALID_SOCKET) {
			log("Failed to accept connection: " + std::to_string(WSAGetLastError()), true);
			continue;
		}

		std::thread clientThread(&SocketServer::handleClient, this, client_fd);
		clientThread.detach();
		log("New client connected: " + std::to_string(client_fd));
	}
}

void SocketServer::handleClient(int client_fd) {
	char buffer[1024] = {};
	std::string roomName;
	bool connected = true;

	while (connected) {
		memset(buffer, 0, sizeof(buffer));
		int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
		if (bytes_received > 0) {
			std::string msg(buffer, bytes_received);

			// Handle JOIN command
			if (msg.rfind("JOIN ", 0) == 0) {
				roomName = msg.substr(5);
				roomName.pop_back(); // Remove the newline character at the end
				if (joinRoom(client_fd, roomName)) {
					log("Client " + std::to_string(client_fd) + " joined room: " + roomName);
				}
			}
			// Handle LEAVE command
			else if (msg == "LEAVE\n") {
				if (leaveRoom(client_fd, roomName)) {
					log("Client " + std::to_string(client_fd) + " left room: " + roomName);
					roomName.clear();
				}
			}
			// Process other messages
			else {
				broadcastMessage(roomName, client_fd, msg);
			}
		}
		else if (bytes_received == 0) {
			log("Client disconnected normally: " + std::to_string(client_fd));
			connected = false;
			break; // Exit loop on normal disconnection
		}
		else {
			log("Error receiving data from client: " + std::to_string(client_fd), true);
			connected = false;
			break; // Exit loop on error
		}
	}

	if (!roomName.empty()) {
		leaveRoom(client_fd, roomName);
	}
	closesocket(client_fd);
	log("Connection with client closed: " + std::to_string(client_fd));
}

bool SocketServer::joinRoom(int client_fd, const std::string& room) {
	std::lock_guard<std::mutex> guard(mtx);
	if (rooms.find(room) == rooms.end()) {
		rooms[room] = std::vector<int>();
	}
	rooms[room].push_back(client_fd);
	clients[client_fd] = room;
	return true;
}

bool SocketServer::leaveRoom(int client_fd, const std::string& room) {
	std::lock_guard<std::mutex> guard(mtx);
	if (rooms.find(room) == rooms.end()) {
		return false;
	}

	auto& members = rooms[room];
	members.erase(std::remove(members.begin(), members.end(), client_fd), members.end());
	clients.erase(client_fd);

	if (members.empty()) {
		rooms.erase(room);
	}
	return true;
}

void SocketServer::broadcastMessage(const std::string& room, int sender_fd, const std::string& message) {
	std::lock_guard<std::mutex> guard(mtx);
	if (rooms.find(room) != rooms.end()) {
		for (int client_fd : rooms[room]) {
			if (client_fd != sender_fd) {
				if (send(client_fd, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
					log("Failed to send message to client: " + std::to_string(client_fd), true);
				}
			}
		}
	}
}

void SocketServer::log(const std::string& message, bool isError) {
	if (isError) {
		std::cerr << "Error: " << message << std::endl;
	}
	else {
		std::cout << message << std::endl;
	}
}
