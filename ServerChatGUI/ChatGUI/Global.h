//#pragma once
//
//#include <afxwin.h>
//#include <afxstr.h>
//
//class ClientSocket;
//class CServerChatGUIDlg;
//class ChatScreen;
//class RegisterScreen;
//
//#define WM_RECV_PACKET           (WM_APP + 100)
//#define WM_UPDATE_USERLIST     (WM_APP + 101)
//#define WM_UPDATE_USERSTATUS   (WM_APP + 102)
//#define WM_LOGIN_SUCCESS       (WM_APP + 103)
//#define WM_LOGIN_FAILED			(WM_APP + 104)
//#define WM_REGISTER_SUCCESS    (WM_APP + 105)
//#define WM_REGISTER_FAILED     (WM_APP + 106)
//
//extern HWND g_hwndMain;
//
//extern ClientSocket g_socket;
//extern CString g_username;
//extern ChatScreen* g_currentChatScreen;
//extern RegisterScreen* g_currentRegisterScreen;
//
//void SetCurrentChatScreen(ChatScreen*);
//void SetCurrentRegisterScreen(RegisterScreen*);

#pragma once
#include <afxwin.h>
#include <afxstr.h>

class ClientSocket;
class ChatScreen;
class RegisterScreen;

// Message
#define WM_RECV_CHAT           (WM_APP + 100)
#define WM_UPDATE_USERLIST     (WM_APP + 101)
#define WM_UPDATE_USERSTATUS   (WM_APP + 102)
#define WM_LOGIN_SUCCESS       (WM_APP + 103)
#define WM_LOGIN_FAILED        (WM_APP + 104)
#define WM_REGISTER_SUCCESS    (WM_APP + 105)
#define WM_REGISTER_FAILED     (WM_APP + 106)
#define WM_RECV_MESSAGE_HISTORY (WM_APP +107)

extern HWND g_hwndMain;                    
extern ClientSocket g_socket;
extern CString g_username;
extern CString g_currentUsername;
extern ChatScreen* g_currentChatScreen;
extern RegisterScreen* g_currentRegisterScreen;

void SetCurrentChatScreen(ChatScreen*);
void SetCurrentRegisterScreen(RegisterScreen*);

