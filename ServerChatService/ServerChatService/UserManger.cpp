#include "UserManager.h"
#include <iostream>
#include<vector>

std::map<std::wstring, UserSession> UserManager::onlineUsers;
std::mutex UserManager::usersMutex;

std::vector<std::wstring> UserManager::GetOnlineUsernames() {
    std::vector<std::wstring> result;
    std::lock_guard<std::mutex> lock(usersMutex);
    for (const auto& pair : onlineUsers) {
        if (pair.second.isOnline)
            result.push_back(pair.first);
    }

    return result;
}

bool UserManager::AddUser(const std::wstring& username, SOCKET socket) {
    std::lock_guard<std::mutex> lock(usersMutex);

    auto it = onlineUsers.find(username);
    if (it != onlineUsers.end()) {

        it->second.socket = socket;
        it->second.isOnline = true;

        char debugMsg[256];
        sprintf_s(debugMsg, " UserManager: User %ls reconnected (socket updated to %d)\n",
            username.c_str(), socket);
        OutputDebugStringA(debugMsg);
        return true;
    }
    UserSession session;
    session.socket = socket;
    session.username = username;
    session.isOnline = true;
    onlineUsers[username] = session;

    char debugMsg[256];
    sprintf_s(debugMsg, " UserManager: User %ls added (socket %d). Online: %zu users\n",
        username.c_str(), socket, onlineUsers.size());
    OutputDebugStringA(debugMsg);
    return true;
}

bool UserManager::RemoveUser(const std::wstring& username) {
    std::lock_guard<std::mutex> lock(usersMutex);

    auto it = onlineUsers.find(username);
    if (it != onlineUsers.end()) {
        onlineUsers.erase(it);

        char debugMsg[256];
        sprintf_s(debugMsg, " UserManager: User %ls removed. Online: %zu users\n",
            username.c_str(), onlineUsers.size());
        OutputDebugStringA(debugMsg);
        return true;
    }

    char debugMsg[256];
    sprintf_s(debugMsg, " UserManager: User %ls not found for removal\n", username.c_str());
    OutputDebugStringA(debugMsg);
    return false;
}

bool UserManager::RemoveUserBySocket(SOCKET socket) {
    std::lock_guard<std::mutex> lock(usersMutex);

    for (auto it = onlineUsers.begin(); it != onlineUsers.end(); ++it) {
        if (it->second.socket == socket) {
            std::wstring username = it->first;
            onlineUsers.erase(it);

            char debugMsg[256];
            sprintf_s(debugMsg, " UserManager: User %ls removed by socket %d\n",
                username.c_str(), socket);
            OutputDebugStringA(debugMsg);
            return true;
        }
    }
}

SOCKET UserManager::GetUserSocket(const std::wstring& username) {
    std::lock_guard<std::mutex> lock(usersMutex);
    auto it = onlineUsers.find(username);
    if (it != onlineUsers.end() && it->second.isOnline) {
        return it->second.socket;
    }
    return INVALID_SOCKET;
}

bool UserManager::IsUserOnline(const std::wstring& username) {
    std::lock_guard<std::mutex> lock(usersMutex);
    auto it = onlineUsers.find(username);
    return (it != onlineUsers.end() && it->second.isOnline);
}

bool UserManager::SendToUser(const std::wstring& username, const char* data, int size) {
    SOCKET targetSocket = GetUserSocket(username);
    if (targetSocket == INVALID_SOCKET) {
        char debugMsg[256];
        sprintf_s(debugMsg, "UserManager: Cannot send to %ls - user not online\n", username.c_str());
        OutputDebugStringA(debugMsg);
        return false;
    }

    int bytesSent = send(targetSocket, data, size, 0);
    if (bytesSent == SOCKET_ERROR) {
        int error = WSAGetLastError();
        char debugMsg[256];
        sprintf_s(debugMsg, "UserManager: Send to %ls failed - error %d\n", username.c_str(), error);
        OutputDebugStringA(debugMsg);

        RemoveUser(username);
        return false;
    }

    char debugMsg[256];
    sprintf_s(debugMsg, " UserManager: Sent %d bytes to %ls\n", bytesSent, username.c_str());
    OutputDebugStringA(debugMsg);
    return true;
}

void UserManager::Broadcast(const char* data, int size, const std::wstring& excludeUser) {
    std::lock_guard<std::mutex> lock(usersMutex);

    if (onlineUsers.empty()) {
        return;
    }

    int successCount = 0;
    for (auto& pair : onlineUsers) {
        if (!pair.second.isOnline || pair.first == excludeUser)
            continue;

        int bytesSent = send(pair.second.socket, data, size, 0);
        if (bytesSent == SOCKET_ERROR) {
            pair.second.isOnline = false;
            char debugMsg[256];
            OutputDebugStringA(debugMsg);
        }
        else {
            successCount++;
        }
    }

    char debugMsg[256];
    sprintf_s(debugMsg, " UserManager: Broadcast completed - %d/%zu users received\n",
        successCount, onlineUsers.size());
    OutputDebugStringA(debugMsg);
}

std::map<std::wstring, UserSession> UserManager::GetOnlineUsers() {
    std::lock_guard<std::mutex> lock(usersMutex);
    return onlineUsers;
}

int UserManager::GetOnlineCount() {
    std::lock_guard<std::mutex> lock(usersMutex);
    int count = 0;
    for (const auto& pair : onlineUsers) {
        if (pair.second.isOnline) count++;
    }
    return count;
}


