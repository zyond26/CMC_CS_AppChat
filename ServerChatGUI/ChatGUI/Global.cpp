//
//#include "pch.h"
//#include "Global.h"
//#include "ClientSocket.h"      // CHỈ include logic, KHÔNG include UI
//
//ClientSocket g_socket;
//CString g_username;
//
//
//HWND g_hwndMain = nullptr;
//
//ChatScreen* g_currentChatScreen = nullptr;
//RegisterScreen* g_currentRegisterScreen = nullptr;
//
//void SetCurrentChatScreen(ChatScreen* s)
//{
//    g_currentChatScreen = s;
//}
//
//void SetCurrentRegisterScreen(RegisterScreen* s)
//{
//    g_currentRegisterScreen = s;
//}


#include "pch.h"
#include "Global.h"
#include "ClientSocket.h"

ClientSocket g_socket;
CString g_username;
CString g_currentUsername;

HWND g_hwndMain = nullptr;
ChatScreen* g_currentChatScreen = nullptr;
RegisterScreen* g_currentRegisterScreen = nullptr;

