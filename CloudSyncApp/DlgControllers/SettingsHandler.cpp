#include "stdafx.h"
#include "resource.h"
#include "SettingsHandler.h"
#include <string>
#include <Shlobj.h>

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

#define SETTINGS_LOGIN_ID		1
#define SETTINGS_PASS_ID		2

bool SelectPathDialog(std::wstring& path);

struct SettingsData
{
	std::wstring login;
	std::wstring pass;

	// This method lets cereal know which data members to serialize
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(login, pass); // serialize things by passing them to the archive
	}
};

// Message handler
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	int browserFLag = 0;
	std::wstring path;
	CSettingsHandler& instance = CSettingsHandler::Instance();

	switch (message)
	{
	case WM_INITDIALOG:

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hDlg, NULL);
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

bool GetSettingsPath(std::wstring& outPath)
{
	wchar_t* szPFPathPtr = NULL;

	if (S_OK == SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &szPFPathPtr)) {

		outPath.assign(szPFPathPtr);
		outPath.append(L"\\CloudSyncApp");
		CoTaskMemFree(szPFPathPtr);

		if (_waccess(outPath.c_str(), 0) == -1) {
			if (0 == CreateDirectory(outPath.c_str(), NULL)) return false;
		}

		return true;
	}

	return false;
}

void CSettingsHandler::SaveSettings()
{
	std::wstring settPath;

	GetSettingsPath(settPath);

	std::ofstream os(settPath + L"\\settings.bin", std::ios::binary);

	cereal::BinaryOutputArchive oarchive(os);

	SettingsData settData;

	settData.login = userID;
	settData.pass = pass;

	oarchive(settData);
	os.close();
}

void CSettingsHandler::InitSettings()
{
	std::wstring settPath;

	GetSettingsPath(settPath);

	std::ifstream is(settPath + L"\\settings.bin", std::ios::binary);

	if (is.is_open()) {
		cereal::BinaryInputArchive iarchive(is);
		SettingsData settData;

		iarchive(settData);
		userID = settData.login;
		pass = settData.pass;

		is.close();
	}
}

void CSettingsHandler::SetSyncPath(const std::wstring& szPath)
{
	syncPath.assign(szPath);
}

const std::wstring& CSettingsHandler::GetSyncPath()
{
	return syncPath;
}

bool CSettingsHandler::HaveCredentials()
{
	if (userID.size() > 0 && pass.size() > 0) {
		return true;
	}

	return false;
}

void CSettingsHandler::SetCreds(const std::wstring& szLogin, const std::wstring& szPass)
{
	userID = szLogin;
	pass = szPass;
}

const std::wstring& CSettingsHandler::GetUserID()
{
	return userID;
}

const std::wstring& CSettingsHandler::GetPass()
{
	return pass;
}

