#include "UserManager.h"
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <windows.h>

extern bool SendPacket(SOCKET s, const char* data, int len);

std::map<std::wstring, UserSession> UserManager::onlineUsers;
std::mutex UserManager::usersMutex;

static std::vector<std::pair<std::wstring, UserSession>> SnapshotOnlineUsers() {
    std::vector<std::pair<std::wstring, UserSession>> out;
    std::lock_guard<std::mutex> lock(UserManager::usersMutex);
    out.reserve(UserManager::onlineUsers.size());
    for (auto& p : UserManager::onlineUsers) {
        out.emplace_back(p.first, p.second);
    }
    return out;
}

bool UserManager::AddUser(const std::wstring& username, SOCKET socket) {
    std::lock_guard<std::mutex> lock(usersMutex);

    auto it = onlineUsers.find(username);
    if (it != onlineUsers.end()) {
        it->second.socket = socket;
        it->second.isOnline = true;

        char debugMsg[256];
        sprintf_s(debugMsg, "UserManager: User %ls reconnected (socket updated = %d)\n",
            username.c_str(), (int)socket);
        OutputDebugStringA(debugMsg);
        return true;
    }

    UserSession session;
    session.username = username;
    session.socket = socket;
    session.isOnline = true;

    onlineUsers[username] = session;

    char debugMsg[256];
    sprintf_s(debugMsg, "UserManager: User %ls added (socket %d). Total: %zu\n",
        username.c_str(), (int)socket, onlineUsers.size());
    OutputDebugStringA(debugMsg);

    return true;
}

bool UserManager::RemoveUser(const std::wstring& username) {
    std::lock_guard<std::mutex> lock(usersMutex);
    auto it = onlineUsers.find(username);
    if (it == onlineUsers.end()) return false;

    onlineUsers.erase(it);

    char debugMsg[256];
    sprintf_s(debugMsg, "UserManager: User %ls removed\n", username.c_str());
    OutputDebugStringA(debugMsg);

    return true;
}


bool UserManager::RemoveUserBySocket(SOCKET socket) {
    std::lock_guard<std::mutex> lock(usersMutex);

    for (auto it = onlineUsers.begin(); it != onlineUsers.end(); ++it) {
        if (it->second.socket == socket) {
            std::wstring username = it->first;

            if (it->second.socket != INVALID_SOCKET) {
                shutdown(it->second.socket, SD_BOTH);
                closesocket(it->second.socket);
            }

            onlineUsers.erase(it);

            char debugMsg[256];
            sprintf_s(debugMsg, "UserManager: User %ls removed by socket %d\n",
                username.c_str(), (int)socket);
            OutputDebugStringA(debugMsg);
            return true;
        }
    }
    return false;
}

SOCKET UserManager::GetUserSocket(const std::wstring& username) {
    std::lock_guard<std::mutex> lock(usersMutex);
    auto it = onlineUsers.find(username);
    if (it != onlineUsers.end() && it->second.isOnline)
        return it->second.socket;
    return INVALID_SOCKET;
}

bool UserManager::SendToUser(const std::wstring& username, const char* data, int size) {
    SOCKET target;
    {
        std::lock_guard<std::mutex> lock(usersMutex);
        auto it = onlineUsers.find(username);
        if (it == onlineUsers.end() || !it->second.isOnline) {
            char debugMsg[256];
            sprintf_s(debugMsg, "UserManager: Cannot send to %ls - not online\n", username.c_str());
            OutputDebugStringA(debugMsg);
            return false;
        }
        target = it->second.socket;
    }

    if (!::SendPacket(target, data, size)) {
        RemoveUser(username);
        char debugMsg[256];
        sprintf_s(debugMsg, "UserManager: SendPacket to %ls failed -> removed\n", username.c_str());
        OutputDebugStringA(debugMsg);
        return false;
    }

    return true;
}

void UserManager::Broadcast(const char* data, int size, const std::wstring& excludeUser) {
    std::vector<std::pair<std::wstring, UserSession>> snapshot;
    {
        std::lock_guard<std::mutex> lock(usersMutex);
        snapshot.reserve(onlineUsers.size());
        for (const auto& p : onlineUsers) {
            snapshot.emplace_back(p.first, p.second);
        }
    }

    int success = 0;
    for (const auto& pr : snapshot) {
        const std::wstring& username = pr.first;
        const UserSession& session = pr.second;

        if (!session.isOnline) continue;
        if (username == excludeUser) continue;

        if (!::SendPacket(session.socket, data, size)) {
            RemoveUser(username);
        }
        else {
            success++;
        }
    }

    char debugMsg[256];
    sprintf_s(debugMsg, "UserManager: Broadcast sent to %d/%zu users\n", success, snapshot.size());
    OutputDebugStringA(debugMsg);
}

std::vector<std::wstring> UserManager::GetOnlineUsernames() {
    std::vector<std::wstring> ret;
    std::lock_guard<std::mutex> lock(usersMutex);
    for (const auto& p : onlineUsers) {
        if (p.second.isOnline)
            ret.push_back(p.first);
    }
    return ret;
}

