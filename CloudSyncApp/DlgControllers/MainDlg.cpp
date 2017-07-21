// MainDlg.cpp : Defines the entry point for the application.
//
#define NOMINMAX

#include "stdafx.h"
#include "resource.h"
#include <CommCtrl.h>
#include <vector>
#include <thread>

#include "MainDlg.h"
#include "SettingsHandler.h"
#include "LoginHandler.h"
#include "UpdatesHandler.h"
#include "i18n.h"
#include "Helpers.h"
#include "EncodingHelper.h"
#include "NetHelper.h"
#include "FolderStruct.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING	100
#define TIMER_ID		14

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

CSettingsHandler& settingsHandler = CSettingsHandler::Instance();
CLoginHandler& loginHandler = CLoginHandler::Instance();
CDirectoryTree& dirTreeInstance = CDirectoryTree::Instance();
CUpdateHandler& updateHandler = CUpdateHandler::Instance();
Ci18n& i18nHelper = Ci18n::Instance();

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK	MainDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc);

	return TRUE;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_WND_CLASS_NAME, szWindowClass, MAX_LOADSTRING);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDS_WND_CLASS_NAME));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

void ShowLocalTree(HWND hDlg)
{
	dirTreeInstance.InitLocalTree(settingsHandler.GetSyncPath());
	dirTreeInstance.InitTreeCtrl(GetDlgItem(hDlg, IDC_TREE), false);
}

void ShowRemoteTree(HWND hDlg)
{
	std::string fields("token=");
	std::string response;

	fields.append(loginHandler.GetToken());

	if (PostHttp(std::string(BASE_URL) + METHOD_GET_TREE, fields, response)) {
		dirTreeInstance.InitRemoteTree(response);
		dirTreeInstance.InitTreeCtrl(GetDlgItem(hDlg, IDC_TREE_REMOTE), true);
	}
}

void InitGUIControls(HWND hDlg)
{
	InitCommonControls();

	/*TODO: handle image list*/
	HIMAGELIST hImageList = ImageList_LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_PNG_DIR), 16, 0, NULL);
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PNG_FILE)), NULL);
	TreeView_SetImageList(GetDlgItem(hDlg, IDC_TREE), hImageList, TVSIL_NORMAL);

	if (settingsHandler.GetSyncPath() != L"") {
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (i18nHelper.Geti18nItem("folder_lb") + L"\n" + settingsHandler.GetSyncPath()).c_str());
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);

		// start new thread to listen for files changes
		dirTreeInstance.WatchDirectory(settingsHandler.GetSyncPath(), hDlg);
		ShowLocalTree(hDlg);
	}

	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPFOLDER), i18nHelper.Geti18nItem("set_folder_btn").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), i18nHelper.Geti18nItem("upload_btn").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_LOGOUT), i18nHelper.Geti18nItem("log_out_btn").c_str());
	SetWindowText(GetDlgItem(hDlg, IDCANCEL), i18nHelper.Geti18nItem("exit_btn").c_str());
	SetWindowText(hDlg, i18nHelper.Geti18nItem("main_title").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_LOCAL), i18nHelper.Geti18nItem("local_lb").c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_REMOTE), i18nHelper.Geti18nItem("remote_lb").c_str());

}

void SetUIToUploadState(HWND hDlg)
{
	ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_SHOW);
	SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETMARQUEE, 1, 20);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), FALSE);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), i18nHelper.Geti18nItem("upload_btn_dis").c_str());
}

void SetMainWindowPos(HWND hWnd)
{
	RECT wndRect;
	HDC screen = GetDC(0);
	int width = GetDeviceCaps(screen, HORZRES);
	int height = GetDeviceCaps(screen, VERTRES);

	GetWindowRect(hWnd, &wndRect);

	int wndWidth = wndRect.right - wndRect.left;
	int wndHeight = wndRect.bottom - wndRect.top;
	
	SetWindowPos(hWnd, NULL, width / 2 - wndWidth / 2, height / 2 - wndHeight / 2, 0, 0, SWP_NOSIZE);
}

// Events handlers
INT_PTR OnFilesChanged(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	int action = wParam;
	std::shared_ptr<std::wstring> fileName(reinterpret_cast<std::wstring*>(lParam));

	LogFileAction(GetDlgItem(hDlg, IDC_EDIT_TRACE), action, *fileName);
	if (!IsFilteredUIAction(action, *fileName)) {
		ShowLocalTree(hDlg);
	}

	if (!IsFilteredUploadAction(action, *fileName)) {
		SetUIToUploadState(hDlg);
		PostMessage(hDlg, UM_UPLOAD_FILES, 0, 0);
	}

	return (INT_PTR)TRUE;
}

INT_PTR OnCheckLogin(HWND hDlg)
{
	if (!loginHandler.IsLoggedIn()) {
		if (settingsHandler.HaveCredentials()) {
			if (loginHandler.LogIn(settingsHandler.GetUserID(), settingsHandler.GetPass())) {
				PostMessage(hDlg, UM_LOGIN_COMPLETE, NULL, NULL);
			} else {
				settingsHandler.SetCreds(L"", L"");
				settingsHandler.SaveSettings();
				if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hDlg, LoginDlgProc)) {
					PostMessage(hDlg, UM_LOGIN_COMPLETE, NULL, NULL);
				}
			}

		} else {
			if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hDlg, LoginDlgProc)) {
				PostMessage(hDlg, UM_LOGIN_COMPLETE, NULL, NULL);
			}
		}
	}

	return (INT_PTR)TRUE;
}

INT_PTR OnLoginComplete(HWND hDlg)
{
	std::wstring wideStr;
	s2ws(loginHandler.loginTrace, wideStr);
	EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), L"Sign In responce:\r\n" + wideStr + L"\r\n");

	ShowRemoteTree(hDlg);

	InitGUIControls(hDlg);

	// start new thread to check updates
	std::thread checkUpdatesThread(&CUpdateHandler::CheckUpdates, std::ref(updateHandler), hDlg);
	checkUpdatesThread.detach();

	return (INT_PTR)TRUE;
}

INT_PTR OnExitClicked(HWND hDlg, WPARAM wParam)
{
	loginHandler.LogOut();
	EndDialog(hDlg, LOWORD(wParam));
	PostQuitMessage(0);
	return (INT_PTR)TRUE;
}

INT_PTR OnLogOutClicked(HWND hDlg)
{
	settingsHandler.SetCreds(L"", L"");
	settingsHandler.SaveSettings();
	loginHandler.LogOut();
	PostMessage(hDlg, UM_CHECK_LOGIN, NULL, NULL);

	return (INT_PTR)TRUE;
}

INT_PTR OnSetFolderClicked(HWND hDlg)
{
	std::wstring path;

	if (SelectPathDialog(path)) {
		settingsHandler.SetSyncPath(path);
		settingsHandler.SaveSettings();

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (i18nHelper.Geti18nItem("folder_lb") + L"\n" + path).c_str());
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);
		// start new thread to listen for files changes
		dirTreeInstance.WatchDirectory(settingsHandler.GetSyncPath(), hDlg);
		ShowLocalTree(hDlg);
	}

	return (INT_PTR)TRUE;
}

INT_PTR OnUploadClicked(HWND hDlg)
{
	SetUIToUploadState(hDlg);
	PostMessage(hDlg, UM_UPLOAD_FILES, 0, 0);

	return (INT_PTR)TRUE;
}

INT_PTR OnUploadComplete(HWND hDlg)
{
	EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), L"\r\n");
	ShowRemoteTree(hDlg);

	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), TRUE);
	SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETMARQUEE, 0, 0);
	ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_HIDE);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), i18nHelper.Geti18nItem("upload_btn").c_str());

	return (INT_PTR)TRUE;
}

// Message handler
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	int browserFLag = 0;
	
	switch (message)
	{
	case WM_INITDIALOG:

		settingsHandler.InitSettings();
		i18nHelper.GetTranslationsFromServer();
		SetMainWindowPos(hDlg);
		return (INT_PTR)TRUE;

	case UM_FILES_CHANGED:
		return OnFilesChanged(hDlg, wParam, lParam);

	case UM_UPLOAD_FILES:
		dirTreeInstance.UploadFiles(hDlg);
		return (INT_PTR)TRUE;

	case UM_UPLOAD_COMPLETE:
		return OnUploadComplete(hDlg);

	case UM_CHECK_LOGIN:
		return OnCheckLogin(hDlg);

	case UM_LOGIN_COMPLETE:
		return OnLoginComplete(hDlg);

	case UM_HAVE_UPDATES:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_UPDATE), hDlg, UpdateDlgProc);
		return (INT_PTR)TRUE;

	case WM_ACTIVATE:
		if (wParam != WA_INACTIVE) {
			PostMessage(hDlg, UM_CHECK_LOGIN, NULL, NULL);
		}
		return (INT_PTR)FALSE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			return OnExitClicked(hDlg, wParam);

		case IDC_BUTTON_LOGOUT:
			return OnLogOutClicked(hDlg);

		case IDC_BUTTON_UPFOLDER:
			return OnSetFolderClicked(hDlg);

		case IDC_BUTTON_UPLOAD:
			return OnUploadClicked(hDlg);
		}
	}
	return (INT_PTR)FALSE;
}