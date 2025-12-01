ChatApp – Ứng dụng Chat Realtime Client/Server (C++ WinSock, MFC) 
 Giới thiệu
ChatAppWinApi là ứng dụng trò chuyện thời gian thực được xây dựng theo mô hình TCP Socket với kiến trúc Client – Server.
Server quản lý danh sách người dùng online, phân phối tin nhắn và xử lý phiên kết nối.
Client cung cấp giao diện thân thiện cho người dùng gửi và nhận tin nhắn nhanh chóng.

Dự án được phát triển bằng C++, WinSock2, và MFC (giao diện Windows).

Tính năng chính
🔹 Client
- Đăng ký / Đăng nhập tài khoản.
- Hiển thị đầy đủ danh sách người dùng trong giao diện chat
- Chat 1–1 (broadcast).
- Hiển thị đầy đủ lịch sử người dùng
- Giao diện trực quan (MFC).

🔹 Server
- dùng database là sqlite3 để lưu trữ quản lý dữ liệu
- có dùng SHA-256 để hash mật khẩu bảo mật 
- Quản lý session người dùng (UserManager).
- Broadcast tin nhắn đến các client khác.
- Hỗ trợ nhiều kết nối đồng thời (multithread).
- Xử lý disconnect an toàn, tránh crash khi nhiều client kết nối.

    
Luồng hoạt động
Client ----> Server : Gửi packet (login, message, request user list)
Server ----> Client : Trả về packet tương ứng
Server ----> All Clients : Broadcast tin nhắn mới

Công nghệ & Thư viện
C++17
WinSock2
Multithreading (std::thread + mutex)
MFC (Microsoft Foundation Class)
Visual Studio 2022

Cách chạy dự án
1️⃣ Build Server
Mở ServerChatService.sln
Build → Run ( nếu lỗi thì cài thêm thư viện sqlite3 về là xong -> link : https://sqlite.org/ )
Server chạy mặc định tại 127.0.0.1:9999 (có thể thay đổi).

2️⃣ Build Client
Mở ServerChatGUI.sln
Build → Run nhiều instance để test nhiều user.

 Ảnh minh hoạ (Demo UI)
 <img width="1458" height="898" alt="image" src="https://github.com/user-attachments/assets/168465bc-b03b-4ef9-963a-7a0babfd3d7d" />



Tìm socket theo username

Giữ trạng thái online
