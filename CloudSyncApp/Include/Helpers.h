#pragma once

#include "stdafx.h"
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
typedef unsigned char BYTE;

void s2ws(const std::string& str, std::wstring& outStr);
void ws2s(const std::wstring& wstr, std::string& outStr);

std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const&);

bool SelectPathDialog(std::wstring& path);
