#pragma once

#include "resource.h"
#include <string>

INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class CLoginHandler {

public:

	static CLoginHandler& Instance()
	{
		static CLoginHandler s;
		return s;
	}

	CLoginHandler(CLoginHandler const&);
	CLoginHandler& operator= (CLoginHandler const&);

	void SetHaveAccount(bool value);
	bool HaveAccount();

	bool IsLoggedIn();
	bool LogIn(const std::wstring& userID, const std::wstring& pass);
	bool SignUp(const std::wstring& userID, const std::wstring& pass, const std::wstring& fname, const std::wstring& lname);
	void LogOut();
	const std::string& GetToken();

	/*******/
	std::string loginTrace;
	/*******/

private:

	CLoginHandler() {
		loggedIn = false;
		haveAccount = false;
	};
	~CLoginHandler() {};

	bool IsCredsValid(const std::wstring& userID, const std::wstring& pass);
	bool IsUserDataValid(const std::wstring& userID, const std::wstring& pass, const std::wstring& fname, const std::wstring& lname);

	bool loggedIn;
	bool haveAccount;
	std::string token;
	
};