
#pragma once
#include <time.h>

#define MAX_NAME                50
#define MAX_EMAIL               100
#define MAX_MSG                 512
#define MAX_CLIENTS             100
#define MAX_PASS                256
#define MAX_HISTORY_MESSAGES    100

enum PacketType {
    PACKET_LOGIN = 1,
    PACKET_REGISTER,
    PACKET_USER_LIST,
    PACKET_LOGIN_RESULT,
    PACKET_REGISTER_RESULT,
    PACKET_MESSAGE,
    PACKET_MESSAGE_HISTORY,
    PACKET_USER_STATUS,
    PACKET_LOGOUT,
    PACKET_REQUEST_HISTORY,
    PACKET_MESSAGE_RESULT
};

struct PacketLogin {
    PacketType type = PACKET_LOGIN;
    wchar_t username[MAX_NAME];
    wchar_t password[MAX_PASS];
};

struct PacketRegister {
    PacketType type = PACKET_REGISTER;
    wchar_t username[MAX_NAME];
    wchar_t email[MAX_EMAIL];
    wchar_t password[MAX_PASS];
};

struct PacketLoginResult {
    PacketType type = PACKET_LOGIN_RESULT;
    bool success;
    wchar_t msg[100];
};

struct PacketRegisterResult {
    PacketType type = PACKET_REGISTER_RESULT;
    bool success;
    wchar_t msg[100];
};

struct UserInfo {
    wchar_t username[MAX_NAME];
    wchar_t email[MAX_EMAIL];
    int userId = 0;
    bool online = false;
};

struct PacketUserList {
    PacketType type = PACKET_USER_LIST;
    int count;
    UserInfo users[MAX_CLIENTS];
};

struct PacketMessage {
    PacketType type = PACKET_MESSAGE;
    int senderId;
    wchar_t sender[MAX_NAME];
    int receiverId;
    wchar_t receiver[MAX_NAME];
    wchar_t message[MAX_MSG];
    time_t timestamp;
};

struct MessageHistory {
    int senderId;
    int receiverId;
    wchar_t sender[MAX_NAME];
    wchar_t receiver[MAX_NAME];
    wchar_t message[MAX_MSG];
    time_t timestamp;
};

struct PacketMessageHistory {
    PacketType type = PACKET_MESSAGE_HISTORY;
    int count;
    MessageHistory messages[MAX_HISTORY_MESSAGES];
};

struct PacketUserStatus {
    PacketType type = PACKET_USER_STATUS;
    wchar_t username[MAX_NAME];
    int userId;
    bool isOnline;
};

struct PacketLogout {
    PacketType type = PACKET_LOGOUT;
    wchar_t username[MAX_NAME];
};

struct PacketRequestHistory
{
    PacketType type = PACKET_REQUEST_HISTORY;
    int requesterId;
    int targetId;
};

struct PacketMessageResult {
    PacketType type = PACKET_MESSAGE_RESULT;
    bool success;
    wchar_t msg[100];
};
