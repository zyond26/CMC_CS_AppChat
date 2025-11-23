#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "ServerChatService.h"
#include "Database.h"

SERVICE_STATUS_HANDLE hStatus;
SERVICE_STATUS ServiceStatus = {};

static volatile bool g_ServiceRunning = false;

//void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
//void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
//
//
//int wmain() {
//    SERVICE_TABLE_ENTRY serviceTable[] = {
//        { (LPWSTR)L"ServerChatService", ServiceMain },
//           { NULL, NULL }
//    };
//
//    if (!StartServiceCtrlDispatcher(serviceTable)) {
//        return GetLastError();
//    }
//    return 0;
//}

static void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
    case SERVICE_CONTROL_STOP:
        OutputDebugStringA("Service: Received STOP command\n");
        ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(hStatus, &ServiceStatus);

        g_ServiceRunning = false;
        StopMyService();
        Database::Close();

        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &ServiceStatus);
        break;
    default:
        break;
    }
}

static void WINAPI ServiceMain(DWORD argc, LPWSTR* argv) {
    OutputDebugStringA("Service: ServiceMain started\n");

    hStatus = RegisterServiceCtrlHandler(L"ServerChatService", ServiceCtrlHandler);

    if (!hStatus) {
        OutputDebugStringA("Service: RegisterServiceCtrlHandler failed\n");
        return;
    }

    ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwWin32ExitCode = NO_ERROR;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    SetServiceStatus(hStatus, &ServiceStatus);

    OutputDebugStringA("Service: Initializing...\n");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        OutputDebugStringA("Service: WSAStartup failed\n");
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        SetServiceStatus(hStatus, &ServiceStatus);
        return;
    }

    OutputDebugStringA("Service: Initializing database...\n");
    if (!Database::Init()) {
        OutputDebugStringA("Service: Database initialization failed\n");
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        SetServiceStatus(hStatus, &ServiceStatus);
        WSACleanup();
        return;
    }

    OutputDebugStringA("Service: Starting main service...\n");
    if (!StartMyService()) {
        OutputDebugStringA("Service: StartMyService failed\n");
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        SetServiceStatus(hStatus, &ServiceStatus);
        Database::Close();
        WSACleanup();
        return;
    }

    g_ServiceRunning = true;
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    SetServiceStatus(hStatus, &ServiceStatus);

    OutputDebugStringA("Service: Service is now RUNNING\n");

    while (g_ServiceRunning) {
        Sleep(1000);
    }

    OutputDebugStringA("Service: ServiceMain exiting\n");

    Database::Close();
    WSACleanup();

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    ServiceStatus.dwCheckPoint = 0;
    SetServiceStatus(hStatus, &ServiceStatus);
}

int main() {
    OutputDebugStringA("Service: main() started\n");

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPWSTR)L"ServerChatService", ServiceMain },
        { NULL, NULL }
    };


    if (!StartServiceCtrlDispatcher(serviceTable)) {
        DWORD error = GetLastError();

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return 1;
        }

        if (!Database::Init()) {
            WSACleanup();
            return 1;
        }

        if (!StartMyService()) {
            Database::Close();
            WSACleanup();
            return 1;
        }

        while (true) {
            if (GetAsyncKeyState('Q') & 0x8000) {
                break;
            }
            Sleep(1000);
        }

        StopMyService();
        Database::Close();
        WSACleanup();
    }
}
  

