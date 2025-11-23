#include <map>
#include <string>
#include <mutex>
#include <winsock2.h>
#include <windows.h>
#include <vector>

struct UserSession {
    SOCKET socket;
    std::wstring username;
    bool isOnline;
};

class UserManager {
public:
    static std::map<std::wstring, UserSession> onlineUsers;
    static std::mutex usersMutex;

public:
    static std::vector<std::wstring> GetOnlineUsernames();

    static bool AddUser(const std::wstring& username, SOCKET socket);
    static bool RemoveUser(const std::wstring& username);
    static bool RemoveUserBySocket(SOCKET socket);

    static SOCKET GetUserSocket(const std::wstring& username);
    static bool IsUserOnline(const std::wstring& username);
    static std::map<std::wstring, UserSession> GetOnlineUsers();
    static int GetOnlineCount();

    static bool SendToUser(const std::wstring& username, const char* data, int size);
    static void Broadcast(const char* data, int size, const std::wstring& excludeUser = L"");
};
