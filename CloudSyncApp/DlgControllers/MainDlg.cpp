// MainDlg.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <CommCtrl.h>
#include <vector>
#include <thread>

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

#define UM_CHECK_LOGIN		WM_APP + 1
#define UM_UPLOAD_FILES		WM_APP + 3
#define UM_FILES_CHANGED	WM_APP + 4

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

CSettingsHandler& settingsHandler = CSettingsHandler::Instance();
CLoginHandler& loginHandler = CLoginHandler::Instance();

int g_SelectedBrowser = -1;
bool g_Watching = false;
bool g_FilesChangedTriggered = false;

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

/*bool WatchDirectory(const std::wstring& szDir, HWND hMainWnd)
{
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles;
	TCHAR lpDrive[4];
	TCHAR lpFile[_MAX_FNAME];
	TCHAR lpExt[_MAX_EXT];

	_tsplitpath_s(szDir.c_str(), lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

	lpDrive[2] = (TCHAR)'\\';
	lpDrive[3] = (TCHAR)'\0';

	// Watch the directory for file creation, deletion and write. 
	dwChangeHandles = FindFirstChangeNotification(szDir.c_str(), FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE);

	if (dwChangeHandles == INVALID_HANDLE_VALUE || dwChangeHandles == NULL) {
		OutputDebugString(L"ERROR: FindFirstChangeNotification function failed.\n");
		return false;
	}

	while (TRUE) {
		// Wait for notification.
		dwWaitStatus = WaitForSingleObject(dwChangeHandles, INFINITE);

		switch (dwWaitStatus) {
			case WAIT_OBJECT_0:

				// Handle event and restart the notification.
				PostMessage(hMainWnd, UM_FILES_CHANGED, 0, 0);
				if (FindNextChangeNotification(dwChangeHandles) == FALSE) {
					OutputDebugString(L"ERROR: FindNextChangeNotification function failed.\n");
					return false;
				}
				break;

			default:
				OutputDebugString(L"ERROR: Unhandled dwWaitStatus.\n");
				return false;
				break;
		}
	}
}*/

void WatchDirectory(const std::wstring& szDir, HWND hMainWnd)
{
	HANDLE hDir = CreateFile(
		szDir.c_str(),                                // pointer to the file name
		FILE_LIST_DIRECTORY,                // access (read/write) mode
		// Share mode MUST be the following to avoid problems with renames via Explorer!
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
		NULL,                               // security descriptor
		OPEN_EXISTING,                      // how to create
		FILE_FLAG_BACKUP_SEMANTICS,         // file attributes
		NULL                                // file with attributes to copy
		);

	if (hDir == INVALID_HANDLE_VALUE) {
		OutputDebugString(L"ERROR: Can not open dir.\n");
		return;
	}

	char szBuffer[1024 * 128];
	DWORD BytesReturned;
	while (ReadDirectoryChangesW(
		hDir,                          // handle to directory
		&szBuffer,                       // read results buffer
		sizeof(szBuffer),                // length of buffer
		TRUE,                          // monitoring option
		FILE_NOTIFY_CHANGE_SECURITY |
		FILE_NOTIFY_CHANGE_CREATION |
		FILE_NOTIFY_CHANGE_LAST_WRITE |
		FILE_NOTIFY_CHANGE_SIZE |
		FILE_NOTIFY_CHANGE_ATTRIBUTES |
		FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_FILE_NAME,  // filter conditions
		&BytesReturned,                // bytes returned
		NULL,                          // overlapped buffer
		NULL                           // completion routine
		)
		) {
		DWORD dwOffset = 0;
		FILE_NOTIFY_INFORMATION* pInfo = NULL;

		do {
			// Get a pointer to the first change record...
			pInfo = (FILE_NOTIFY_INFORMATION*)&szBuffer[dwOffset];

			std::wstring action = L"*";
			switch (pInfo->Action) {
				case FILE_ACTION_ADDED: action = L"Added"; break;
				case FILE_ACTION_REMOVED: action = L"Deleted"; break;
				case FILE_ACTION_MODIFIED: action = L"Modified"; break;
				case FILE_ACTION_RENAMED_OLD_NAME: action = L"Old name"; break;
				case FILE_ACTION_RENAMED_NEW_NAME: action = L"New name"; break;
			}

			std::wstring szFileName(pInfo->FileName, pInfo->FileNameLength / 2);
			
			//OutputDebugString((action + L": " + szFileName + L"\n").c_str());
			AppendText(GetDlgItem(hMainWnd, IDC_EDIT_TRACE), action + L": " + szFileName + L"\r\n");
			//PostMessage(hMainWnd, UM_FILES_CHANGED, 0, 0);

			// More than one change may happen at the same time. Load the next change and continue...
			dwOffset += pInfo->NextEntryOffset;
		} while (pInfo->NextEntryOffset != 0);
	}
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

		if (settingsHandler.GetSyncPath() != L"") {
			// start new thread to listen for files changes
			std::thread watchDirThread(WatchDirectory, settingsHandler.GetSyncPath(), hDlg);
			watchDirThread.detach();
		}
		
		return (INT_PTR)TRUE;

	case UM_FILES_CHANGED:
		//UploadFiles(hDlg);
		OutputDebugString(L"files changed\n");
		return (INT_PTR)TRUE;

	case UM_UPLOAD_FILES:
		UploadFiles(hDlg);
		return (INT_PTR)TRUE;

	case UM_CHECK_LOGIN:
		if (!loginHandler.IsLoggedIn()) {
			if (settingsHandler.HaveCredentials()) {
				if (loginHandler.LogIn(settingsHandler.GetUserID(), settingsHandler.GetPass())) {
					SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_TRACE), ("Sign In responce:\r\n" + loginHandler.loginTrace + "\r\n").c_str());
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
		SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_TRACE), ("Sign In responce:\r\n" + loginHandler.loginTrace + "\r\n").c_str());
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