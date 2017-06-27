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

	bool IsLoggedIn();
	bool LogIn(const std::wstring& userID, const std::wstring& pass);
	void LogOut();
	const std::string& GetSecid();

private:

	CLoginHandler() {
		loggedIn = false;
	};
	~CLoginHandler() {};

	bool IsCredsValid(const std::wstring& userID, const std::wstring& pass);

	bool loggedIn;
	std::string secid;
};