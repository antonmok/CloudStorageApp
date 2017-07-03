// MainDlg.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <CommCtrl.h>
#include <vector>

#include "MainDlg.h"
#include "SettingsHandler.h"
#include "LoginHandler.h"
#include "Helpers.h"
#include "NetHelper.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING	100
#define TIMER_ID		14

#define UM_CHECK_LOGIN	WM_APP + 1
#define UM_UPLOAD_FILES	WM_APP + 3

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

CSettingsHandler& settingsHandler = CSettingsHandler::Instance();
CLoginHandler& loginHandler = CLoginHandler::Instance();

int g_SelectedBrowser = -1;
bool g_Watching = false;

// Forward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	MainDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc);

	return TRUE;
}

void InitGUIControls(HWND hDlg)
{
	if (settingsHandler.GetSyncPath() != L"") {
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (L"Folder:\n" + settingsHandler.GetSyncPath()).c_str());
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);
	}
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

bool UploadFiles(HWND hDlg) {

	std::vector<std::string> files;
	std::wstring path(settingsHandler.GetSyncPath());
	std::string resData;
	std::string resDataTotal;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	

	path.append(L"\\*");
	hFind = FindFirstFile(path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		MessageBox(hDlg, L"Upload directory access error", L"Error", MB_ICONEXCLAMATION);
		return false;
	}

	// List all the files in the directory
	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// sub dir
		} else {
			std::string fileName;

			ws2s(settingsHandler.GetSyncPath() + L"\\" + ffd.cFileName, fileName);
			files.push_back(fileName);
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		OutputDebugString(L"File access error");
	}

	FindClose(hFind);

	if (files.size()) {

		for (auto fileName : files) {
			UploadFile(std::string(BASE_URL) + METHOD_UPLOAD, fileName, loginHandler.GetToken(), resData);
			resDataTotal.append(resData + "\r\n");
		}
		
	} else {
		MessageBox(hDlg, L"Upload directory is empty", L"Error", MB_ICONEXCLAMATION);
		return false;
	}

	SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_TRACE), ("Upload file responce:\r\n" + resDataTotal).c_str());
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), TRUE);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), L"Upload Folder");

	return true;
}

// Message handler
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	int browserFLag = 0;
	std::wstring path;
	
	switch (message)
	{
	case WM_INITDIALOG:

		settingsHandler.InitSettings();
		InitGUIControls(hDlg);
		SetMainWindowPos(hDlg);

		return (INT_PTR)TRUE;

	case UM_UPLOAD_FILES:
		UploadFiles(hDlg);
		return (INT_PTR)TRUE;

	case UM_CHECK_LOGIN:
		if (!loginHandler.IsLoggedIn()) {
			if (settingsHandler.HaveCredentials()) {
				if (loginHandler.LogIn(settingsHandler.GetUserID(), settingsHandler.GetPass())) {
					SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_TRACE), ("Sign In responce:\r\n" + loginHandler.loginTrace).c_str());
				} else {
					settingsHandler.SetCreds(L"", L"");
					settingsHandler.SaveSettings();
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hDlg, LoginDlgProc);
				}
				
			} else {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hDlg, LoginDlgProc);
			}
		}
		
		return (INT_PTR)TRUE;

	case UM_LOGIN_COMPLETE:
		SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_TRACE), ("Sign In responce:\r\n" + loginHandler.loginTrace).c_str());
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
			loginHandler.LogOut();
			EndDialog(hDlg, LOWORD(wParam));
			PostQuitMessage(0);
			return (INT_PTR)TRUE;

		case IDC_BUTTON_LOGOUT:
			settingsHandler.SetCreds(L"", L"");
			settingsHandler.SaveSettings();
			loginHandler.LogOut();
			PostMessage(hDlg, UM_CHECK_LOGIN, NULL, NULL);

			return (INT_PTR)TRUE;

		case IDC_BUTTON_UPFOLDER:
			
			SelectPathDialog(path);
			settingsHandler.SetSyncPath(path);
			settingsHandler.SaveSettings();
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (L"Folder:\n" + path).c_str());
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);

			return (INT_PTR)TRUE;

		case IDC_BUTTON_UPLOAD:

			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), FALSE);
			SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), L"Uploading...");
			PostMessage(hDlg, UM_UPLOAD_FILES, NULL, NULL);

			return (INT_PTR)TRUE;

		// Settings menu
		/*case IDM_SETTINGS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), hDlg, SettingsDlgProc);
			return (INT_PTR)TRUE;*/

		}
	}
	return (INT_PTR)FALSE;
}