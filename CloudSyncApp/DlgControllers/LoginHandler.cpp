#include "stdafx.h"
#include "LoginHandler.h"
#include "SettingsHandler.h"
#include "NetHelper.h"
#include "Helpers.h"
#include "resource.h"

//#define LOGIN_URL		"https://server.com/login.php"

// Message handler
INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	CLoginHandler& loginHandler = CLoginHandler::Instance();

	switch (message)
	{
	case WM_INITDIALOG:

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			PostQuitMessage(0);
			return (INT_PTR)TRUE;

		case IDOK:
			wchar_t bufLogin[256] = {};
			wchar_t bufPass[256] = {};

			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LOGIN), bufLogin, 256);
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PASS), bufPass, 256);

			if (loginHandler.LogIn(bufLogin, bufPass)) {
				CSettingsHandler::Instance().SetCreds(bufLogin, bufPass);
				CSettingsHandler::Instance().SaveSettings();
				EndDialog(hDlg, LOWORD(wParam));
			} else {
				MessageBox(hDlg, L"Failed to log in", L"App", MB_ICONEXCLAMATION);
			}
			
			return (INT_PTR)TRUE;
		}

	}
	return (INT_PTR)FALSE;
}

bool CLoginHandler::IsLoggedIn()
{
	return loggedIn;
}

bool CLoginHandler::LogIn(const std::wstring& userID, const std::wstring& pass)
{
	if (!IsCredsValid(userID, pass)) {
		return false;
	}

	return true;
	/*
	std::string narrowUserID;
	std::string narrowPass;
	ws2s(userID, narrowUserID);
	ws2s(pass, narrowPass);

	std::string postForm(std::string(LOGIN_USERID) + "=" + narrowUserID + "&" + LOGIN_PASS + "=" + narrowPass);
	std::string postResData;

	if (PostHttps(LOGIN_URL, postForm, postResData)) {

		// parse XML
		pugi::xml_document xmlDoc;
		pugi::xml_parse_result xmlResult = xmlDoc.load(postResData.c_str());


		loggedIn = true;

		return true;
	}

	return false;*/
}

const std::string& CLoginHandler::GetSecid()
{
	return secid;
}

void CLoginHandler::LogOut()
{
	loggedIn = false;
}

bool CLoginHandler::IsCredsValid(const std::wstring& userID, const std::wstring& pass)
{
	if (userID.size() > 0 && pass.size() > 0) {
		return true;
	} else {
		return false;
	}
}


