// MainDlg.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <CommCtrl.h>

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

	case UM_CHECK_LOGIN:
		if (!loginHandler.IsLoggedIn()) {
			if (settingsHandler.HaveCredentials()) {
				loginHandler.LogIn(settingsHandler.GetUserID(), settingsHandler.GetPass());
			} else {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hDlg, LoginDlgProc);
			}
		}
		
		return (INT_PTR)TRUE;

	case WM_ACTIVATE:
		if (wParam != WA_INACTIVE) {
			PostMessage(hDlg, UM_CHECK_LOGIN, NULL, NULL);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case IDCANCEL:
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
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (L"Folder:\n" + path).c_str());
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);

			return (INT_PTR)TRUE;

		case IDC_BUTTON_UPLOAD:

			/*if (settingsHandler.GetSyncPath() == L"") {

			}*/
			return (INT_PTR)TRUE;

		// Settings menu
		/*case IDM_SETTINGS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETTINGS), hDlg, SettingsDlgProc);
			return (INT_PTR)TRUE;*/

		}
	}
	return (INT_PTR)FALSE;
}