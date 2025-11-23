#include "pch.h"
#include "ServerChatGUI.h"
#include "ServerChatGUIDlg.h"
#include "afxdialogex.h"
#include "ChatScreen.h"
#include "ClientSocket.h"
#include "Struct.h"
#include "Global.h"
#include "PacketDispatcher.h"
#include <map>
#include <ctime>

IMPLEMENT_DYNAMIC(ChatScreen, CDialogEx)

ChatScreen::ChatScreen(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_ChatScreen, pParent), m_myUserId(0), m_currentTargetId(0)
{
    m_currentChatWith = L"";
}

ChatScreen::~ChatScreen()
{
    if (g_currentChatScreen == this) {
        g_currentChatScreen = nullptr;
    }
}

void ChatScreen::DoDataExchange(CDataExchange* pDX)
{
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
    ON_BN_CLICKED(IDC_BUTTON_save,&ChatScreen::OnBnClickedButtonsave)
    
    ON_MESSAGE(WM_RECV_CHAT, &ChatScreen::OnMessageReceived)
    ON_MESSAGE(WM_UPDATE_USERLIST, &ChatScreen::OnUpdateUserList)
    ON_MESSAGE(WM_UPDATE_USERSTATUS, &ChatScreen::OnUpdateUserStatus)
    ON_MESSAGE(WM_RECV_MESSAGE_HISTORY, &ChatScreen::OnMessageHistoryReceived)
    
END_MESSAGE_MAP()

BOOL ChatScreen::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CString title;
    title.Format(L"Chat - %s", m_username);
    SetWindowText(title);

    list_user.InsertColumn(0, L"User", LVCFMT_LEFT, 200);
    list_user.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    list_mess.InsertColumn(0, L"Message", LVCFMT_LEFT, 400);
    list_mess.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    g_socket.SetCallback(GlobalPacketHandler);
    SetCurrentChatScreen(this);

    return TRUE;
}

void ChatScreen::OnBnClickedButtonsend()
{
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

    CString line;
    CTime t(msg.timestamp);
    line.Format(L"[%s] Bạn → %s: %s", t.Format(L"%H:%M"), m_currentChatWith, msg.message);
    AddMessage(line);

    edit_text.SetWindowText(L"");
}
void ChatScreen::AddMessage(const CString& msg)
{
    int idx = list_mess.InsertItem(list_mess.GetItemCount(), msg);
    list_mess.EnsureVisible(idx, FALSE);
}

void ChatScreen::OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMItem = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if ((pNMItem->uChanged & LVIF_STATE) && (pNMItem->uNewState & LVIS_SELECTED)) {
        if (pNMItem->iItem == -1) return;

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
            title.Format(L"Chat - %s → %s", m_username, m_currentChatWith);
            SetWindowText(title);
        }
    }
    *pResult = 0;
}

 LRESULT ChatScreen::OnMessageReceived(WPARAM wParam, LPARAM lParam)
{
    auto msg = (PacketMessage*)wParam;
    if (!msg) return 0;

    if (m_userIdMap.find(msg->sender) == m_userIdMap.end()) {
        int idx = list_user.InsertItem(list_user.GetItemCount(), msg->sender);
        m_userIdMap[msg->sender] = msg->senderId;
        list_user.SetItemData(idx, msg->senderId);
    }

    CString line;
    CTime t(msg->timestamp);

    if (wcscmp(msg->sender, m_username) == 0)
        line.Format(L"[%s] Bạn → %s: %s", t.Format(L"%H:%M"), msg->receiver, msg->message);
    else
        line.Format(L"[%s] %s → Bạn: %s", t.Format(L"%H:%M"), msg->sender, msg->message);

    CString chatUser = (wcscmp(msg->sender, m_username) == 0) ? msg->receiver : msg->sender;

    if (chatUser == m_currentChatWith)
        AddMessage(line);

    delete msg;
    return 0;
}

LRESULT ChatScreen::OnUpdateUserList(WPARAM wParam, LPARAM lParam)
{
    auto packet = (PacketUserList*)wParam;
    if (!packet) return 0;

    
        m_userIdMap.clear();
    list_user.DeleteAllItems();

    for (int i = 0; i < packet->count && i < MAX_CLIENTS; i++) {
        if (packet->users[i].username[0] == L'\0') continue;

        CString username = packet->users[i].username;
        int userId = packet->users[i].userId;
        m_userIdMap[username] = userId;

        if (username == m_username) {
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

LRESULT ChatScreen::OnUpdateUserStatus(WPARAM wParam, LPARAM lParam)
{
    auto p = (PacketUserStatus*)wParam;
    if (!p) return 0;

    CString name = p->username;
    bool found = false;

    for (int i = 0; i < list_user.GetItemCount(); i++) {
        if (list_user.GetItemText(i, 0) == name) {
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

LRESULT ChatScreen::OnMessageHistoryReceived(WPARAM wParam, LPARAM lParam)
{
    auto packet = (PacketMessageHistory*)wParam;
    if (!packet) return 0;
    
    for (int i = 0; i < packet->count; i++) {
        const MessageHistory& msg = packet->messages[i];
        CTime t(msg.timestamp);
        CString line;

        if (wcscmp(msg.receiver, L"") == 0) {
            line.Format(L"[%s] %s: %s", t.Format(L"%H:%M"),
                wcscmp(msg.sender, m_username) == 0 ? L"Bạn" : msg.sender,
                msg.message);
        }
        else if (wcscmp(msg.sender, m_username) == 0) {
            line.Format(L"[%s] Bạn → %s: %s", t.Format(L"%H:%M"), msg.receiver, msg.message);
        }
        else if (wcscmp(msg.receiver, m_username) == 0) {
            line.Format(L"[%s] %s → Bạn: %s", t.Format(L"%H:%M"), msg.sender, msg.message);
        }
        else {
            line.Format(L"[%s] %s → %s: %s", t.Format(L"%H:%M"), msg.sender, msg.receiver, msg.message);
        }

        AddMessage(line);
    }

    delete packet;
    return 0;
    

}

void ChatScreen::OnBnClickedButtonlogout()
{
    DestroyWindow();
    if (g_hwndMain && ::IsWindow(g_hwndMain))
        ::ShowWindow(g_hwndMain, SW_SHOW);
}

void ChatScreen::OnBnClickedButtonsave()
{
    if (list_mess.GetItemCount() == 0)
    {
        AfxMessageBox(L"Không có tin nhắn nào để lưu cả!");
        return;
    }

        if (m_currentChatWith.IsEmpty())
        {
            AfxMessageBox(L"Vui lòng chọn người để lưu lịch sử chat!");
            return;
        }

    CString chatWith = m_currentChatWith;

    CTime now = CTime::GetCurrentTime();
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
    fileName.Format(L"Chat với %s_%04d-%02d-%02d_%02d-%02d-%02d.txt",
        safeName,
        now.GetYear(), now.GetMonth(), now.GetDay(),
        now.GetHour(), now.GetMinute(), now.GetSecond());

    CFileDialog dlg(FALSE, L"txt", fileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
        L"Text Files (*.txt)|*.txt|All Files (*.*)|*.*||", this);

    if (dlg.DoModal() != IDOK) return;

    CString pathName = dlg.GetPathName();

    CFile file;
    if (!file.Open(pathName, CFile::modeCreate | CFile::modeWrite))
    {
        AfxMessageBox(L"Không thể tạo file! Kiểm tra quyền ghi hoặc đường dẫn.");
        return;
    }

    WORD bom = 0xFEFF;
    file.Write(&bom, sizeof(bom));

    CString header;
    header.Format(L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n"
        L"     LỊCH SỬ CHAT VỚI %s\r\n"
        L"     Người dùng: %s\r\n"
        L"     Thời gian lưu: %s\r\n"
        L"     Tổng số tin nhắn: %d\r\n"
        L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\r\n\r\n",
        chatWith,
        m_username,
        now.Format(L"%d/%m/%Y lúc %H:%M:%S"),
        list_mess.GetItemCount());

    file.Write(header, header.GetLength() * sizeof(wchar_t));

    for (int i = 0; i < list_mess.GetItemCount(); i++)
    {
        CString msg = list_mess.GetItemText(i, 0);
        msg += L"\r\n";
        file.Write(msg, msg.GetLength() * sizeof(wchar_t));
    }

   

}

