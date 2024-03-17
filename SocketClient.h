#pragma once
#include <winsock2.h> 
#include <string>
#include <functional>

class SocketClient {
public:
	SocketClient(const std::string& serverAddr, int serverPort);
	~SocketClient();

	void connectToServer();
	void disconnect();

	void joinRoom(const std::string& roomName);
	void leaveRoom();

	bool sendMessage(const std::string& message);
	void receiveMessages(std::function<void(const std::string&)> callback);

private:
	std::string serverAddr;
	int serverPort;
	SOCKET sock; // Use the SOCKET type for the socket descriptor
	std::string currentRoom;
	bool connected;

	void initializeWinsock();
	void cleanupWinsock();
};


