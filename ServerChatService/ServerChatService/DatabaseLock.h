#pragma once
// DatabaseLock.h
#pragma once
#include <mutex>

class DatabaseLock
{
private:
    static std::mutex m_mutex;

public:
    // RAII guard – an toàn tuyệt đối
    class Guard
    {
    public:
        Guard() { m_mutex.lock(); }
        ~Guard() { m_mutex.unlock(); }
        // Không cho copy
        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;
    };
};

// ĐỊNH NGHĨA BIẾN STATIC Ở ĐÂY LUÔN – QUAN TRỌNG NHẤT!
std::mutex DatabaseLock::m_mutex;
