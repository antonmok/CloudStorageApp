#pragma once

#include <string>

#define BASE_URL		"http://13.59.116.57:3000/api"

#define METHOD_LOGIN	"/login"
#define METHOD_LOGOUT	"/logout"
#define METHOD_SIGNUP	"/signup"
#define METHOD_UPLOAD	"/uploadFile"

#define PARAM_EMAIL		"email"
#define PARAM_PASS		"password"
#define PARAM_FNAME		"first_name"
#define PARAM_LNAME		"last_name"
#define PARAM_TOKEN		"token"

#define FIELD_TOKEN		"access_token"
#define FIELD_SUCCESS	"success"
#define FIELD_DATA		"data"

bool PostHttps(const std::string& url, const std::string& postFields, std::string& resData);
bool UploadFile(const std::string& url, const std::string& filename, const std::string& token, std::string& resData);