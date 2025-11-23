#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>


#define SERVICE_NAME L"ServerChatService"

//extern BOOL Running;

bool StartMyService();
void StopMyService();


