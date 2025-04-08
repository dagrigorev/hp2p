#include "peer.h"
#include <iostream>
#include <Winsock.h>
#include <WS2tcpip.h>
#include "flatbuffers/flatbuffers.h"
#include "message_generated.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

bool SendMessageFlat(SOCKET socket, const std::string& sender, hp2p::MessageType type, const std::string& payload);
void HandleReceivedMessage(const uint8_t* data, size_t size);

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

	CreateIoCompletionPort((HANDLE)listenSocket, iocp, (ULONG_PTR)listenSocket, 0);

	running = true;
	acceptThread = std::thread(&Peer::AcceptThread, this);
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

	if (acceptThread.joinable()) {
		acceptThread.join();
	}

	for (SOCKET s : clients) {
		closesocket(s);
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
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		LPOVERLAPPED overlapped = nullptr;

		BOOL result = GetQueuedCompletionStatus(iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);

		if (!result || completionKey == 0) {
			if (!running) break;
			std::cerr << "GetQueuedCompletionStatus failed" << std::endl;
			continue;
		}

		auto* context = reinterpret_cast<PerIOContext*>(overlapped);

		if (bytesTransferred == 0) {
			std::cerr << "[!] Connection closed" << std::endl;
			closesocket(context->socket);
			delete context;
			continue;
		}
		else {
			context->bytesTransferred = bytesTransferred;
		}

		HandleReceivedMessage(reinterpret_cast<const uint8_t*>(context->buffer), context->bytesTransferred);

		BeginReceive(context->socket);
		delete context;
	}
}

void Peer::AcceptThread() {
	while (running) {
		sockaddr_in clientAddr;
		int clinetSize = sizeof(clientAddr);

		SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clinetSize);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "[!] Accept failed" << std::endl;
			continue;
		}

		std::cout << "[+] Accepted connection" << std::endl;
		clients.push_back(clientSocket);

		if (!CreateIoCompletionPort((HANDLE)clientSocket, iocp, (ULONG_PTR)clientSocket, 0)) {
			std::cerr << "[!] CreateIoCompletionPort failed" << std::endl;
			closesocket(clientSocket);
			continue;
		}

		BeginReceive(clientSocket);
	}
}

void Peer::BeginReceive(SOCKET client) {
	auto* context = new PerIOContext();
	context->socket = client;
	context->wsabuf.buf = context->buffer;
	context->wsabuf.len = sizeof(context->buffer);

	DWORD flags = 0x0;
	DWORD recvBytes = 0;

	int res = WSARecv(client, &context->wsabuf, 1, &recvBytes, &flags, &context->overlapped, NULL);

	if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		std::cerr << "[!] WSARecv failed" << std::endl;
		closesocket(client);
		delete context;
	}
}

bool SendMessageFlat(SOCKET socket, const std::string& sender, hp2p::MessageType type, const std::string& payload) {
	flatbuffers::FlatBufferBuilder builder;

	auto sender_id_offset = builder.CreateString(sender);
	auto payload_offset = builder.CreateString(payload);

	auto message_offset = hp2p::CreateMessage(builder, sender_id_offset, type, payload_offset);

	builder.Finish(message_offset);

	const uint8_t* buf = builder.GetBufferPointer();
	int size = builder.GetSize();

	// Optionally: prefix the message with its size
	int sent = send(socket, reinterpret_cast<const char*>(buf), size, 0);
	return sent == size;
}

void HandleReceivedMessage(const uint8_t* data, size_t size) {
	auto *msg = hp2p::GetMessage(data);

	std::cout << "Got a new message...\n";

	flatbuffers::Verifier verifier(data, size);
	if (!msg->Verify(verifier)) {
		std::cerr << "[Error] Invalid FlatBuffer message.\n";
		return;
	}

	if (!msg) {
		std::cerr << "[Warning] Received invalid FlatBuffer message.\n";
		return;
	}

	std::string sender = msg->sender_id() ? msg->sender_id()->str() : "<unknown>";
	std::string payload = msg->payload() ? msg->payload()->str() : "";

	std::cout << "[Received] From: " << sender << ", Type: " << hp2p::EnumNameMessageType(msg->type()) << "\n";

	switch (msg->type()) {
	case hp2p::MessageType_HELLO:
		std::cout << "👋 HELLO: " << payload << "\n";
		break;
	case hp2p::MessageType_PING:
		std::cout << "📡 PING\n";
		break;
	case hp2p::MessageType_CHAT:
		std::cout << "💬 CHAT: " << payload << "\n";
		break;
	case hp2p::MessageType_METRIC_WARNING:
		std::cout << "⚠️ METRIC WARNING: " << payload << "\n";
		break;
	default:
		std::cout << "❓ Unknown message type.\n";
	}
}