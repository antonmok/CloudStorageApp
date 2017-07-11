#define NOMINMAX
#include "stdafx.h"
#include <CommCtrl.h>
#include "LoginHandler.h"
#include "SettingsHandler.h"
#include "NetHelper.h"
#include "Helpers.h"
#include "resource.h"
#include "cereal/external/rapidjson/document.h"
#include "cereal/external/rapidjson/reader.h"

void SwitchGUIStateToSignin(HWND hDlg)
{
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_FNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_LNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_FNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_LNAME), 0);

	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_HAVEACCOUNT), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_SYSLINK_SIGNIN), 0);

	SetWindowText(GetDlgItem(hDlg, IDOK), L"Sign in");

	RECT rc;
	int yShift = 85;	// pixels
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_STATIC_EMAIL));
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_EMAIL), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_EDIT_LOGIN));
	SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_LOGIN), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_STATIC_PASS));
	SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_PASS), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_EDIT_PASS));
	SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_PASS), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDOK));
	SetWindowPos(GetDlgItem(hDlg, IDOK), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDCANCEL));
	SetWindowPos(GetDlgItem(hDlg, IDCANCEL), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_CHECK_AGREE));
	SetWindowPos(GetDlgItem(hDlg, IDC_CHECK_AGREE), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);
	rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_SYSLINK_TERMS));
	SetWindowPos(GetDlgItem(hDlg, IDC_SYSLINK_TERMS), NULL, rc.left, rc.top - ScaleDPI(yShift), 0, 0, SWP_NOSIZE);

	rc = GetCtrlLocalCoordinates(hDlg);
	SetWindowPos(hDlg, NULL, 0, 0, rc.right - rc.left, (rc.bottom - rc.top) - ScaleDPI(yShift + 13), SWP_NOMOVE);
}

void HandleCredentials(HWND hDlg, WPARAM wParam, CLoginHandler& loginHandler)
{
	wchar_t bufFName[256] = {};
	wchar_t bufLName[256] = {};
	wchar_t bufLogin[256] = {};
	wchar_t bufPass[256] = {};

	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LOGIN), bufLogin, 256);
	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PASS), bufPass, 256);

	if (loginHandler.HaveAccount()) {
		if (loginHandler.LogIn(bufLogin, bufPass)) {
			CSettingsHandler::Instance().SetCreds(bufLogin, bufPass);
			CSettingsHandler::Instance().SaveSettings();
			EndDialog(hDlg, LOWORD(wParam));
		} else {
			MessageBox(hDlg, L"Failed to log in", L"App", MB_ICONEXCLAMATION);
		}
	} else {
		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FNAME), bufFName, 256);
		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LNAME), bufLName, 256);

		if (loginHandler.SignUp(bufLogin, bufPass, bufFName, bufLName)) {
			SwitchGUIStateToSignin(hDlg);
			loginHandler.SetHaveAccount(true);
			MessageBox(hDlg, L"You have been registered. Now you can sign in", L"App", MB_OK);
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT_LOGIN), L"");
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PASS), L"");
			CheckDlgButton(hDlg, IDC_CHECK_AGREE, false);
			EnableWindow(GetDlgItem(hDlg, IDOK), false);
		} else {
			MessageBox(hDlg, L"Failed to sign up", L"App", MB_ICONEXCLAMATION);
		}
	}
}

// Message handler
INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	CLoginHandler& loginHandler = CLoginHandler::Instance();

	switch (message)
	{
	case WM_INITDIALOG:

		loginHandler.hParent = GetParent(hDlg);

		if (loginHandler.HaveAccount()) {
			SwitchGUIStateToSignin(hDlg);
		}
		return (INT_PTR)TRUE;

	case WM_NOTIFY:

		if (((LPNMHDR)lParam)->code == NM_CLICK) {
			switch (((LPNMHDR)lParam)->idFrom) {
				case IDC_SYSLINK_TERMS:
					MessageBox(hDlg, L"Terms of use here", L"App", MB_OK);
					break;

				case IDC_SYSLINK_SIGNIN:
					loginHandler.SetHaveAccount(true);
					SwitchGUIStateToSignin(hDlg);

					break;
			}
		}

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

			case IDCANCEL:

				EndDialog(hDlg, LOWORD(wParam));
				PostQuitMessage(0);
				return (INT_PTR)TRUE;

			case IDOK:

				HandleCredentials(hDlg, wParam, loginHandler);
				return (INT_PTR)TRUE;

			case IDC_CHECK_AGREE:

				if (IsDlgButtonChecked(hDlg, IDC_CHECK_AGREE)) {
					EnableWindow(GetDlgItem(hDlg, IDOK), true);
				} else {
					EnableWindow(GetDlgItem(hDlg, IDOK), false);
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

bool CLoginHandler::HaveAccount()
{
	return haveAccount;
}

void CLoginHandler::SetHaveAccount(bool value)
{
	haveAccount = value;
}

bool CLoginHandler::LogIn(const std::wstring& userID, const std::wstring& pass)
{
	if (!IsCredsValid(userID, pass)) {
		return false;
	}

	std::string narrowUserID;
	std::string narrowPass;
	ws2s(userID, narrowUserID);
	ws2s(pass, narrowPass);

	std::string postForm(std::string(PARAM_EMAIL) + "=" + narrowUserID + "&" + PARAM_PASS + "=" + narrowPass);
	std::string postResData;

	if (PostHttps(std::string(BASE_URL) + std::string(METHOD_LOGIN), postForm, postResData)) {

		// parse JSON
		rapidjson::Document doc;
		if (doc.Parse(postResData.c_str()).HasParseError()) {
			return false;
		}

		if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
			if (doc["success"].GetInt() == 1) {

				rapidjson::Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
				if (itrData != doc.MemberEnd()) {

					rapidjson::Value::ConstMemberIterator itr = itrData->value.FindMember(FIELD_TOKEN);
					if (itrData != itrData->value.MemberEnd()) {
						if (itr->value.IsString()) {
							loggedIn = true;
							token.assign(itr->value.GetString());
							

							/**/
							loginTrace = postResData;
							PostMessage(hParent, UM_LOGIN_COMPLETE, 0, 0);
							/**/

							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool CLoginHandler::SignUp(const std::wstring& userID, const std::wstring& pass, const std::wstring& fname, const std::wstring& lname)
{
	if (!IsUserDataValid(userID, pass, fname, lname)) {
		return false;
	}

	std::string narrowUserID;
	std::string narrowPass;
	std::string narrowFName;
	std::string narrowLName;
	ws2s(userID, narrowUserID);
	ws2s(pass, narrowPass);
	ws2s(fname, narrowFName);
	ws2s(lname, narrowLName);

	std::string postForm(std::string(PARAM_EMAIL) + "=" + narrowUserID + "&" + PARAM_PASS + "=" + narrowPass + "&" + PARAM_FNAME + "=" + narrowFName + "&" + PARAM_LNAME + "=" + narrowLName);
	std::string postResData;

	if (PostHttps(std::string(BASE_URL) + std::string(METHOD_SIGNUP), postForm, postResData)) {

		// parse JSON
		rapidjson::Document doc;
		if (doc.Parse(postResData.c_str()).HasParseError()) {
			return false;
		}

		if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
			if (doc["success"].GetInt() == 1) {
				return true;
			}
		}
	}

	return false;
}

const std::string& CLoginHandler::GetToken()
{
	return token;
}

void CLoginHandler::LogOut()
{
	loggedIn = false;
	haveAccount = true;

	std::string postForm(std::string(PARAM_TOKEN) + "=" + token);
	std::string postResData;

	PostHttps(std::string(BASE_URL) + std::string(METHOD_LOGOUT), postForm, postResData);
}

bool CLoginHandler::IsCredsValid(const std::wstring& userID, const std::wstring& pass)
{
	if (userID.size() > 0 && pass.size() > 0) {
		return true;
	} else {
		return false;
	}
}

bool CLoginHandler::IsUserDataValid(const std::wstring& userID, const std::wstring& pass, const std::wstring& fname, const std::wstring& lname)
{
	if (userID.size() > 0 && pass.size() > 0 && fname.size() > 0 && lname.size() > 0) {
		return true;
	}
	else {
		return false;
	}
}