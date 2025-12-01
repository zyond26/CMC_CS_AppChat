#include "pch.h"
#include "ClientSocket.h"
#include <ws2tcpip.h>
#include <chrono>

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
    if (!isConnected) return;

    running = false;
    isConnected = false;

    SOCKET s = sock;
    sock = INVALID_SOCKET;   // tránh thread access sock sau khi bị close

    if (s != INVALID_SOCKET) {
        shutdown(s, SD_BOTH);
        closesocket(s);
    }

    // BẮT BUỘC join SAU khi socket đã close
    if (recvThread.joinable()) {
        recvThread.join();
    }
}


bool ClientSocket::RecvAll(char* buf, int len) {
    int total = 0;
    while (total < len) {
        int ret = recv(sock, buf + total, len - total, 0);
        if (ret == 0) return false;
        if (ret < 0) {
            int err = WSAGetLastError();

            // Socket đã bị đóng từ thread khác
            if (err == WSAENOTSOCK) return false;

            if (err == WSAEWOULDBLOCK || err == WSAEINTR) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            return false;
        }
        total += ret;
    }
    return true;
}


bool ClientSocket::SendAll(const char* data, int size) {
    int sent = 0;
    while (sent < size) {
        int ret = send(sock, data + sent, size - sent, 0);
        if (ret == SOCKET_ERROR || ret == 0) {
            return false;
        }
        sent += ret;
    }
    return true;
}

bool ClientSocket::SendPacket(const void* data, int size) {
    if (!isConnected || sock == INVALID_SOCKET) return false;
    int len = size;
    if (!SendAll((char*)&len, sizeof(len))) return false;
    return SendAll((const char*)data, size);
}

bool ClientSocket::Send(const void* data, int size) {
    if (!isConnected) {
        OutputDebugStringA("[ClientSocket] Send failed - not connected\n");
        return false;
    }

    bool ok = SendPacket(data, size);
    if (!ok) {
        char debugMsg[128];
        int error = WSAGetLastError();
        sprintf_s(debugMsg, "[ClientSocket] SendPacket failed - error: %d\n", error);
        OutputDebugStringA(debugMsg);
        Disconnect();
        return false;
    }

    return true;
}

void ClientSocket::StartRecvThread() {
    recvThread = std::thread([this]() {
        while (running) {

            int packetSize = 0;
            if (!RecvAll((char*)&packetSize, 4)) break;
            if (packetSize <= 0 || packetSize > 100000) break;

            char* data = new char[packetSize];
            if (!RecvAll(data, packetSize)) {
                delete[] data;
                break;
            }

            if (callback)
                callback(data, packetSize);

            delete[] data;
        }

        OutputDebugStringA("ClientSocket: Receive thread stopped\n");
        });
}


void ClientSocket::SetCallback(void(*cb)(const char*, int)) {
    this->callback = cb;
}

