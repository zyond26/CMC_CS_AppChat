// LoginScreen.cpp : implementation file
//

#include "pch.h"
#include "ChatGUI.h"
#include "afxdialogex.h"
#include "LoginScreen.h"


// LoginScreen dialog

IMPLEMENT_DYNAMIC(LoginScreen, CDialogEx)

LoginScreen::LoginScreen(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_Login, pParent)
{

}

LoginScreen::~LoginScreen()
{
}

void LoginScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LoginScreen, CDialogEx)
END_MESSAGE_MAP()


// LoginScreen message handlers
