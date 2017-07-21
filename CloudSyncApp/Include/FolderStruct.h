#pragma once
#include <string>
#include <memory>
#include <functional>
#include <stdafx.h>

#include "tree.hh"
#include "cereal/external/rapidjson/document.h"
#include "cereal/external/rapidjson/reader.h"

using namespace rapidjson;

struct SDirNode {

	SDirNode() {}

	SDirNode(std::string name, bool isFile)
	{
		this->name = name;
		this->isFile = isFile;
	}

	std::string name;
	std::string pathId;
	std::string localPath;
	std::string remotePath;
	bool isFile;
};

struct SFileAction {

};

typedef tree<SDirNode> TDirTree;
typedef std::function<bool(TDirTree::iterator)> THandleNodeCallback;

class CDirectoryTree {
public:

	static CDirectoryTree& Instance()
	{
		static CDirectoryTree s;
		return s;
	}

	CDirectoryTree(CDirectoryTree const&);
	CDirectoryTree& operator= (CDirectoryTree const&);

	bool LocalTreeIsSet();
	bool RemoteTreeIsSet();

	void InitLocalTree(const std::wstring& dirPath);
	void InitRemoteTree(const std::string& jsonStr);
	void InitTreeCtrl(HWND hTreeView, bool remote);
	
	void WatchDirectory(const std::wstring& szDir, HWND hMainWnd);
	void UploadFiles(HWND hDlg);

	void IterateTree(THandleNodeCallback HandleNodeCallback, bool remote);
	bool RemoteNodeExist(const std::string& name, const std::string& remotePath, std::string& pathId);

private:
	CDirectoryTree() 
	{
		hDir = INVALID_HANDLE_VALUE;
	}

	~CDirectoryTree() {}

	bool BuildLocalTree(TDirTree::iterator dirTreeIt, const std::wstring& dirPath);
	bool ParseRemoteTree(TDirTree::iterator dirTreeIt, const Value& childrenArray);
	void TreeIterator(const TDirTree& t, TDirTree::iterator tIt, THandleNodeCallback HandleNodeCallback);
	void WatchDirectoryFunc(const std::wstring& szDir, HWND hMainWnd);
	void StopWatchingDir();
	void UploadFilesFunc(HWND hDlg);
	bool LocalTreeIsNotEmpty();

	TDirTree localTree;
	TDirTree remoteTree;
	
	std::wstring watchedDir;
	HANDLE hDir;
};

bool SelectPathDialog(std::wstring& path);
bool IsFilteredUploadAction(int action, const std::wstring& name);
bool IsFilteredUIAction(int action, const std::wstring& name);
std::string GetNewObjectId(const std::string jsonStr);