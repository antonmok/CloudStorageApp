#pragma once

#include <string>
#include <map>

#define LangCodeStr_def	L"def"
#define LangCodeStr_en	L"EN"
#define LangCodeStr_es	L"ES"

class Ci18n {

public:
	struct Si18nItem {
		std::wstring def;
		std::wstring en;
		std::wstring es;
		/*...*/
	};

	enum ELangCode {
		LC_DEFAULT,
		LC_ENGLISH,
		LC_SPANISH,
		/*...*/
	};

	static Ci18n& Instance()
	{
		static Ci18n s;
		return s;
	}

	Ci18n(Ci18n const&);
	Ci18n& operator= (Ci18n const&);

	const std::wstring& Geti18nItem(const std::string& key);

	void GetTranslationsFromServer();
	void SetLangCode(ELangCode languageCode);
	void SetLangCode(const std::wstring& languageCode);
	ELangCode GetLangCode();

private:

	Ci18n();
	~Ci18n() {}

	void UpdateI18nItem(const std::string& key, const Si18nItem& item);
	void SetLangCodeFromSystemLocale();
	void Seti18nDataFromJSON(const std::string& json);

	std::map<std::string, Si18nItem> i18nData;
	ELangCode langCode;
	std::wstring emptyKey;
};