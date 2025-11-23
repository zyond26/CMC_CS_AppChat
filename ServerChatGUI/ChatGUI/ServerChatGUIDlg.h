
// ChatGUIDlg.h : header file
//

#pragma once


// CServerChatGUIDlg dialog
class CServerChatGUIDlg : public CDialogEx
{
// Construction
public:
	CServerChatGUIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATGUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CEdit edit_user;
	CEdit edit_pass;
protected:
	CButton btn_login;
	CButton btn_register;
public:
	afx_msg void OnBnClickedButtonlogin();
	afx_msg void OnBnClickedButtonregister();
	afx_msg LRESULT OnLoginSuccess(WPARAM, LPARAM);
	afx_msg LRESULT OnLoginFailed(WPARAM, LPARAM);
};
