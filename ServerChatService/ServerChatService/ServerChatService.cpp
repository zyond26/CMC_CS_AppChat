
#include "ServerChatService.h"
#include "TcpServer.h"
#include "Database.h"

bool g_ServerRunning = false;

bool StartMyService() {
    OutputDebugStringA("[SERVICE] StartMyService: Starting server...\n");

    g_ServerRunning = true;

    std::thread serverThread([]() {
        StartTcpServer();   
        });

    serverThread.detach();

    OutputDebugStringA("[SERVICE] StartMyService: Server thread started\n");
    return true;
}

void StopMyService() {
    OutputDebugStringA("[SERVICE] StopMyService: Stopping server...\n");

    g_ServerRunning = false;    
    StopTcpServer();            

    OutputDebugStringA("[SERVICE] StopMyService: Server stopped\n");
}
