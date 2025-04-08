#include "peer.h"
#include <iostream>
#include <Winsock.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

Peer::Peer() {
	// Constructor
}

Peer::~Peer() {
	// Destructor
	Stop();
}

bool Peer::Start() {
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return false;
	}
	
	if (!InitSocket())
		return false;

	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!iocp) {
		std::cerr << "CreateIoCompletionPort failed." << std::endl;
		return false;
	}

	CreateIoCompletionPort((HANDLE)listenSocket, iocp, 0, 0);

	running = true;
	worker = std::thread(&Peer::WorkerThread, this);

	std::cout << "[+] Listening for connections on " << PORT << std::endl;
	return true;
}

void Peer::Stop() {
	running = false;

	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}

	if (iocp) {
		PostQueuedCompletionStatus(iocp, 0, 0, nullptr);
		CloseHandle(iocp);
		iocp = nullptr;
	}

	if (worker.joinable()) {
		worker.join();
	}

	WSACleanup();
}

bool Peer::InitSocket() {
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		std::cerr << "WSASocket failed" << std::endl;
		return false;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed" << std::endl;
		closesocket(listenSocket);
		return false;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "Listen failed" << std::endl;
		closesocket(listenSocket);
		return false;
	}

	return true;
}

void Peer::WorkerThread() {
	while (running) {
		DWORD bytesTransferred;
		ULONG_PTR completionKey;
		LPOVERLAPPED overlapped;

		BOOL result = GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);

		if (!result || completionKey == 0) {
			if (!running) break;
			std::cerr << "GetQueuedCompletionStatus failed" << std::endl;
			continue;
		}

		std::cout << "[*] New message received" << std::endl;
		// TODO: Process received message here
	}
}