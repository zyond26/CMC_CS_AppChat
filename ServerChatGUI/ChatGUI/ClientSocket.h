//#pragma once
//#include <winsock2.h>
//#include <thread>
//#include <atomic>
//
//class ClientSocket {
//private:
//    SOCKET sock;
//    std::atomic<bool> isConnected;
//    std::atomic<bool> running;
//    std::thread recvThread;
//    void(*callback)(const char*, int);
//
//public:
//    ClientSocket();
//    ~ClientSocket();
//
//    bool Connect(const char* serverIp, int port);
//    bool IsConnect();
//    void Disconnect();
//    void Send(const void* data, int size);
//    void SetCallback(void(*cb)(const char*, int));
//
//    void StartRecvThread();
//
//
////private:
////    void StartRecvThread();
//};


#pragma once
#include <thread>
#include <winsock2.h>
#include <windows.h>

class ClientSocket {
public:
    ClientSocket();
    ~ClientSocket();

    bool Connect(const char* serverIp, int port);
    void Disconnect();
    bool IsConnect();
    bool Send(const void* data, int size);
    void SetCallback(void(*cb)(const char*, int));

    //bool RequestHistory(int requesterId, int targetId);

private:
    void StartRecvThread();

private:
    SOCKET sock;
    bool isConnected;
    bool running;
    std::thread recvThread;
    void(*callback)(const char*, int);
};

