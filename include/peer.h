#pragma once 

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <vector>
#include <thread>
#include <atomic>

class Peer {
public:
	Peer();
	~Peer();

	bool Start();
	void Stop();

private:
	bool InitSocket();
	void WorkerThread();

	HANDLE iocp = INVALID_HANDLE_VALUE;
	SOCKET listenSocket = INVALID_SOCKET;
	std::thread worker;
	std::atomic<bool> running{ false };
};

// Per-connection context
struct PerIOConnect {
	OVERLAPPED overlapped{};
	WSABUF wsabuf{};
	char buffer[1024];
	DWORD bytesTransferred = 0;
	DWORD flags = 0;
	SOCKET socket = INVALID_SOCKET;
};