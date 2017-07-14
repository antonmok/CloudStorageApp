#pragma once

#include "stdafx.h"
#include <string>

#define APP_VERSION	"1.0.1"
#define TEMP_INSTALLER_NAME L"CloudSyncAppInstaller.exe"

INT_PTR CALLBACK UpdateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class CUpdateHandler {

public:

	static CUpdateHandler& Instance()
	{
		static CUpdateHandler s;
		return s;
	}

	CUpdateHandler(CUpdateHandler const&);
	CUpdateHandler& operator= (CUpdateHandler const&);

	bool ForceFlag();
	void CheckUpdates(HWND hMainWnd);
	void InitGUI(HWND hDlg);
	void PerformUpdate(HWND hMainWnd);

private:

	CUpdateHandler()
	{
		forceFlag = false;
		currentVersion.assign(APP_VERSION);
	};

	~CUpdateHandler() {};

	bool VersionCmp(const std::string& versionFromServer);

	bool forceFlag;
	std::string installerURL;
	std::string currentVersion;
};
