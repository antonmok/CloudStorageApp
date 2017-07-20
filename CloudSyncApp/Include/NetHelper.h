#pragma once

#include "stdafx.h"
#include <string>
#include "JSONFields.h"

//#define BASE_URL		"http://13.59.116.57:3000/api"
#define BASE_URL		"http://13.58.207.110/api"

#define METHOD_LOGIN			"/login"
#define METHOD_LOGOUT			"/logout"
#define METHOD_SIGNUP			"/signup"
#define METHOD_CREATE_FILE		"/createObject"
#define METHOD_CREATE_FOLDER	"/createFolder"
#define METHOD_GET_TREE			"/getObjectList"
#define METHOD_VERSION			"/getVersion"
#define METHOD_I18N				"/getTranslations"

#define PARAM_EMAIL			"email"
#define PARAM_PASS			"password"
#define PARAM_FNAME			"first_name"
#define PARAM_LNAME			"last_name"
#define PARAM_TOKEN			"token"
#define PARAM_NAME			"name"
#define PARAM_PATH			"path"
#define PARAM_TYPE			"type"

#define TRANSLATION_TYPE	"windowApp"

bool PostHttp(const std::string& url, const std::string& postFields, std::string& resData);
bool CreateObject(const std::string& url, const std::string& localPath, const std::string& path, const std::string& name, const std::string& token, std::string& resData);

UINT GetFileSizeHTTP(const std::string& url);
bool GetFileHTTP(const std::string& url, const std::wstring& outPath, HWND hProgressBar);