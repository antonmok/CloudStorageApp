#include "stdafx.h"
#include "NetHelper.h"
#include <string>
#include "curl/curl.h"

#pragma comment(lib, "libcurl.lib")

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

bool PostHttps(const std::string& url, const std::string& postFields, std::string& resData)
{
	CURL *curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	//std::wstring szURL = urlencode_weak(url);
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

	// disable peer verification
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.3; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 OPR/45.0.2552.888");

	/* Now specify the POST data */
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postFields.c_str());

	//curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)postFields.size());

	/* get it data */
	res = curl_easy_perform(curl_handle);

	/* check for errors */
	if (res != CURLE_OK) {
		OutputDebugStringA(curl_easy_strerror(res));
	}
	else {
		if (chunk.size) {
			resData.assign(chunk.memory);
		}
		else {
			res = CURLE_READ_ERROR;
		}
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if (chunk.memory)
		free(chunk.memory);

	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return res == CURLE_OK;
}