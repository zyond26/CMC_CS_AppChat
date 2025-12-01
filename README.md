# ChatApp – Ứng dụng Chat Client/Server (C++ WinSock, MFC)

[![C++](https://img.shields.io/badge/Language-C++17-blue)](https://isocpp.org/) 
[![Windows](https://img.shields.io/badge/Platform-Windows-green)](https://www.microsoft.com/windows)
[![MFC](https://img.shields.io/badge/GUI-MFC-orange)](https://learn.microsoft.com/en-us/cpp/mfc/mfc-desktop-applications)

# Giới thiệu
**ChatAppWinApi** là ứng dụng trò chuyện **thời gian thực**, xây dựng theo mô hình **TCP Socket** với kiến trúc **Client – Server**:

- **Server**: Quản lý danh sách người dùng online, phân phối tin nhắn và xử lý phiên kết nối.
- **Client**: Cung cấp giao diện thân thiện để người dùng gửi và nhận tin nhắn nhanh chóng.

Dự án được phát triển bằng **C++17**, **WinSock2**, và **MFC** (giao diện Windows).

## Tính năng chính

### 🔹 Client
- [x] Đăng ký / Đăng nhập tài khoản.
- [x] Hiển thị danh sách người dùng online.
- [x] Chat 1–1 và broadcast tin nhắn.
- [x] Hiển thị đầy đủ **lịch sử tin nhắn**.
- [x] Giao diện trực quan, dễ sử dụng (MFC).

### 🔹 Server
- [x] Sử dụng **SQLite3** để lưu trữ và quản lý dữ liệu.
- [x] Hash mật khẩu người dùng bằng **SHA-256**.
- [x] Quản lý phiên người dùng (**UserManager**).
- [x] Broadcast tin nhắn đến các client khác.
- [x] Hỗ trợ nhiều kết nối đồng thời (**multithreading**).
- [x] Xử lý disconnect an toàn, tránh crash khi nhiều client kết nối.

## Luồng hoạt động

- Client ---> Server : Gửi packet (login, message, request user list)
- Server ---> Client : Trả về packet tương ứng
- Server ---> All Clients : Broadcast tin nhắn mới

## Công nghệ & Thư viện
- **Ngôn ngữ**: C++17
- **Network**: WinSock2
- **Đa luồng**: std::thread + mutex
- **Giao diện**: MFC (Microsoft Foundation Class)
- **IDE**: Visual Studio 2022
- **Database**: SQLite3
- **Bảo mật**: SHA-256 cho mật khẩu

## Cách chạy dự án

### 1️⃣ Build Server
1. Mở `ServerChatService.sln` bằng Visual Studio 2022.
2. Build → Run.
3. thường sẽ bị gặp lỗi ko link được sqlite3.lib thì cài thêm **SQLite3** từ [https://sqlite.org/](https://sqlite.org/).  
   Server chạy mặc định tại **127.0.0.1:9999** (có thể thay đổi).

### 2️⃣ Build Client
1. Mở `ServerChatGUI.sln`.
2. Build → Run nhiều instance để test nhiều user.

## Ảnh minh hoạ 
<img width="800" alt="Demo UI" src="https://github.com/user-attachments/assets/168465bc-b03b-4ef9-963a-7a0babfd3d7d" />

## Liên kết
- SQLite3: [https://sqlite.org/](https://sqlite.org/)
- Visual Studio 2022: [https://visualstudio.microsoft.com/](https://visualstudio.microsoft.com/)
- MFC Documentation: [https://learn.microsoft.com/en-us/cpp/mfc/mfc-desktop-applications](https://learn.microsoft.com/en-us/cpp/mfc/mfc-desktop-applications)
