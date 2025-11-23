#include "Database.h"
#include <Windows.h>
#include <mutex>
#pragma comment(lib, "sqlite3.lib")


sqlite3* Database::db = nullptr;
//std::recursive_mutex Database::dbMutex;

std::recursive_mutex Database::dbMutex;  

bool Database::Exec16(const wchar_t* sql) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        OutputDebugStringA(sqlite3_errmsg(db));
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        OutputDebugStringA(sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::Init() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* lastSlash = strrchr(path, '\\');
    if (lastSlash) *lastSlash = 0;
    strcat_s(path, "\\sqlite333.db");

        char debugMsg[512];
    sprintf_s(debugMsg, " Database path: %s\n", path);
    OutputDebugStringA(debugMsg);

    if (sqlite3_open(path, &db) != SQLITE_OK) {
        sprintf_s(debugMsg, " SQLite open failed: %s\n", sqlite3_errmsg(db));
        OutputDebugStringA(debugMsg);
        return false;
    }
   
    OutputDebugStringA("SQLite opened successfully with UTF-16 support\n");

    sqlite3_busy_timeout(db, 3000);

    const wchar_t* createUsers =
        L"CREATE TABLE IF NOT EXISTS users ("
        L"id INTEGER PRIMARY KEY AUTOINCREMENT,"
        L"username TEXT UNIQUE NOT NULL COLLATE NOCASE,"
        L"email TEXT NOT NULL,"
        L"password TEXT NOT NULL);";

    const wchar_t* createMessages =
        L"CREATE TABLE IF NOT EXISTS messages ("
        L"id INTEGER PRIMARY KEY AUTOINCREMENT,"
        L"sender_id INTEGER NOT NULL,"
        L"receiver_id INTEGER NOT NULL DEFAULT 0,"
        L"message TEXT NOT NULL,"
        L"timestamp INTEGER NOT NULL,"
        L"FOREIGN KEY(sender_id) REFERENCES users(id),"
        L"FOREIGN KEY(receiver_id) REFERENCES users(id));";

    return Exec16(createUsers) && Exec16(createMessages);
      }
bool Database::RegisterUser(const wchar_t* username, const wchar_t* email, const wchar_t* password) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);

    const wchar_t* sql = L"INSERT INTO users(username, email, password) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {  
        return false;
    }
   
    sqlite3_bind_text16(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text16(stmt, 2, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text16(stmt, 3, password, -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
  }

bool Database::CheckLogin(const wchar_t* username, const wchar_t* password) {
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    
    const wchar_t* sql = L"SELECT id FROM users WHERE username = ? AND password = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text16(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text16(stmt, 2, password, -1, SQLITE_TRANSIENT);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
    
   }
int Database::GetUserId(const wchar_t* username)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    const wchar_t* sql = L"SELECT id FROM users WHERE username = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text16(stmt, 1, username, -1, SQLITE_TRANSIENT);
    int id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        id = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return id;
}

bool Database::GetUsername(int userId, wchar_t* username, size_t size)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    const wchar_t* sql = L"SELECT username FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);
    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const wchar_t* name = (const wchar_t*)sqlite3_column_text16(stmt, 0);
        if (name) wcscpy_s(username, size, name);
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}


bool Database::GetMessageHistory(int currentUserId, int targetUserId,
    MessageHistory* history, int maxCount, int* actualCount)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    *actualCount = 0;

    sqlite3_stmt* stmt = nullptr;
        const wchar_t* sql =
        L"SELECT sender_id, receiver_id, message, timestamp "
        L"FROM messages "
        L"WHERE (sender_id = ? AND receiver_id = ?) "
        L"   OR (sender_id = ? AND receiver_id = ?) "
        L"ORDER BY timestamp ASC;";

    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, currentUserId);
    sqlite3_bind_int(stmt, 2, targetUserId);
    sqlite3_bind_int(stmt, 3, targetUserId);
    sqlite3_bind_int(stmt, 4, currentUserId);
    sqlite3_bind_int(stmt, 5, maxCount);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && count < maxCount)
    {
        const wchar_t* sender = (const wchar_t*)sqlite3_column_text16(stmt, 0);
        const wchar_t* receiver = (const wchar_t*)sqlite3_column_text16(stmt, 1);
        const wchar_t* msg = (const wchar_t*)sqlite3_column_text16(stmt, 2);
        time_t ts = sqlite3_column_int64(stmt, 3);

        wcscpy_s(history[count].sender, sender ? sender : L"Unknown");
        wcscpy_s(history[count].receiver, receiver ? receiver : L"Private");
        wcscpy_s(history[count].message, msg ? msg : L"");
        history[count].timestamp = ts;
        count++;
    }

    *actualCount = count;
    sqlite3_finalize(stmt);
    return true;
}

bool Database::SaveMessage(int senderId, int receiverId, const wchar_t* message, time_t timestamp)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    const wchar_t* sql = L"INSERT INTO messages(sender_id, receiver_id, message, timestamp) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, senderId);
    sqlite3_bind_int(stmt, 2, receiverId);
    sqlite3_bind_text16(stmt, 3, message, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, timestamp);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

void Database::Close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
        OutputDebugStringA("Database: Closed\n");
    }
}

bool Database::UserExists(const wchar_t* username)
{
    std::lock_guard<std::recursive_mutex> lock(dbMutex);
    const wchar_t* sql = L"SELECT id FROM users WHERE username = ? LIMIT 1;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare16_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text16(stmt, 1, username, -1, SQLITE_TRANSIENT);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return exists;
}


