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
#include "Helpers.h"
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
	InitCommonControls();

	/*TODO: handle image list*/
	HIMAGELIST hImageList = ImageList_LoadBitmap(hInst, MAKEINTRESOURCEW(IDB_PNG_DIR), 16, 0, NULL);
	ImageList_Add(hImageList, LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PNG_FILE)), NULL);
	TreeView_SetImageList(GetDlgItem(hDlg, IDC_TREE), hImageList, TVSIL_NORMAL);

	if (settingsHandler.GetSyncPath() != L"") {
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (L"Folder:\n" + settingsHandler.GetSyncPath()).c_str());
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);
	}
}

void ShowRemoteTree(HWND hDlg)
{
	std::string fields("token=");
	std::string response;

	fields.append(loginHandler.GetToken());

	if (PostHttps(std::string(BASE_URL) + METHOD_GET_TREE, fields, response)) {
		dirTreeInstance.InitRemoteTree(response);
		dirTreeInstance.InitTreeCtrl(GetDlgItem(hDlg, IDC_TREE_REMOTE), true);
	}
}

void ShowLocalTree(HWND hDlg)
{
	dirTreeInstance.InitLocalTree(settingsHandler.GetSyncPath());
	dirTreeInstance.InitTreeCtrl(GetDlgItem(hDlg, IDC_TREE), false);
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

std::string GetNewObjectId(const std::string jsonStr)
{
	std::string idStr;

	// parse JSON
	Document doc;
	if (doc.Parse(jsonStr.c_str()).HasParseError()) {
		return "";
	}

	if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
		if (doc["success"].GetInt() == 1) {

			Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
			if (itrData != doc.MemberEnd()) {
				
				Value::ConstMemberIterator itrId = itrData->value.FindMember(FIELD_OBJ_ID);
				if (itrId != itrData->value.MemberEnd()) {
					idStr = itrId->value.GetString();
				}
			}
		}
	}

	return idStr;
}

bool RemotelyExist(const std::string& name, const std::string& remotePath, std::string& pathId)
{
	bool found = false;

	THandleNodeCallback FindObject = [&found, &name, &remotePath, &pathId](TDirTree::iterator nodeIt)
	{
		SDirNode& node = *nodeIt;

		if (node.remotePath == remotePath && node.name == name) {
			found = true;
			// update remote path for local tree
			pathId = node.pathId;
			// stop iterating
			return false;
		}

		return true;
	};

	dirTreeInstance.IterateTree(FindObject, true);

	return found;
}

void UploadFiles(HWND hDlg) {

	if (dirTreeInstance.LocalTreeIsSet()) {

		bool root = true;

		THandleNodeCallback UploadObject = [hDlg, &root](TDirTree::iterator nodeIt)
		{
			SDirNode& node = *nodeIt;
			std::wstring wideLocalPath;
			std::wstring wideName;
			std::wstring traceStr;
			std::string response;
			std::string newObjId;
			std::string remotePath;
			bool creationRes = false;

			s2ws(node.localPath, wideLocalPath);
			s2ws(node.name, wideName);

			// get remote path for object
			if (!root) {
				const SDirNode& parentNode = nodeIt.node->parent->data;

				if (parentNode.remotePath == "") {
					node.remotePath = "," + parentNode.pathId + ",";
				} else {
					node.remotePath = parentNode.remotePath + parentNode.pathId + ",";
				}
				
				// check if it is not exist already remotely
				if (RemotelyExist(node.name, node.remotePath, node.pathId)) {
					// skip this node
					EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), (L"\r\nObject already exist on server: " + wideLocalPath + L"\\" + wideName).c_str());
					return true;
				}

			} else {
				root = false;
				if (node.pathId.size()) {
					//no need to create remote root
					return true;
				}
			}

			if (node.isFile) {
				creationRes = CreateObject(std::string(BASE_URL) + METHOD_CREATE_FILE, node.localPath, node.remotePath, node.name, loginHandler.GetToken(), response);
				traceStr = L"file";
			} else {
				std::string fields(std::string(PARAM_TOKEN) + "=" + loginHandler.GetToken() + "&" + PARAM_NAME + "=" + node.name + "&" + PARAM_PATH + "=" + node.remotePath);
				creationRes = PostHttps(std::string(BASE_URL) + METHOD_CREATE_FOLDER, fields, response);
				traceStr = L"folder";
			}

			if (creationRes) {
				// extract new object id
				newObjId = GetNewObjectId(response);
				node.pathId = newObjId;

				EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), (L"\r\n" + traceStr + L" created: " + wideLocalPath + L"\\" + wideName).c_str());
			} else {
				EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), (L"\r\nFailed to create " + traceStr + L": " + wideLocalPath + L"\\" + wideName).c_str());
				return false;
			}

			return true;
		};

		dirTreeInstance.IterateTree(UploadObject, false);
		EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), L"\r\n");
		ShowRemoteTree(hDlg);
		
	} else {
		MessageBox(hDlg, L"Upload directory is empty", L"Error", MB_ICONEXCLAMATION);
		return;
	}

	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), TRUE);
	SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), L"Upload Folder");

	return;
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
			ShowLocalTree(hDlg);
		}

		return (INT_PTR)TRUE;

	case UM_FILES_CHANGED:
	{
		std::shared_ptr<std::wstring> action(reinterpret_cast<std::wstring*>(wParam));
		std::shared_ptr<std::wstring> fileName(reinterpret_cast<std::wstring*>(lParam));

		EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), *action + L": " + *fileName + L"\r\n");
		
		if (!IsFilteredAction(*action, *fileName)) {
			//UploadFiles(hDlg);
			ShowLocalTree(hDlg);
		}

		return (INT_PTR)TRUE;
	}

	case UM_UPLOAD_FILES:
		UploadFiles(hDlg);
		return (INT_PTR)TRUE;

	case UM_CHECK_LOGIN:
		if (!loginHandler.IsLoggedIn()) {
			if (settingsHandler.HaveCredentials()) {
				if (loginHandler.LogIn(settingsHandler.GetUserID(), settingsHandler.GetPass())) {
					PostMessage(hDlg, UM_LOGIN_COMPLETE, NULL, NULL);
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
	{
		std::wstring wideStr;
		s2ws(loginHandler.loginTrace, wideStr);
		EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), L"Sign In responce:\r\n" + wideStr + L"\r\n");

		ShowRemoteTree(hDlg);

		return (INT_PTR)TRUE;
	}

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
			
			if (SelectPathDialog(path)) {
				settingsHandler.SetSyncPath(path);
				settingsHandler.SaveSettings();

				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_UPFOLDER), (L"Folder:\n" + path).c_str());
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPLOAD), true);
				
				ShowLocalTree(hDlg);
			}

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