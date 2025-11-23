#pragma once
#include "afxdialogex.h"

class RegisterScreen : public CDialogEx
{
    DECLARE_DYNAMIC(RegisterScreen)

public:
    virtual BOOL OnInitDialog();

    RegisterScreen(CWnd* pParent = nullptr);
    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_Register };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

    CEdit edit_user;
    CEdit edit_pass;
    CEdit edit_email;

public:
    afx_msg void OnBnClickedButtonregister();
    afx_msg void OnEnChangeEdituser();
    afx_msg void OnEnChangeEditemail();
    afx_msg void OnEnChangeEditpass();
    afx_msg void OnBnClickedButtonlogin();

    afx_msg LRESULT OnRegisterSuccess(WPARAM, LPARAM);
    afx_msg LRESULT OnRegisterFailure(WPARAM, LPARAM);
};