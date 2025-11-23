#include "pch.h"
#include "framework.h"
#include "ServerChatGUI.h"
#include "ServerChatGUIDlg.h"
#include "afxdialogex.h"
#include "ChatScreen.h"
#include "RegisterScreen.h"

#include "ClientSocket.h"
#include "Struct.h"
#include "PacketDispatcher.h"
#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <string>

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CServerChatGUIDlg::CServerChatGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATGUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerChatGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_user, edit_user);
	DDX_Control(pDX, IDC_EDIT_pass, edit_pass);
	DDX_Control(pDX, IDC_BUTTON_login, btn_login);
	DDX_Control(pDX, IDC_BUTTON_register, btn_register);
}

BEGIN_MESSAGE_MAP(CServerChatGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_login, &CServerChatGUIDlg::OnBnClickedButtonlogin)
	ON_BN_CLICKED(IDC_BUTTON_register, &CServerChatGUIDlg::OnBnClickedButtonregister)
	ON_MESSAGE(WM_LOGIN_SUCCESS, &CServerChatGUIDlg::OnLoginSuccess)
	ON_MESSAGE(WM_LOGIN_FAILED, &CServerChatGUIDlg::OnLoginFailed)
	//ON_MESSAGE(WM_RECV_PACKET, &CServerChatGUIDlg::OnRecvPacket)

END_MESSAGE_MAP()


BOOL CServerChatGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	g_hwndMain = this->m_hWnd;   

	if (!g_socket.IsConnect())
	{
		OutputDebugStringA("[MainDlg] Attempting to connect to server...\n");
		if (g_socket.Connect("127.0.0.1", 9999)) {
			OutputDebugStringA("[MainDlg] ✅ Connected to server successfully\n");
		}
		else {
			OutputDebugStringA("[MainDlg] ❌ Failed to connect to server\n");
		}
	}
	else {
		OutputDebugStringA("[MainDlg] Already connected to server\n");
	}

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	return TRUE;  
}

void CServerChatGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
void CServerChatGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CServerChatGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerChatGUIDlg::OnBnClickedButtonlogin() {
	CString user, pass;
	edit_user.GetWindowText(user);
	edit_pass.GetWindowText(pass);

	g_currentUsername = user;

	if (user.IsEmpty() || pass.IsEmpty()) {
		AfxMessageBox(L"Vui lòng nhập tên đăng nhập và mật khẩu!");
		return;
	}

	char debugMsg[512];
	sprintf_s(debugMsg, "[MainDlg] Login attempt - User: %ls, Pass: %ls\n", user, pass);
	OutputDebugStringA(debugMsg);

	if (!g_socket.IsConnect()) {
		OutputDebugStringA("[MainDlg] Socket not connected, reconnecting...\n");
		if (!g_socket.Connect("127.0.0.1", 9999)) {
			AfxMessageBox(L"Không kết nối được server");
			OutputDebugStringA("[MainDlg]  Reconnect failed\n");
			return;
		}
		OutputDebugStringA("[MainDlg]  Reconnected successfully\n");
	}

	g_socket.SetCallback(GlobalPacketHandler);

	PacketLogin p{};
	wcscpy_s(p.username, user);
	wcscpy_s(p.password, pass);

	sprintf_s(debugMsg, "[MainDlg] Sending login packet - Size: %d bytes\n", sizeof(p));
	OutputDebugStringA(debugMsg);

	if (g_socket.Send(&p, sizeof(p))) {
		OutputDebugStringA("[MainDlg]  Login packet sent successfully\n");
	}
	else {
		OutputDebugStringA("[MainDlg] Failed to send login packet\n");
		int error = WSAGetLastError();
		sprintf_s(debugMsg, "[MainDlg] Socket error: %d\n", error);
		OutputDebugStringA(debugMsg);
	}
}

LRESULT CServerChatGUIDlg::OnLoginSuccess(WPARAM, LPARAM)
{
	AfxMessageBox(L"Đăng nhập thành công :333");
	this->ShowWindow(SW_HIDE);

	if (g_currentChatScreen) {
		g_currentChatScreen->DestroyWindow();
		delete g_currentChatScreen;
		g_currentChatScreen = nullptr;
	}
	g_currentChatScreen = new ChatScreen();
	g_currentChatScreen->m_username = g_currentUsername;
	g_currentChatScreen->Create(IDD_DIALOG_ChatScreen, this);
	g_currentChatScreen->ShowWindow(SW_SHOW);
	g_currentChatScreen->SetWindowText(L"Chat - " + g_currentChatScreen->m_username);
	g_currentChatScreen->SetForegroundWindow();

	return 0;
}

LRESULT CServerChatGUIDlg::OnLoginFailed(WPARAM wParam, LPARAM lParam)
{
	PacketLoginResult* p = (PacketLoginResult*)wParam;
	if (p && p->msg[0] != L'\0')
	{
		AfxMessageBox(p->msg, MB_ICONERROR | MB_OK);
	}
	else
	{
		AfxMessageBox(L"Đăng nhập thất bại! Sai tên đăng nhập hoặc mật khẩu.", MB_ICONERROR);
	}

	if (p) delete p;  // Giải phóng bộ nhớ
	return 0;
}
void CServerChatGUIDlg::OnBnClickedButtonregister()
{
	RegisterScreen registerScreen;
	registerScreen.DoModal();

	edit_pass.SetWindowText(L"");
	edit_user.SetFocus();

}

