#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "TcpServer.h"
#include "ClientHandler.h"
#include "Database.h"
#include <iostream>
#include <thread>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

static SOCKET g_server_socket = INVALID_SOCKET;
static std::vector<HANDLE> g_clientThreads;
static volatile bool g_server_running = false;

void StartTcpServer() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        OutputDebugStringA("[SERVICE] WSAStartup failed");
        return;
    }

    g_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_server_socket == INVALID_SOCKET) {
        OutputDebugStringA("[SERVICE] socket() failed");
        WSACleanup();
        return;
    }

    BOOL opt = TRUE;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999); 
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(g_server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        OutputDebugStringA("[SERVICE] bind() failed");
        closesocket(g_server_socket);
        WSACleanup();
        return;
    }

    if (listen(g_server_socket, SOMAXCONN) == SOCKET_ERROR) {
        OutputDebugStringA("[SERVICE] listen() failed");
        closesocket(g_server_socket);
        WSACleanup();
        return;
    }

    OutputDebugStringA("[SERVICE] Listening on TCP 9999");
    g_server_running = true;

    while (g_server_running) {
        sockaddr_in clientAddr;
        int len = sizeof(clientAddr);
        SOCKET client = accept(g_server_socket, (sockaddr*)&clientAddr, &len);
        if (client == INVALID_SOCKET) {
            if (!g_server_running) break;
            Sleep(50);
            continue;
        }

        HANDLE h = CreateThread(NULL, 0, ClientHandler, (LPVOID)client, 0, NULL);
        if (h) g_clientThreads.push_back(h);
    }

    OutputDebugStringA("[SERVICE] TCP Server stopped");
    WSACleanup();
}

void StopTcpServer() {
    g_server_running = false;
    if (g_server_socket != INVALID_SOCKET) {
        closesocket(g_server_socket);
        g_server_socket = INVALID_SOCKET;
    }
    
    // Wait for client threads to finish
    for (auto h : g_clientThreads) {
        WaitForSingleObject(h, 2000);
        CloseHandle(h);
    }
    g_clientThreads.clear();
}

