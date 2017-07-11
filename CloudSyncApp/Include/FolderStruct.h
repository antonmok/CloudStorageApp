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

	void IterateTree(THandleNodeCallback HandleNodeCallback, bool remote);

private:
	CDirectoryTree() {}
	~CDirectoryTree() {}

	bool BuildLocalTree(TDirTree::iterator dirTreeIt, const std::wstring& dirPath);
	bool ParseRemoteTree(TDirTree::iterator dirTreeIt, const Value& childrenArray);
	void TreeIterator(const TDirTree& t, TDirTree::iterator tIt, THandleNodeCallback HandleNodeCallback);
	TDirTree localTree;
	TDirTree remoteTree;

};

bool SelectPathDialog(std::wstring& path);
void WatchDirectory(const std::wstring& szDir, HWND hMainWnd);