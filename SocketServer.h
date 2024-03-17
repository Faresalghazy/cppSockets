#pragma once
#define SOCKETSERVER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

class SocketServer {
public:
	SocketServer(int port);
	~SocketServer();

	void start();
	void stop();

private:
	int server_fd;
	std::unordered_map<int, std::string> clients;
	std::unordered_map<std::string, std::vector<int>> rooms;
	std::mutex mtx;

	void acceptConnections();
	void handleClient(int client_fd);
	void broadcastMessage(const std::string& room, int sender_fd, const std::string& message);
	bool joinRoom(int client_fd, const std::string& room);
	bool leaveRoom(int client_fd, const std::string& room);
	void log(const std::string& message, bool isError = false);
};

