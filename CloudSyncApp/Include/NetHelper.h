#pragma once

#include <string>

bool PostHttps(const std::string& url, const std::string& postFields, std::string& resData);