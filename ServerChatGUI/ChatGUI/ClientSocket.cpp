#include "pch.h"
#include "ClientSocket.h"
#include <thread>
#include <ws2tcpip.h>
#include "Global.h"
#include "Struct.h"

ClientSocket::ClientSocket() : sock(INVALID_SOCKET), isConnected(false), running(false), callback(nullptr) {
    static bool wsaInited = false;
    if (!wsaInited) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        wsaInited = true;
    }
}

ClientSocket::~ClientSocket() {
    Disconnect();
}

bool ClientSocket::Connect(const char* serverIp, int port) {
    if (isConnected) return true;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        OutputDebugStringA("ClientSocket: Socket creation failed\n");
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp, &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        char msg[128];
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }
    isConnected = true;
    running = true;
    StartRecvThread();
    return true;
}

bool ClientSocket::IsConnect() {
    return isConnected && sock != INVALID_SOCKET;
}

void ClientSocket::Disconnect() {
    running = false;
    isConnected = false;

    if (sock != INVALID_SOCKET) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
}
bool ClientSocket::Send(const void* data, int size) {
    if (!isConnected) {
        OutputDebugStringA("[ClientSocket]  Send failed - not connected\n");
        return false;
    }

    int sent = send(sock, (const char*)data, size, 0);
    if (sent == SOCKET_ERROR) {
        char debugMsg[128];
        int error = WSAGetLastError();
        sprintf_s(debugMsg, "[ClientSocket]  Send failed - error: %d\n", error);
        OutputDebugStringA(debugMsg);

        Disconnect();
        return false;
    }

    char debugMsg[128];
    sprintf_s(debugMsg, "[ClientSocket] Sent %d/%d bytes successfully\n", sent, size);
    OutputDebugStringA(debugMsg);
    return true;
}

void ClientSocket::StartRecvThread() {
    recvThread = std::thread([this]() {
        char buffer[4096];
        while (running && isConnected && sock != INVALID_SOCKET) {
            int bytes = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                if (callback) {
                    char* copy = new char[bytes];
                    memcpy(copy, buffer, bytes);

                    callback(copy, bytes);  
                }
            }
            else if (bytes == 0) {
                OutputDebugStringA("ClientSocket: Server closed connection\n");
                break;
            }
            else {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK && err != WSAEINTR) {
                    char msg[128];
                    sprintf_s(msg, "ClientSocket: recv error %d\n", err);
                    OutputDebugStringA(msg);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        isConnected = false;
        running = false;
        OutputDebugStringA("ClientSocket: Receive thread stopped\n");
        });
}
void ClientSocket::SetCallback(void(*cb)(const char*, int)) {
    this->callback = cb;
}



