#include "pch.h"
#include "PacketDispatcher.h"
#include "ServerChatGUIDlg.h"
#include "RegisterScreen.h"
#include "ChatScreen.h"
#include "Struct.h"
#include "Global.h"

void SetCurrentChatScreen(ChatScreen* screen) {
    g_currentChatScreen = screen;
}

void SetCurrentRegisterScreen(RegisterScreen* screen) {
    g_currentRegisterScreen = screen;
}

void GlobalPacketHandler(const char* data, int size)
{
    if (size < sizeof(PacketType)) {
        OutputDebugStringA("[PacketDispatcher] Invalid packet size\n");
        return;
    }
    PacketType type = *(PacketType*)data;

    switch (type)
    {
    case PACKET_LOGIN_RESULT:
    {
        if (size < sizeof(PacketLoginResult)) return;
        PacketLoginResult* lg = (PacketLoginResult*)data;
        if (lg->success) {
            if (g_hwndMain && ::IsWindow(g_hwndMain))
                ::PostMessage(g_hwndMain, WM_LOGIN_SUCCESS, 0, 0);
        }
        else {
            CString* msg = new CString(lg->msg);
            if (g_hwndMain && ::IsWindow(g_hwndMain))
                ::PostMessage(g_hwndMain, WM_LOGIN_FAILED, (WPARAM)msg, 0);
        }
        break;
    }
   
    case PACKET_REGISTER_RESULT:
    {
        if (size < sizeof(PacketRegisterResult)) return;

        PacketRegisterResult* reg = new PacketRegisterResult();
        memcpy(reg, data, sizeof(PacketRegisterResult));

        if (g_currentRegisterScreen && ::IsWindow(g_currentRegisterScreen->m_hWnd))
        {
            if (reg->success)  {
                ::PostMessage(g_currentRegisterScreen->m_hWnd,WM_REGISTER_SUCCESS,(WPARAM)reg,0);
            }
            else{
                ::PostMessage(g_currentRegisterScreen->m_hWnd,WM_REGISTER_FAILED,(WPARAM)reg,0);
            }
        }
        else {
            delete reg; 
        }
        break;
    }

    case PACKET_MESSAGE:
    {
        if (size < sizeof(PacketMessage)) return;
        PacketMessage* mess = new PacketMessage();
        memcpy(mess, data, sizeof(PacketMessage));
        if (g_currentChatScreen && ::IsWindow(g_currentChatScreen->m_hWnd))
            ::PostMessage(g_currentChatScreen->m_hWnd, WM_RECV_CHAT, (WPARAM)mess, 0);
        else
            delete mess;
        break;
    }

    case PACKET_MESSAGE_HISTORY:
    {
        if (size < sizeof(PacketType) + sizeof(int))
            break;

        int count = *(int*)((char*)data + sizeof(PacketType));
        if (count < 0 || count > 100)
            break;

        int expected = sizeof(PacketType) + sizeof(int) + count * sizeof(MessageHistory);
        if (size < expected)
        {
            break; 
        }

        PacketMessageHistory* packet = new PacketMessageHistory();
        packet->type = PACKET_MESSAGE_HISTORY;
        packet->count = count;

        char* ptr = (char*)data + sizeof(PacketType) + sizeof(int);
        for (int i = 0; i < count; i++)
        {
            memcpy(&packet->messages[i], ptr, sizeof(MessageHistory));
            ptr += sizeof(MessageHistory);
        }

        if (g_currentChatScreen && ::IsWindow(g_currentChatScreen->m_hWnd))
            ::PostMessage(g_currentChatScreen->m_hWnd, WM_RECV_MESSAGE_HISTORY, (WPARAM)packet, 0);
        else
            delete packet;

        break;
    }
   
    case PACKET_USER_LIST:
    {
        if (size < sizeof(PacketUserList)) return;
        PacketUserList* uL = new PacketUserList();
        memcpy(uL, data, sizeof(PacketUserList));
        if (g_currentChatScreen && ::IsWindow(g_currentChatScreen->m_hWnd))
            ::PostMessage(g_currentChatScreen->m_hWnd, WM_UPDATE_USERLIST, (WPARAM)uL, 0);
        else
            delete uL;
        break;
    }

    case PACKET_USER_STATUS:
    {
        if (size < sizeof(PacketUserStatus)) return;
        PacketUserStatus* uS = new PacketUserStatus();
        memcpy(uS, data, sizeof(PacketUserStatus));
        if (g_currentChatScreen && ::IsWindow(g_currentChatScreen->m_hWnd))
            ::PostMessage(g_currentChatScreen->m_hWnd, WM_UPDATE_USERSTATUS, (WPARAM)uS, 0);
        else
            delete uS;
        break;
    }
  
    default:
        break;
    }
}
