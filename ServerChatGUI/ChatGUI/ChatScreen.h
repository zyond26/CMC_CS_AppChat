#pragma once
#include "afxdialogex.h"
#include "map"
#include "vector"


#define WM_RECV_CHAT (WM_USER + 100)

// ChatScreen dialog

class ChatScreen : public CDialogEx
{
	DECLARE_DYNAMIC(ChatScreen)


public:
	ChatScreen(CWnd* pParent = nullptr);   // standard constructor
	virtual ~ChatScreen();

	void SetUsername(const CString& username) {
		m_username = username;
	}

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ChatScreen };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CListCtrl list_user;
	CListCtrl list_mess;
	CEdit edit_text;
	CButton btn_send;
	CButton btn_logout;
public:

	CString m_username;
	CString selected;

	CString m_currentChatWith;  // Lưu đang chat với ai: L"ALL" hoặc tên user

	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemchangedListmess(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEdittext();
	afx_msg void OnBnClickedButtonsend();

	afx_msg LRESULT OnUpdateUserList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateUserStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageHistoryReceived(WPARAM wParam, LPARAM lParam);


	LRESULT OnMessageReceived(WPARAM wParam, LPARAM lParam);
	void AddMessage(const CString& message);
	//void SaveMessagesToFile();

public:
	int m_myUserId = 0;                    
	int m_currentTargetId = 0;
	std::map<CString, int> m_userIdMap;    


public:
	afx_msg void OnBnClickedButtonlogout();
protected:
	CButton btn_save;
public:
	afx_msg void OnBnClickedButtonsave();
};
