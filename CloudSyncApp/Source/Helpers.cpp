#define NOMINMAX

#include "stdafx.h"
#include "Helpers.h"
#include <string>
#include <sstream>
#include <locale>
#include <iterator>
#include <Shellapi.h>

void StrToupper(std::wstring& str)
{
	for (auto & c : str) c = std::toupper(c, std::locale());
}

template<typename Out>
void split(const std::string &s, char delim, Out result)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

int ScaleDPI(int x)
{
	HDC screen = GetDC(0);

	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(0, screen);

	return MulDiv(x, MulDiv(dpiX, 100, 96), 100);
}

RECT GetCtrlLocalCoordinates(HWND hWnd)
{
	RECT Rect;
	GetWindowRect(hWnd, &Rect);
	MapWindowPoints(HWND_DESKTOP, GetParent(hWnd), (LPPOINT)&Rect, 2);
	return Rect;
}

void EditAppendText(const HWND &hwndOutput, const std::wstring& newText)
{
	// get the current selection
	DWORD StartPos, EndPos;
	SendMessage(hwndOutput, EM_GETSEL, reinterpret_cast<WPARAM>(&StartPos), reinterpret_cast<WPARAM>(&EndPos));

	// move the caret to the end of the text
	int outLength = GetWindowTextLength(hwndOutput);
	SendMessage(hwndOutput, EM_SETSEL, outLength, outLength);

	// insert the text at the new caret position
	SendMessage(hwndOutput, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(newText.c_str()));

	// restore the previous selection
	SendMessage(hwndOutput, EM_SETSEL, StartPos, EndPos);
}

bool ExecInstaller(const std::wstring& path, const std::wstring& cmd)
{
	SHELLEXECUTEINFO shExInfo = { 0 };
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = L"runas";
	shExInfo.lpFile = path.c_str();
	shExInfo.lpParameters = cmd.c_str();
	shExInfo.lpDirectory = 0;
	shExInfo.nShow = SW_SHOW;
	shExInfo.hInstApp = 0;

	if (ShellExecuteEx(&shExInfo)) {
		CloseHandle(shExInfo.hProcess);
		return true;
	}

	return false;
}

bool RunProcess(std::wstring path, std::wstring cmd)
{
	bool execRes = false;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (path.size() > 0) {
		if (CreateProcess(path.c_str(), (wchar_t*)cmd.c_str(), NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
			// Close process and thread handles. 
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return true;
		} else {
			int err = GetLastError();
		}
	}

	return false;
}

void LogFileAction(HWND hEdit, int action, const std::wstring flName)
{
	std::wstring actionStr;

	switch (action) {
	case FILE_ACTION_ADDED: actionStr = L"Added"; break;
	case FILE_ACTION_REMOVED: actionStr = L"Deleted"; break;
	case FILE_ACTION_MODIFIED: actionStr = L"Modified"; break;
	case FILE_ACTION_RENAMED_OLD_NAME: actionStr = L"Old name"; break;
	case FILE_ACTION_RENAMED_NEW_NAME: actionStr = L"New name"; break;
	}

	EditAppendText(hEdit, actionStr + L": " + flName + L"\r\n");
}

/*void PrintTree(const TDirTree& t, TDirTree::iterator tIt)
{
	if (t.empty()) return;
	if (t.number_of_children(tIt) == 0) {
		OutputDebugStringA(tIt->name.c_str());
	} else {
		// parent
		OutputDebugStringA(tIt->name.c_str());
		OutputDebugStringA("(");
		// child1, ..., child_n
		int siblingCount = t.number_of_siblings(t.begin(tIt));
		int siblingNum;
		TDirTree::sibling_iterator iChildren;
		for (iChildren = t.begin(tIt), siblingNum = 0; iChildren != t.end(tIt); ++iChildren, ++siblingNum) {
			// recursively print child
			PrintTree(t, iChildren);
			// comma after every child except the last one
			if (siblingNum != siblingCount) {
				OutputDebugStringA(", ");
			}
		}
		OutputDebugStringA(")\n");
	}
}*/