#define NOMINMAX
#include "stdafx.h"
#include <CommCtrl.h>
#include "LoginHandler.h"
#include "SettingsHandler.h"
#include "i18n.h"
#include "NetHelper.h"
#include "EncodingHelper.h"
#include "Helpers.h"
#include "resource.h"
#include "cereal/external/rapidjson/document.h"
#include "cereal/external/rapidjson/reader.h"

void InitGuiElements(HWND hDlg)
{
	Ci18n& i18nHelper = Ci18n::Instance();

	SetWindowText(GetDlgItem(hDlg, IDCANCEL), i18nHelper.Geti18nItem("exit_btn").c_str());
	SetWindowText(GetDlgItem(hDlg, IDOK), i18nHelper.Geti18nItem("sign_up_btn").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FNAME), i18nHelper.Geti18nItem("first_name_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_LNAME), i18nHelper.Geti18nItem("last_name_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_EMAIL), i18nHelper.Geti18nItem("email_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_PASS), i18nHelper.Geti18nItem("password_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_HAVEACCOUNT), i18nHelper.Geti18nItem("already_have_acc_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_SYSLINK_SIGNIN), (L"<a>" + i18nHelper.Geti18nItem("sign_in_link") + L"</a>").c_str());

	// TODO split terms_check to terms_check and terms_link
	if (i18nHelper.GetLangCode() == Ci18n::LC_SPANISH) {
		SetWindowText(GetDlgItem(hDlg, IDC_CHECK_AGREE), i18nHelper.Geti18nItem("terms_check").substr(0, 24).c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_SYSLINK_TERMS), (L"<a>" + i18nHelper.Geti18nItem("terms_check").substr(25, 15) + L"</a>").c_str());

		RECT rc;
		rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_CHECK_AGREE));
		SetWindowPos(GetDlgItem(hDlg, IDC_CHECK_AGREE), NULL, 0, 0, rc.right - rc.left + ScaleDPI(35), rc.bottom - rc.top, SWP_NOMOVE);

		rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_SYSLINK_TERMS));
		SetWindowPos(GetDlgItem(hDlg, IDC_SYSLINK_TERMS), NULL, rc.left + ScaleDPI(37), rc.top, 0, 0, SWP_NOSIZE);

		rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_STATIC_HAVEACCOUNT));
		SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_HAVEACCOUNT), NULL, 0, 0, rc.right - rc.left - ScaleDPI(35), rc.bottom - rc.top, SWP_NOMOVE);

		rc = GetCtrlLocalCoordinates(GetDlgItem(hDlg, IDC_SYSLINK_SIGNIN));
		SetWindowPos(GetDlgItem(hDlg, IDC_SYSLINK_SIGNIN), NULL, rc.left - ScaleDPI(25), rc.top, 0, 0, SWP_NOSIZE);
		
	} else {
		SetWindowText(GetDlgItem(hDlg, IDC_CHECK_AGREE), i18nHelper.Geti18nItem("terms_check").substr(0, 14).c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_SYSLINK_TERMS), (L"<a>" + i18nHelper.Geti18nItem("terms_check").substr(15, 12) + L"</a>").c_str());
	}

	SetWindowText(hDlg, i18nHelper.Geti18nItem("login_title").c_str());
}

void SwitchGUIStateToSignin(HWND hDlg)
{
	Ci18n& i18nHelper = Ci18n::Instance();

	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_FNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_LNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_FNAME), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_LNAME), 0);

	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_HAVEACCOUNT), 0);
	ShowWindow(GetDlgItem(hDlg, IDC_SYSLINK_SIGNIN), 0);

	SetWindowText(GetDlgItem(hDlg, IDOK), i18nHelper.Geti18nItem("sign_in_btn").c_str());

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

void CLoginHandler::HandleCredentials(HWND hDlg, WPARAM wParam)
{
	wchar_t bufFName[256] = {};
	wchar_t bufLName[256] = {};
	wchar_t bufLogin[256] = {};
	wchar_t bufPass[256] = {};

	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LOGIN), bufLogin, 256);
	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PASS), bufPass, 256);

	if (HaveAccount()) {
		if (LogIn(bufLogin, bufPass)) {
			CSettingsHandler::Instance().SetCreds(bufLogin, bufPass);
			CSettingsHandler::Instance().SaveSettings();
			EndDialog(hDlg, true);
		} else {
			MessageBox(hDlg, L"Failed to log in", L"App", MB_ICONEXCLAMATION);
		}
	} else {
		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_FNAME), bufFName, 256);
		GetWindowText(GetDlgItem(hDlg, IDC_EDIT_LNAME), bufLName, 256);

		if (SignUp(bufLogin, bufPass, bufFName, bufLName)) {
			SwitchGUIStateToSignin(hDlg);
			SetHaveAccount(true);
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
	CLoginHandler& loginHandler = CLoginHandler::Instance();

	switch (message)
	{
	case WM_INITDIALOG:
		
		InitGuiElements(hDlg);

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

				EndDialog(hDlg, false);
				PostQuitMessage(0);
				return (INT_PTR)TRUE;

			case IDOK:

				loginHandler.HandleCredentials(hDlg, wParam);
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

using namespace rapidjson;

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

	if (PostHttp(std::string(BASE_URL) + std::string(METHOD_LOGIN), postForm, postResData)) {

		// parse JSON
		Document doc;
		if (doc.Parse(postResData.c_str()).HasParseError()) {
			return false;
		}

		if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
			if (doc["success"].GetInt() == 1) {

				Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
				if (itrData != doc.MemberEnd()) {

					Value::ConstMemberIterator itrLangCode = itrData->value.FindMember(FIELD_LANG);
					if (itrLangCode != itrData->value.MemberEnd() && itrLangCode->value.IsString()) {
						Ci18n& i18nHelper = Ci18n::Instance();
						std::wstring langCode;
						UTF8ToWs(itrLangCode->value.GetString(), langCode);
						i18nHelper.SetLangCode(langCode);
					}

					Value::ConstMemberIterator itrToken = itrData->value.FindMember(FIELD_TOKEN);
					if (itrToken != itrData->value.MemberEnd() && itrToken->value.IsString()) {

						loggedIn = true;
						token.assign(itrToken->value.GetString());
						/* TODO: remove*/
						loginTrace = postResData;

						return true;
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

	if (PostHttp(std::string(BASE_URL) + std::string(METHOD_SIGNUP), postForm, postResData)) {

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

	PostHttp(std::string(BASE_URL) + std::string(METHOD_LOGOUT), postForm, postResData);
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