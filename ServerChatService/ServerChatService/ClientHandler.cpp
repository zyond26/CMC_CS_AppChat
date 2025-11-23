#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <ctime>

#include "ClientHandler.h"
#include "Struct.h"
#include "Database.h"
#include "UserManager.h"

DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET client = (SOCKET)lpParam;
    char buffer[4096];
    std::wstring currentUsername;

    OutputDebugStringA("ClientHandler: New client connected\n");

    while (true) {
        int bytesReceived = recv(client, buffer, sizeof(buffer), 0);

        if (bytesReceived == 0) {
            OutputDebugStringA("ClientHandler: Client disconnected gracefully\n");
            break;
        }
        else if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            char debugMsg[128];
            sprintf_s(debugMsg, "ClientHandler: recv error %d\n", error);
            OutputDebugStringA(debugMsg);
            break;
        }

        if (bytesReceived < (int)sizeof(PacketType)) {
            OutputDebugStringA("ClientHandler: Packet too small, skipping\n");
            continue;
        }

        PacketType type = *(PacketType*)buffer;
        char debugMsg[256];
        sprintf_s(debugMsg, "ClientHandler: Packet type = %d\n", type);
        OutputDebugStringA(debugMsg);

        switch (type) {

        case PACKET_LOGIN: {
            if (bytesReceived < (int)sizeof(PacketLogin)) break;

            PacketLogin* p = (PacketLogin*)buffer;
            PacketLoginResult result{};
            result.type = PACKET_LOGIN_RESULT;

            if (!Database::CheckLogin(p->username, p->password)) {
                result.success = false;
                wcscpy_s(result.msg, L"Sai tên đăng nhập hoặc mật khẩu!");
                send(client, (char*)&result, sizeof(result), 0);
                break;
            }

            if (!UserManager::AddUser(p->username, client)) {
                result.success = false;
                wcscpy_s(result.msg, L"Tài khoản đang đăng nhập ở nơi khác!");
                send(client, (char*)&result, sizeof(result), 0);
                break;
            }

            currentUsername = p->username;
            result.success = true;
            wcscpy_s(result.msg, L"Đăng nhập thành công!");
            send(client, (char*)&result, sizeof(result), 0);

            {
                std::vector<std::wstring> onlineUsers = UserManager::GetOnlineUsernames();
                PacketUserList listPacket{};
                listPacket.type = PACKET_USER_LIST;
                listPacket.count = (int)onlineUsers.size();

                for (int i = 0; i < listPacket.count && i < MAX_CLIENTS; i++) {
                    wcsncpy_s(listPacket.users[i].username, onlineUsers[i].c_str(), MAX_NAME - 1);
                    listPacket.users[i].userId = Database::GetUserId(onlineUsers[i].c_str());
                    listPacket.users[i].online = true;
                }
                send(client, (char*)&listPacket, sizeof(listPacket), 0);
            }

            {
                PacketUserStatus status{};
                status.type = PACKET_USER_STATUS;
                wcscpy_s(status.username, currentUsername.c_str());
                status.isOnline = true;
                UserManager::Broadcast((char*)&status, sizeof(status), currentUsername);
            }

            break;
        }

        case PACKET_REGISTER: {
            if (bytesReceived < (int)sizeof(PacketRegister)) break;

            PacketRegister* reg = (PacketRegister*)buffer;
            PacketRegisterResult result{};
            result.type = PACKET_REGISTER_RESULT;

            if (Database::RegisterUser(reg->username, reg->email, reg->password)) {
                result.success = true;
                wcscpy_s(result.msg, L"Đăng ký thành công!");
            }
            else {
                result.success = false;
                wcscpy_s(result.msg, L"Tên đăng nhập đã tồn tại!");
            }
            send(client, (char*)&result, sizeof(result), 0);
            break;
        }

        case PACKET_MESSAGE:
        {
            if (bytesReceived < (int)sizeof(PacketMessage)) break;

            PacketMessage* p = (PacketMessage*)buffer;

            int senderId = Database::GetUserId(currentUsername.c_str());
            if (senderId == -1) break;

            int receiverId = 0;
            std::wstring receiverName = p->receiver;
            bool isPublic = (wcslen(p->receiver) == 0 || _wcsicmp(p->receiver, L"ALL") == 0);
            if (!isPublic) {
                receiverId = Database::GetUserId(p->receiver);
                if (receiverId == -1) {
                    receiverId = 0;
                    receiverName = L"ALL";
                    isPublic = true;
                }
            }
            else {
                receiverName = L"ALL";
            }

            if (!Database::SaveMessage(senderId, receiverId, p->message, p->timestamp)) {
                PacketMessageResult result{};
                result.type = PACKET_MESSAGE_RESULT;
                result.success = false;
                wcscpy_s(result.msg, L"Lưu tin nhắn thất bại!");
                send(client, (char*)&result, sizeof(result), 0);
                break;
            }

            PacketMessageResult result{};
            result.type = PACKET_MESSAGE_RESULT;
            result.success = true;
            wcscpy_s(result.msg, L"Tin nhắn đã gửi!");

            if (isPublic) {
                UserManager::Broadcast((char*)p, sizeof(PacketMessage), L"");
                PacketUserStatus status{};
                status.type = PACKET_USER_STATUS;
                wcscpy_s(status.username, currentUsername.c_str());
                status.isOnline = true;
                UserManager::Broadcast((char*)&status, sizeof(status), currentUsername);
            }
            else {
                send(client, (char*)p, sizeof(PacketMessage), 0); // gửi lại cho người gửi
                SOCKET receiverSocket = UserManager::GetUserSocket(receiverName);
                if (receiverSocket != INVALID_SOCKET) {
                    send(receiverSocket, (char*)p, sizeof(PacketMessage), 0);
                }
            }

            send(client, (char*)&result, sizeof(result), 0);
            break;
        }

        case PACKET_REQUEST_HISTORY:
        {
            if (bytesReceived < sizeof(PacketRequestHistory)) break;

            PacketRequestHistory* req = (PacketRequestHistory*)buffer;
            int currentUserId = Database::GetUserId(currentUsername.c_str());
            if (currentUserId == -1) break;
            
            int targetUserId = req->targetId;

            const int MAX_HISTORY = 100;
            MessageHistory history[MAX_HISTORY];
            int actualCount = 0;

            if (Database::GetMessageHistory(currentUserId, targetUserId, history, MAX_HISTORY, &actualCount))
            {
                PacketMessageHistory packet{};
                packet.type = PACKET_MESSAGE_HISTORY;
                packet.count = actualCount;

                for (int i = 0; i < actualCount; i++) {
                    packet.messages[i] = history[i];
                }

                int headerSize = sizeof(PacketType) + sizeof(int);
                int dataSize = actualCount * sizeof(MessageHistory);
                int totalSize = headerSize + dataSize;

                send(client, (char*)&packet, totalSize, 0);

                char dbg[256];
                if (targetUserId == 0)
                    sprintf_s(dbg, "Sent %d public messages to %ls\n", actualCount, currentUsername.c_str());
                else
                    sprintf_s(dbg, "Sent %d private messages to %ls (with user %d)\n", actualCount, currentUsername.c_str(), targetUserId);
                OutputDebugStringA(dbg);
            }
            else
            {
                PacketMessageHistory emptyPacket{};
                emptyPacket.type = PACKET_MESSAGE_HISTORY;
                emptyPacket.count = 0;
                send(client, (char*)&emptyPacket, sizeof(PacketType) + sizeof(int), 0);
            }
            break;
        }

        case PACKET_LOGOUT: {
            PacketLogout* p = (PacketLogout*)buffer;
            UserManager::RemoveUser(currentUsername);

            PacketUserStatus status{};
            status.type = PACKET_USER_STATUS;
            wcscpy_s(status.username, currentUsername.c_str());
            status.isOnline = false;
            UserManager::Broadcast((char*)&status, sizeof(status), currentUsername);

            closesocket(client);
            return 0;
        }

        default:
            break;
        }
    }

    if (!currentUsername.empty()) {
        UserManager::RemoveUser(currentUsername);

        PacketUserStatus status{};
        status.type = PACKET_USER_STATUS;
        wcscpy_s(status.username, currentUsername.c_str());
        status.isOnline = false;
        UserManager::Broadcast((char*)&status, sizeof(status), currentUsername);
    }

    closesocket(client);
    OutputDebugStringA("ClientHandler: Client handler exited\n");
    return 0;
}