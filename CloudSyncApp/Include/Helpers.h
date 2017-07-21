#pragma once

#include "stdafx.h"
#include <string>
#include <vector>

typedef unsigned char BYTE;

std::vector<std::string> split(const std::string &s, char delim);
void StrToupper(std::wstring& str);

int ScaleDPI(int x);
RECT GetCtrlLocalCoordinates(HWND hWnd);
void EditAppendText(const HWND &hwndOutput, const std::wstring& newText);
void LogFileAction(HWND hEdit, int action, const std::wstring flName);

bool RunProcess(std::wstring path, std::wstring cmd);
bool ExecInstaller(const std::wstring& path, const std::wstring& cmd);