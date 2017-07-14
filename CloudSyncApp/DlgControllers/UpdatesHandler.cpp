#define NOMINMAX
#include "UpdatesHandler.h"
#include "NetHelper.h"
#include "Helpers.h"
#include <thread>
#include <CommCtrl.h>
#include "resource.h"
#include "cereal/external/rapidjson/document.h"
#include "cereal/external/rapidjson/reader.h"

bool CUpdateHandler::VersionCmp(const std::string& versionFromServer)
{
	std::vector<std::string> versComponentsServer(split(versionFromServer, '.'));
	std::vector<std::string> versComponentsCur(split(currentVersion, '.'));

	int idx = 0;
	for (auto verComp : versComponentsServer) {
		if (std::stoi(verComp) > std::stoi(versComponentsCur[idx])) {
			return true;
		}

		if (std::stoi(verComp) < std::stoi(versComponentsCur[idx])) {
			return false;
		}

		idx++;
	}

	return false;
}

void CUpdateHandler::InitGUI(HWND hDlg)
{
	if (ForceFlag()) {
		SetWindowText(GetDlgItem(hDlg, IDCANCEL), L"Exit");
	} else {
		SetWindowText(GetDlgItem(hDlg, IDCANCEL), L"Skip");
	}
}

// Message handler
INT_PTR CALLBACK UpdateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	CUpdateHandler& updateHandler = CUpdateHandler::Instance();

	switch (message) {

	case UM_SET_PROGRESS:
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_UPDATE), PBM_SETPOS, wParam, 0);
		return (INT_PTR)TRUE;

	case WM_INITDIALOG:
		updateHandler.InitGUI(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDCANCEL:

			EndDialog(hDlg, LOWORD(wParam));
			if (updateHandler.ForceFlag() == true) {
				PostQuitMessage(0);
			}
			return (INT_PTR)TRUE;

		case IDOK:
			// start new thread to perform update
			std::thread watchDirThread(&CUpdateHandler::PerformUpdate, std::ref(updateHandler), hDlg);
			watchDirThread.detach();
			return (INT_PTR)TRUE;
		}
	}

	return (INT_PTR)FALSE;
}

void CUpdateHandler::PerformUpdate(HWND hMainWnd)
{
	UINT fSize = GetFileSizeHTTP(installerURL);

	ShowWindow(GetDlgItem(hMainWnd, IDC_PROGRESS_UPDATE), 1);
	SendMessage(GetDlgItem(hMainWnd, IDC_PROGRESS_UPDATE), PBM_SETRANGE32, 0, fSize + 1);
	SendMessage(GetDlgItem(hMainWnd, IDC_PROGRESS_UPDATE), PBM_SETPOS, 0, 0);

	// get temp dir
	wchar_t szTmpPath[MAX_PATH] = { 0 };
	bool needToWriteBOM = false;

	GetTempPath(MAX_PATH, szTmpPath);
	wcscat_s(szTmpPath, TEMP_INSTALLER_NAME);

	if (GetFileHTTP(installerURL, szTmpPath, hMainWnd)) {

		if (RunProcess(szTmpPath, std::wstring(TEMP_INSTALLER_NAME) + L" -update")) {
			// stop itself
			PostMessage(hMainWnd, WM_QUIT, 0, 0);
		} else {
			MessageBox(hMainWnd, L"Could not start updates installer.", L"Error", MB_ICONEXCLAMATION);
		}

	} else {
		MessageBox(hMainWnd, L"Some error occurred during downloading updates.", L"Error", MB_ICONEXCLAMATION);
	}
}

bool CUpdateHandler::ForceFlag()
{
	return forceFlag;
}

void CUpdateHandler::CheckUpdates(HWND hMainWnd)
{
	std::string responce;
	std::string fields;
	std::string version;

	if (PostHttps(std::string(BASE_URL) + METHOD_VERSION, fields, responce)) {
		// parse JSON
		rapidjson::Document doc;
		if (doc.Parse(responce.c_str()).HasParseError()) {
			return;
		}

		if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
			if (doc["success"].GetInt() == 1) {

				rapidjson::Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
				if (itrData != doc.MemberEnd()) {

					rapidjson::Value::ConstMemberIterator itrVal = itrData->value.FindMember(FIELD_VERSION);
					if (itrVal != itrData->value.MemberEnd() && itrVal->value.IsString()) {
						version = itrVal->value.GetString();
					}

					itrVal = itrData->value.FindMember(FIELD_INST_URL);
					if (itrVal != itrData->value.MemberEnd() && itrVal->value.IsString()) {
						installerURL = itrVal->value.GetString();
					}

					itrVal = itrData->value.FindMember(FIELD_FORCE_FLAG);
					if (itrVal != itrData->value.MemberEnd() && itrVal->value.IsString()) {
						if (std::stoi(itrVal->value.GetString())) {
							forceFlag = true;
						}
					}

					if (VersionCmp(version)) {
						PostMessage(hMainWnd, UM_HAVE_UPDATES, 0, 0);
					}
				}
			}
		}
	}
}










