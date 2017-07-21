#pragma once

#include <string>
#include <vector>

void UTF8ToWs(const std::string& str, std::wstring& outStr);
void s2ws(const std::string& str, std::wstring& outStr);
void ws2s(const std::wstring& wstr, std::string& outStr);

std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const&);