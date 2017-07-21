#define NOMINMAX

#include "FolderStruct.h"
#include <Shobjidl.h>
#include <thread>
#include "EncodingHelper.h"
#include "Helpers.h"
#include "NetHelper.h"
#include "SettingsHandler.h"
#include "LoginHandler.h"
#include "JSONFields.h"
#include "resource.h"

bool IsINode(const std::wstring fileName)
{
	return std::wstring(fileName) == L"." || std::wstring(fileName) == L"..";
}

bool CDirectoryTree::LocalTreeIsSet()
{
	if (localTree.number_of_siblings(localTree.head)) {
		return true;
	}

	return false;
}

bool CDirectoryTree::LocalTreeIsNotEmpty()
{
	if (localTree.number_of_children(localTree.head->next_sibling)) {
		return true;
	}

	return false;
}

bool CDirectoryTree::RemoteTreeIsSet()
{
	if (remoteTree.number_of_siblings(remoteTree.head)) {
		return true;
	}

	return false;
}

HTREEITEM AddItemToTree(HWND hwndTV, const std::string& itemNme, HTREEITEM hParent)
{
	TVITEM tvi;
	TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT | TVIF_IMAGE;

	// Set the text of the item.
	std::wstring name;
	s2ws(itemNme, name);
	tvi.pszText = (LPTSTR)name.c_str();
	tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);
	tvi.iImage = 1;
	//tvi.lParam = (LPARAM)nLevel;

	tvins.item = tvi;
	tvins.hInsertAfter = TVI_LAST;
	tvins.hParent = hParent;

	// Add the item to the tree-view control. 
	return (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
}

void SetTreeControl(HWND hTreeCtrl, const TDirTree& t, TDirTree::iterator tIt, HTREEITEM hParent)
{
	if (t.empty()) return;

	if (t.number_of_children(tIt) == 0) {
		AddItemToTree(hTreeCtrl, tIt->name, hParent);
	}
	else {
		// subdir
		hParent = AddItemToTree(hTreeCtrl, tIt->name, hParent);
		// files
		int siblingNum;
		TDirTree::sibling_iterator iChildren;

		for (iChildren = t.begin(tIt), siblingNum = 0; iChildren != t.end(tIt); ++iChildren, ++siblingNum) {
			// recursively add child
			SetTreeControl(hTreeCtrl, t, iChildren, hParent);
		}
	}

	SendMessage(hTreeCtrl, TVM_EXPAND, TVE_EXPAND, (LPARAM)hParent);
}

void CDirectoryTree::InitTreeCtrl(HWND hTreeView, bool remote)
{
	TDirTree::iterator root = remote ? remoteTree.begin() : localTree.begin();

	if (remote ? remoteTree.number_of_siblings(remoteTree.head) : localTree.number_of_siblings(localTree.head)) {
		TreeView_DeleteAllItems(hTreeView);
		SetTreeControl(hTreeView, remote ? remoteTree : localTree, root, TVI_ROOT);
	}
}

bool CDirectoryTree::BuildLocalTree(TDirTree::iterator dirTreeIt, const std::wstring& dirPath)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	bool childRes = true;

	std::wstring path(dirPath);
	std::string narrowPath;

	ws2s(dirPath, narrowPath);

	path.append(L"\\*");
	hFind = FindFirstFile(path.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		OutputDebugString(L"Upload directory access error");
		return false;
	}

	// List all the files in the directory
	do {
		std::string fileName;
		SDirNode node;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

			// sub dir
			if (!IsINode(ffd.cFileName)) {
				ws2s(ffd.cFileName, fileName);
				node.name = fileName;
				node.localPath = narrowPath;
				node.isFile = false;
				TDirTree::iterator subDirTreeIt = localTree.append_child(dirTreeIt, node);
				childRes = BuildLocalTree(subDirTreeIt, dirPath + L"\\" + ffd.cFileName);
			}
		} else {
			ws2s(ffd.cFileName, fileName);
			node.name = fileName;
			node.localPath = narrowPath;
			node.isFile = true;

			localTree.append_child(dirTreeIt, node);
		}
	} while (childRes && (FindNextFile(hFind, &ffd) != 0));

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		OutputDebugString(L"File access error");
		return false;
	}

	FindClose(hFind);

	return true;
}

void CDirectoryTree::InitLocalTree(const std::wstring& dirPath)
{
	if (LocalTreeIsSet()) {
		localTree.clear();
	}

	std::string rootName;
	std::size_t pos = dirPath.rfind(L"\\") + 1;

	ws2s(dirPath.substr(pos, dirPath.size() - pos), rootName);

	TDirTree::iterator root;
	SDirNode node(rootName, false);

	if (RemoteTreeIsSet()) {
		node.pathId = remoteTree.begin()->pathId;
	}

	root = localTree.insert(localTree.begin(), node);
	BuildLocalTree(root, dirPath);
}

bool CDirectoryTree::ParseRemoteTree(TDirTree::iterator dirTreeIt, const Value& childrenArray)
{
	std::string id;
	std::string name;
	std::string type;
	SDirNode node;

	for (Value::ConstValueIterator itr = childrenArray.Begin(); itr != childrenArray.End(); ++itr) {

		Value::ConstMemberIterator itrType = itr->FindMember(FIELD_OBJ_TYPE);
		if (itrType != itr->MemberEnd()) {
			type = itrType->value.GetString();
		} else {
			return false;
		}

		Value::ConstMemberIterator itrId = itr->FindMember(FIELD_OBJ_ID);
		if (itrId != itr->MemberEnd()) {
			id = itrId->value.GetString();
		} else {
			return false;
		}

		Value::ConstMemberIterator itrName = itr->FindMember(FIELD_OBJ_NAME);
		if (itrName != itr->MemberEnd()) {
			name = itrName->value.GetString();
		} else {
			return false;
		}

		node.name = name;
		node.localPath = "";
		node.pathId = id;
		if (dirTreeIt != NULL) {
			if (dirTreeIt->remotePath == "") {
				node.remotePath = "," + dirTreeIt->pathId + ",";
			} else {
				node.remotePath = dirTreeIt->remotePath + dirTreeIt->pathId + ",";
			}
		}
		
		node.isFile = (type != "category");
		TDirTree::iterator subDirTreeIt;

		if (dirTreeIt == NULL) {
			subDirTreeIt = remoteTree.insert(remoteTree.begin(), node);
			if (LocalTreeIsSet()) {
				localTree.begin()->pathId = id;
			}
		} else {
			subDirTreeIt = remoteTree.append_child(dirTreeIt, node);
		}

		Value::ConstMemberIterator itrChildren = itr->FindMember(FIELD_CHILDREN);
		if (itrChildren != itr->MemberEnd() && itrChildren->value.IsArray()) {
			if (!ParseRemoteTree(subDirTreeIt, itrChildren->value)) return false;
		} else {
			return false;
		}
	}

	return true;
}

void CDirectoryTree::InitRemoteTree(const std::string& jsonStr)
{
	if (LocalTreeIsSet()) {
		remoteTree.clear();
	}

	// parse JSON
	Document doc;
	if (doc.Parse(jsonStr.c_str()).HasParseError()) {
		return;
	}

	if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
		if (doc["success"].GetInt() == 1) {

			Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
			if (itrData != doc.MemberEnd()) {
				if (itrData->value.IsArray()) {
					ParseRemoteTree(NULL, itrData->value);
				}
			}
		}
	}
}

void CDirectoryTree::TreeIterator(const TDirTree& t, TDirTree::iterator tIt, THandleNodeCallback HandleNodeCallback)
{
	if (t.empty()) return;
	if (t.number_of_children(tIt) == 0) {
		if (!HandleNodeCallback(tIt)) return;
	} else {
		// parent
		if (!HandleNodeCallback(tIt)) return;

		int siblingNum;
		TDirTree::sibling_iterator iChildren;
		for (iChildren = t.begin(tIt), siblingNum = 0; iChildren != t.end(tIt); ++iChildren, ++siblingNum) {
			// iterate child
			TreeIterator(t, iChildren, HandleNodeCallback);
		}
	}
}

void CDirectoryTree::IterateTree(THandleNodeCallback HandleNodeCallback, bool remote)
{
	TDirTree::iterator it = remote ? remoteTree.begin() : localTree.begin();
	TreeIterator(remote ? remoteTree : localTree, it, HandleNodeCallback);
}

bool CDirectoryTree::RemoteNodeExist(const std::string& name, const std::string& remotePath, std::string& pathId)
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

	IterateTree(FindObject, true);

	return found;
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

void CDirectoryTree::UploadFiles(HWND hDlg)
{
	std::thread watchDirThread(&CDirectoryTree::UploadFilesFunc, std::ref(*this), hDlg);
	watchDirThread.detach();
}

void CDirectoryTree::UploadFilesFunc(HWND hDlg)
{

	if (LocalTreeIsNotEmpty()) {

		bool root = true;
		CLoginHandler& loginHandler = CLoginHandler::Instance();

		THandleNodeCallback UploadObjects = [hDlg, &root, &loginHandler, this](TDirTree::iterator nodeIt)
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
				if (RemoteNodeExist(node.name, node.remotePath, node.pathId)) {
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
				creationRes = PostHttp(std::string(BASE_URL) + METHOD_CREATE_FOLDER, fields, response);
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

		IterateTree(UploadObjects, false);
		PostMessage(hDlg, UM_UPLOAD_COMPLETE, true, 0);
	} else {
		PostMessage(hDlg, UM_UPLOAD_COMPLETE, false, 0);
		EditAppendText(GetDlgItem(hDlg, IDC_EDIT_TRACE), L"\nUpload directory is empty\n");
	}
}

bool SelectPathDialog(std::wstring& path)
{
	IFileDialog *pfd;
	WCHAR* pathBuf = NULL;

	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

	if (SUCCEEDED(hr)) {

		DWORD dwOptions;
		if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}

		if (SUCCEEDED(pfd->Show(NULL))) {
			IShellItem *psi;
			if (SUCCEEDED(pfd->GetResult(&psi))) {
				if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pathBuf))) {
					path.assign(pathBuf);
					CoTaskMemFree(pathBuf);

					return true;
				}
				psi->Release();
			}
		}
		pfd->Release();
	}

	return false;
}

void CDirectoryTree::StopWatchingDir()
{
	if (hDir != INVALID_HANDLE_VALUE) {
		CancelIoEx(hDir, NULL);
		hDir = INVALID_HANDLE_VALUE;
	}
}

void CDirectoryTree::WatchDirectory(const std::wstring& szDir, HWND hMainWnd)
{
	StopWatchingDir();

	std::thread watchDirThread(&CDirectoryTree::WatchDirectoryFunc, std::ref(*this), szDir, hMainWnd);
	watchDirThread.detach();
}

void CDirectoryTree::WatchDirectoryFunc(const std::wstring& szDir, HWND hMainWnd)
{
	hDir = CreateFile(
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

	watchedDir = szDir;

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
		)) {

		DWORD dwOffset = 0;
		FILE_NOTIFY_INFORMATION* pInfo = NULL;

		do {
			// Get a pointer to the first change record...
			pInfo = (FILE_NOTIFY_INFORMATION*)&szBuffer[dwOffset];
			std::wstring szFileName(pInfo->FileName, pInfo->FileNameLength / 2);
			PostMessage(hMainWnd, UM_FILES_CHANGED, (WPARAM)(pInfo->Action), (LPARAM)(new std::wstring(szFileName)));

			// More than one change may happen at the same time. Load the next change and continue...
			dwOffset += pInfo->NextEntryOffset;
		} while (pInfo->NextEntryOffset != 0);
	}
}

bool IsDirectory(const std::wstring path)
{
	if (GetFileAttributes(path.c_str()) == FILE_ATTRIBUTE_DIRECTORY) {
		return true;
	}
	return false;
}

bool IsFilteredUploadAction(int action, const std::wstring& name)
{
	if (action == FILE_ACTION_RENAMED_OLD_NAME) {
		return true;
	} else if (action == FILE_ACTION_RENAMED_NEW_NAME) {
		return true;
	} else if (action == FILE_ACTION_MODIFIED) {
		
		/*if (IsDirectory()) {
			return true;
		}*/

		return true;
	} else if (action == FILE_ACTION_REMOVED) {
		return true;
	}

	return false;
}

bool IsFilteredUIAction(int action, const std::wstring& name)
{
	if (action == FILE_ACTION_RENAMED_OLD_NAME) {
		return true;
	} else if (action == FILE_ACTION_MODIFIED) {
		return true;
	}

	return false;
}