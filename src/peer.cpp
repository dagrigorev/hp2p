#include "peer.h"
#include <iostream>
#include <Winsock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

Peer::Peer() {
	// Constructor
}

Peer::~Peer() {
	// Destructor
}

bool Peer::Start() {
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return false;
	}
	// Create IOCP
	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (iocp == NULL) {
		std::cerr << "CreateIoCompletionPort failed." << std::endl;
		return false;
	}
	
	// Create a listening socket
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed." << std::endl;
		return false;
	}
	// Bind the socket to a local address and port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed." << std::endl;
		return false;
	}
	// Start listening for incoming connections
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed." << std::endl;
		return false;
	}
	std::cout << "Peer started and listening on port 8080." << std::endl;
	return true;
}

void Peer::Stop() {
	// Close the listening socket
	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}
	// Close IOCP
	if (iocp != NULL) {
		CloseHandle(iocp);
		iocp = NULL;
	}
	// Cleanup Winsock
	WSACleanup();
	std::cout << "Peer stopped." << std::endl;
}