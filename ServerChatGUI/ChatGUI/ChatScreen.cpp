#include "pch.h"
#include "ServerChatGUI.h"
#include "ServerChatGUIDlg.h"
#include "afxdialogex.h"
#include "ChatScreen.h"
#include "ClientSocket.h"
#include "Struct.h"
#include "Global.h"
#include "PacketDispatcher.h"
#include <ctime>

IMPLEMENT_DYNAMIC(ChatScreen, CDialogEx)

extern ClientSocket g_socket;

ChatScreen::ChatScreen(CWnd* pParent) : CDialogEx(IDD_DIALOG_ChatScreen, pParent), m_myUserId(0), m_currentTargetId(0) {
    g_currentChatScreen = this;
    m_currentChatWith = L"";
}
ChatScreen::~ChatScreen() {
    g_currentChatScreen = nullptr;
}

void ChatScreen::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_user, list_user);
    DDX_Control(pDX, IDC_LIST_mess, list_mess);
    DDX_Control(pDX, IDC_EDIT_text, edit_text);
    DDX_Control(pDX, IDC_BUTTON_send, btn_send);
    DDX_Control(pDX, IDC_BUTTON_logout, btn_logout);
    DDX_Control(pDX, IDC_BUTTON_save, btn_save);
}

BEGIN_MESSAGE_MAP(ChatScreen, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_send, &ChatScreen::OnBnClickedButtonsend)
    ON_BN_CLICKED(IDC_BUTTON_logout, &ChatScreen::OnBnClickedButtonlogout)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_user, &ChatScreen::OnLvnItemchangedListuser)
    ON_BN_CLICKED(IDC_BUTTON_save, &ChatScreen::OnBnClickedButtonsave)
    ON_MESSAGE(WM_RECV_CHAT, &ChatScreen::OnMessageReceived)
    ON_MESSAGE(WM_UPDATE_USERLIST, &ChatScreen::OnUpdateUserList)
    ON_MESSAGE(WM_UPDATE_USERSTATUS, &ChatScreen::OnUpdateUserStatus)
    ON_MESSAGE(WM_RECV_MESSAGE_HISTORY, &ChatScreen::OnMessageHistoryReceived)
END_MESSAGE_MAP()

BOOL ChatScreen::OnInitDialog() {
    CDialogEx::OnInitDialog();

    CString title;
    title.Format(L"Chat - %s", m_username);
    SetWindowText(title);

    list_user.SetView(LV_VIEW_DETAILS);
    list_user.InsertColumn(0, L"User", LVCFMT_LEFT, 200);
    list_user.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    list_mess.InsertColumn(0, L"Message", LVCFMT_LEFT, 600);
    list_mess.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    g_socket.SetCallback(GlobalPacketHandler);
    SetCurrentChatScreen(this);

    PacketType req = PACKET_USER_LIST;
    g_socket.Send(&req, sizeof(req));

    return TRUE;
}

void ChatScreen::OnBnClickedButtonsend() {
    CString text;
    edit_text.GetWindowText(text);
    if (text.IsEmpty()) {
        AfxMessageBox(L"Vui lòng nhập tin nhắn!");
        return;
    }

    if (m_currentChatWith.IsEmpty()) {
        AfxMessageBox(L"Vui lòng chọn người để chat!");
        return;
    }

    PacketMessage msg{};
    msg.type = PACKET_MESSAGE;
    wcscpy_s(msg.sender, m_username.GetString());
    wcscpy_s(msg.receiver, m_currentChatWith.GetString());
    wcscpy_s(msg.message, text.GetString());
    msg.timestamp = time(nullptr);

    if (!g_socket.IsConnect()) {
        AfxMessageBox(L"Mất kết nối đến server!");
        return;
    }

    g_socket.Send(&msg, sizeof(msg));
    edit_text.SetWindowText(L"");
}

void ChatScreen::AddMessage(const CString& msg) {
    int idx = list_mess.InsertItem(list_mess.GetItemCount(), msg);
    list_mess.EnsureVisible(idx, FALSE);
}

void ChatScreen::OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMLISTVIEW pNMItem = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    if ((pNMItem->uChanged & LVIF_STATE) && (pNMItem->uNewState & LVIS_SELECTED)) {
        if (pNMItem->iItem == -1) { *pResult = 0; return; }

        CString selectedName = list_user.GetItemText(pNMItem->iItem, 0);
        auto it = m_userIdMap.find(selectedName);
        if (it != m_userIdMap.end()) {
            m_currentChatWith = selectedName;
            m_currentTargetId = it->second;

            list_mess.DeleteAllItems();

            PacketRequestHistory req{};
            req.type = PACKET_REQUEST_HISTORY;
            req.requesterId = m_myUserId;
            req.targetId = m_currentTargetId;
            g_socket.Send(&req, sizeof(req));

            CString title;
            title.Format(L"Chat - %s -> %s", m_username, m_currentChatWith);
            SetWindowText(title);
        }
    }
    *pResult = 0;
}

LRESULT ChatScreen::OnMessageReceived(WPARAM wParam, LPARAM lParam) {
    auto msg = (PacketMessage*)wParam;
    if (!msg) return 0;

    CString sender(msg->sender);
    CString receiver(msg->receiver);

    if (m_userIdMap.find(sender) == m_userIdMap.end() && sender != m_username) {
        int idx = list_user.InsertItem(list_user.GetItemCount(), sender);
        m_userIdMap[sender] = msg->senderId;
        list_user.SetItemData(idx, msg->senderId);
    }

    CTime t(msg->timestamp);
    CString line;

    if (sender.CompareNoCase(m_username) == 0)
        line.Format(L"[%s] Bạn -> %s: %s", t.Format(L"%H:%M"), receiver, msg->message);
    else
        line.Format(L"[%s] %s -> Bạn: %s", t.Format(L"%H:%M"), sender, msg->message);

    CString chatUser = (sender.CompareNoCase(m_username) == 0) ? receiver : sender;

    if (chatUser.CompareNoCase(m_currentChatWith) == 0) AddMessage(line);

    delete msg;
    return 0;
}

LRESULT ChatScreen::OnUpdateUserList(WPARAM wParam, LPARAM lParam) {
    auto packet = (PacketUserList*)wParam;
    if (!packet) return 0;

    m_userIdMap.clear();
    list_user.DeleteAllItems();

    for (int i = 0; i < packet->count && i < MAX_CLIENTS; i++) {
        if (packet->users[i].username[0] == L'\0') continue;
        CString username(packet->users[i].username);
        int userId = packet->users[i].userId;
        if (userId <= 0) continue; 

        m_userIdMap[username] = userId;

        if (username.CompareNoCase(m_username) == 0) {
            m_myUserId = userId;

            PacketRequestHistory req{};
            req.type = PACKET_REQUEST_HISTORY;
            req.requesterId = m_myUserId;
            req.targetId = 0;
            g_socket.Send(&req, sizeof(req));
        }
        else {
            int idx = list_user.InsertItem(list_user.GetItemCount(), username);
            list_user.SetItemData(idx, userId);
        }
    }

    delete packet;
    return 0;
}

LRESULT ChatScreen::OnUpdateUserStatus(WPARAM wParam, LPARAM lParam) {
    auto p = (PacketUserStatus*)wParam;
    if (!p) return 0;

    CString name(p->username);
    bool found = false;

    for (int i = 0; i < list_user.GetItemCount(); i++) {
        if (list_user.GetItemText(i, 0).CompareNoCase(name) == 0) {
            found = true;
            if (!p->isOnline) list_user.DeleteItem(i);
            break;
        }
    }

    if (!found && p->isOnline) {
        int idx = list_user.InsertItem(list_user.GetItemCount(), name);
        m_userIdMap[name] = p->userId;
        list_user.SetItemData(idx, p->userId);
    }

    delete p;
    return 0;
}

LRESULT ChatScreen::OnMessageHistoryReceived(WPARAM wParam, LPARAM lParam) {
    auto packet = (PacketMessageHistory*)wParam;
    if (!packet) return 0;

    for (int i = 0; i < packet->count; i++) {
        const MessageHistory& msg = packet->messages[i];
        CTime t(msg.timestamp);
        CString line;

        CString sSender(msg.sender);
        CString sReceiver(msg.receiver);

        if (sReceiver.IsEmpty()) {
            line.Format(L"[%s] %s: %s", t.Format(L"%H:%M"),
                (sSender.CompareNoCase(m_username) == 0) ? L"Bạn" : sSender,
                msg.message);
        }
        else if (sSender.CompareNoCase(m_username) == 0) {
            line.Format(L"[%s] Bạn -> %s: %s", t.Format(L"%H:%M"), sReceiver, msg.message);
        }
        else if (sReceiver.CompareNoCase(m_username) == 0) {
            line.Format(L"[%s] %s -> Bạn: %s", t.Format(L"%H:%M"), sSender, msg.message);
        }
        else {
            line.Format(L"[%s] %s -> %s: %s", t.Format(L"%H:%M"), sSender, sReceiver, msg.message);
        }

        AddMessage(line);
    }

    delete packet;
    return 0;
}

void ChatScreen::OnBnClickedButtonlogout() {
    PacketLogout out{};
    out.type = PACKET_LOGOUT;
    wcscpy_s(out.username, m_username.GetString());
    g_socket.Send(&out, sizeof(out));

    DestroyWindow();
    if (g_hwndMain && ::IsWindow(g_hwndMain))
        ::ShowWindow(g_hwndMain, SW_SHOW);
}

void ChatScreen::OnBnClickedButtonsave()
{
    if (list_mess.GetItemCount() == 0)
    {
        MessageBox(L"Không có tin nhắn nào để lưu cả!", L"Thông báo", MB_ICONWARNING);
        return;
    }

    if (m_currentChatWith.IsEmpty())
    {
        MessageBox(L"Vui lòng chọn người để lưu lịch sử chat!", L"Thông báo", MB_ICONWARNING);
        return;
    }

    CString chatWith = m_currentChatWith;

    SYSTEMTIME st;
    GetLocalTime(&st);

    CString safeName = chatWith;
    safeName.Replace(L"\\", L"_");
    safeName.Replace(L"/", L"_");
    safeName.Replace(L":", L"-");
    safeName.Replace(L"*", L"_");
    safeName.Replace(L"?", L"_");
    safeName.Replace(L"\"", L"'");
    safeName.Replace(L"<", L"_");
    safeName.Replace(L">", L"_");
    safeName.Replace(L"|", L"_");

    CString fileName;
    fileName.Format(L"Chat_%s_%04d-%02d-%02d_%02d-%02d-%02d.txt",
        safeName,
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);

    OPENFILENAMEW ofn = { 0 };
    wchar_t szFile[MAX_PATH] = { 0 };
    wcscpy_s(szFile, MAX_PATH, fileName);

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetSaveFileNameW(&ofn))
        return;

    HANDLE hFile = CreateFileW(
        szFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(L"Không thể tạo file! Kiểm tra quyền ghi hoặc đường dẫn.", L"Lỗi", MB_ICONERROR);
        return;
    }

    WORD bom = 0xFEFF;
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, &bom, sizeof(bom), &dwBytesWritten, NULL);

    CString header;
    CString timeStr;
    timeStr.Format(L"%02d/%02d/%04d %02d:%02d:%02d",
        st.wDay, st.wMonth, st.wYear,
        st.wHour, st.wMinute, st.wSecond);

    header.Format(
        L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n"
        L"   LỊCH SỬ CHAT VỚI %s\r\n"
        L"   Người dùng: %s\r\n"
        L"   Thời gian lưu: %s\r\n"
        L"   Tổng số tin nhắn: %d\r\n"
        L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n\r\n",
        chatWith,
        m_username,
        timeStr,
        list_mess.GetItemCount()
    );

    WriteFile(hFile,
        (LPCVOID)(LPCTSTR)header,
        header.GetLength() * sizeof(wchar_t),
        &dwBytesWritten,
        NULL);

    for (int i = 0; i < list_mess.GetItemCount(); i++)
    {
        CString msg = list_mess.GetItemText(i, 0);
        msg += L"\r\n";
        WriteFile(hFile,
            (LPCVOID)(LPCTSTR)msg,
            msg.GetLength() * sizeof(wchar_t),
            &dwBytesWritten,
            NULL);
    }

    CloseHandle(hFile);

    MessageBox(L"Đã lưu lịch sử chat thành công!", L"Thành công", MB_ICONINFORMATION);
}