#include "pch.h"
#include "ServerChatGUI.h"
#include "afxdialogex.h"
#include "RegisterScreen.h"
#include "ServerChatGUIDlg.h"
#include "ClientSocket.h"
#include "Struct.h"
#include "Global.h"
#include "PacketDispatcher.h" 
#include <string>

IMPLEMENT_DYNAMIC(RegisterScreen, CDialogEx)

RegisterScreen::RegisterScreen(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_Register, pParent) {
}

void RegisterScreen::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_user, edit_user);
    DDX_Control(pDX, IDC_EDIT_pass, edit_pass);
    DDX_Control(pDX, IDC_EDIT_email, edit_email);
}

BEGIN_MESSAGE_MAP(RegisterScreen, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_register, &RegisterScreen::OnBnClickedButtonregister)
    ON_BN_CLICKED(IDC_BUTTON_login, &RegisterScreen::OnBnClickedButtonlogin)
    ON_MESSAGE(WM_REGISTER_SUCCESS, &RegisterScreen::OnRegisterSuccess)
    ON_MESSAGE(WM_REGISTER_FAILED, &RegisterScreen::OnRegisterFailure)  
END_MESSAGE_MAP()

BOOL RegisterScreen::OnInitDialog() {
    CDialogEx::OnInitDialog();

    OutputDebugStringA("RegisterScreen: Dialog initialized\n");

    return TRUE;
}

void RegisterScreen::OnBnClickedButtonregister() {
    CString user, pass, email;
    edit_user.GetWindowText(user);
    edit_pass.GetWindowText(pass);
    edit_email.GetWindowText(email);

    if (user.IsEmpty() || pass.IsEmpty() || email.IsEmpty()) {
        AfxMessageBox(L"Vui lòng điền đầy đủ thông tin!");
        return;
    }

    if (user.GetLength() >= MAX_NAME) {
        AfxMessageBox(L"Tên đăng nhập quá dài!");
        return;
    }

    if (email.GetLength() >= MAX_EMAIL) {
        AfxMessageBox(L"Email quá dài!");
        return;
    }

    if (pass.GetLength() < 6) {
        AfxMessageBox(L"Mật khẩu phải có ít nhất 6 ký tự!");
        return;
    }

    if (!g_socket.IsConnect()) {
        OutputDebugStringA("RegisterScreen: Attempting to connect...\n");
        if (!g_socket.Connect("127.0.0.1", 9999)) {
            AfxMessageBox(L"Không kết nối được server");
            return;
        }
    }
    PacketRegister p = {}; 

    std::string username = CT2A(user.GetString());
    std::string email_s = CT2A(email.GetString());
    std::string password = CT2A(pass.GetString());

    wcscpy_s(p.username, user);
    wcscpy_s(p.email, email);
    wcscpy_s(p.password, pass);

    char debugMsg[512];
    sprintf_s(debugMsg, "RegisterScreen: Sending register - user=%s, email=%s, pass=%s\n", p.username, p.email, p.password);
    OutputDebugStringA(debugMsg);

    g_socket.Send(&p, sizeof(p));
    OutputDebugStringA("RegisterScreen: Register packet sent\n");
}

LRESULT RegisterScreen::OnRegisterSuccess(WPARAM wParam, LPARAM lParam)
{
    AfxMessageBox(L"Đăng ký thành công!\nBạn có thể đăng nhập ngay bây giờ.");
    this->ShowWindow(SW_HIDE);

    PacketRegisterResult* p = (PacketRegisterResult*)wParam;

    if (p) delete p;  

    EndDialog(IDOK); 
    return 0;
}

LRESULT RegisterScreen::OnRegisterFailure(WPARAM wParam, LPARAM lParam)
{
    PacketRegisterResult* p = (PacketRegisterResult*)wParam;
    OutputDebugStringA("RegisterScreen: Register FAILED\n");

    if (p && p->msg[0] != L'\0')
    {
        AfxMessageBox(p->msg, MB_ICONERROR);
    }
    else
    {
        AfxMessageBox(L"Đăng ký thất bại! Vui lòng thử lại.", MB_ICONERROR);
    }

    if (p) delete p; 

    return 0;
}

void RegisterScreen::OnBnClickedButtonlogin() {
    EndDialog(IDCANCEL);
}