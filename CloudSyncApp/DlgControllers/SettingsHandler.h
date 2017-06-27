#pragma once

#include <string>

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class CSettingsHandler {

public:

	static CSettingsHandler& Instance()
	{
		static CSettingsHandler s;
		return s;
	}

	CSettingsHandler(CSettingsHandler const&);
	CSettingsHandler& operator= (CSettingsHandler const&);

	void SaveSettings();
	void InitSettings();

	void SetCreds(const std::wstring& szLogin, const std::wstring& szPass);
	bool HaveCredentials();
	const std::wstring& GetUserID();
	const std::wstring& GetPass();

	void SetSyncPath(const std::wstring& szPath);
	const std::wstring& GetSyncPath();

private:
	
	CSettingsHandler() {};
	~CSettingsHandler() {};

	std::wstring userID;
	std::wstring pass;
	std::wstring syncPath;

};