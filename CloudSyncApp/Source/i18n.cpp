#define NOMINMAX

#include "stdafx.h"


#include <cctype>
#include <locale>


#include "i18n.h"
#include "cereal/external/rapidjson/document.h"
#include "cereal/external/rapidjson/reader.h"
#include "JSONFields.h"
#include "EncodingHelper.h"
#include "NetHelper.h"
#include "Helpers.h"

using namespace rapidjson;

void Ci18n::SetLangCodeFromSystemLocale()
{
	std::wstring localeName(LOCALE_NAME_MAX_LENGTH, 0);
	GetUserDefaultLocaleName(&localeName[0], LOCALE_NAME_MAX_LENGTH);
	localeName = localeName.substr(0, 2);
	StrToupper(localeName);
	SetLangCode(localeName);
}

Ci18n::Ci18n()
{
	std::string key;
	Si18nItem i18nItem;

	SetLangCodeFromSystemLocale();

	key = "login_title";
	i18nItem.def = L"Login";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "first_name_lb";
	i18nItem.def = L"First Name";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "last_name_lb";
	i18nItem.def = L"Last Name";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "email_lb";
	i18nItem.def = L"Email";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "password_lb";
	i18nItem.def = L"Password";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "terms_check";
	i18nItem.def = L"I agree to the Terms of use";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "sign_up_btn";
	i18nItem.def = L"Sign up";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "sign_in_btn";
	i18nItem.def = L"Sign in";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "already_have_acc_lb";
	i18nItem.def = L"Have account already?";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "sign_in_link";
	i18nItem.def = L"SIGN IN";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "exit_btn";
	i18nItem.def = L"Exit";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "main_title";
	i18nItem.def = L"CloudSyncApp";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "folder_lb";
	i18nItem.def = L"Folder: ";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "set_folder_btn";
	i18nItem.def = L"Set Folder";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "upload_btn";
	i18nItem.def = L"Upload Folder";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "upload_btn_dis";
	i18nItem.def = L"Uploading...";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "local_lb";
	i18nItem.def = L"Local";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "remote_lb";
	i18nItem.def = L"Remote";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "log_out_btn";
	i18nItem.def = L"Log Out";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "ok_btn";
	i18nItem.def = L"OK";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "cancel_btn";
	i18nItem.def = L"Cancel";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "update_title";
	i18nItem.def = L"Check Update";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "update_guide_lb";
	i18nItem.def = L"New version is available. Click Download to update.";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "download_btn";
	i18nItem.def = L"Download";
	i18nData.insert(std::make_pair(key, i18nItem));

	key = "skip_btn";
	i18nItem.def = L"Skip";
	i18nData.insert(std::make_pair(key, i18nItem));
}

const std::wstring& Ci18n::Geti18nItem(const std::string& key)
{
	auto i18nItem = i18nData.find(key);

	if (i18nItem != i18nData.end()) {
		switch (langCode) {
		case Ci18n::LC_DEFAULT:
			return i18nItem->second.def;
		
		case Ci18n::LC_ENGLISH:
			if (i18nItem->second.en.size()) {
				return i18nItem->second.en;
			}
		
		case Ci18n::LC_SPANISH:
			if (i18nItem->second.es.size()) {
				return i18nItem->second.es;
			}
		
		default:
			return i18nItem->second.def;
		}
	}

	return emptyKey;
}

void Ci18n::UpdateI18nItem(const std::string& key, const Si18nItem& item)
{
	auto i18nItem = i18nData.find(key);

	if (i18nItem != i18nData.end()) {
		i18nItem->second.en = item.en;
		i18nItem->second.es = item.es;
	}
}

void Ci18n::Seti18nDataFromJSON(const std::string& jsonStr)
{
	// parse JSON
	Document doc;

	if (doc.Parse(jsonStr.c_str()).HasParseError()) {
		return;
	}

	if (doc.HasMember(FIELD_SUCCESS) && doc[FIELD_SUCCESS].IsNumber()) {
		if (doc[FIELD_SUCCESS].GetInt() == 1) {

			Value::ConstMemberIterator itrData = doc.FindMember(FIELD_DATA);
			if (itrData != doc.MemberEnd() && itrData->value.IsArray()) {

				std::string key;
				std::string EN;
				std::string ES;

				for (Value::ConstValueIterator itr = itrData->value.Begin(); itr != itrData->value.End(); ++itr) {

					Value::ConstMemberIterator itrKey= itr->FindMember(FIELD_KEY);
					if (itrKey!= itr->MemberEnd()) {
						key = itrKey->value.GetString();
					} else {
						continue;
					}

					Value::ConstMemberIterator itrTrans = itr->FindMember(FIELD_TRANS);
					if (itrTrans != itr->MemberEnd() && itrTrans->value.IsObject()) {

						Value::ConstMemberIterator itrEN = itrTrans->value.FindMember(FIELD_TRANS_EN);
						if (itrEN!= itrTrans->value.MemberEnd()) {
							EN = itrEN->value.GetString();
						}

						Value::ConstMemberIterator itrES = itrTrans->value.FindMember(FIELD_TRANS_ES);
						if (itrES != itrTrans->value.MemberEnd()) {
							ES = itrES->value.GetString();
						}
					}

					Si18nItem item;
					UTF8ToWs(EN, item.en);
					UTF8ToWs(ES, item.es);
					
					UpdateI18nItem(key, item);
				}
			}
		}
	}
}

void Ci18n::SetLangCode(const std::wstring& languageCode)
{
	if (languageCode == LangCodeStr_en) {
		langCode = LC_ENGLISH;
	} else if (languageCode == LangCodeStr_es) {
		langCode = LC_SPANISH;
	} else {
		langCode = LC_DEFAULT;
	}
}

void Ci18n::SetLangCode(ELangCode languageCode)
{
	langCode = languageCode;
}

Ci18n::ELangCode Ci18n::GetLangCode()
{
	return langCode;
}

void Ci18n::GetTranslationsFromServer()
{
	std::string fields(std::string(PARAM_TYPE) + "=" + TRANSLATION_TYPE);
	std::string responce;

	if (PostHttp(std::string(BASE_URL) + METHOD_I18N, fields, responce)) {
		Seti18nDataFromJSON(responce);
	}
}