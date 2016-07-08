#include "CurlWrapper.h"


CurlWrapper::CurlWrapper()
{
	CURLcode res = ::curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != res)
	{
		fprintf(stderr, "curl_global_init failed: %d \n", res);
		return;
	}

	return;
}


CurlWrapper::~CurlWrapper()
{
	curl_global_cleanup();
}

size_t http_data_writer(void* data, size_t size, size_t nmemb, void* content)
{
	long totalSize = size*nmemb;
	std::string* symbolBuffer = (std::string*)content;
	if (symbolBuffer)
	{
		symbolBuffer->append((char *)data, ((char*)data) + totalSize);
	}
	return totalSize;
}

std::string CurlWrapper::access_http(const char* szUrl)
{
	std::string strData = "";
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return strData;
	}
	CURLcode code;
#if 1
	/*SET connect type: keep-alive*/
	struct curl_slist *header_list = NULL;
	header_list = curl_slist_append(header_list, "Connection: keep-alive");
	if (NULL == header_list) {
    printf("append error\n");
		return strData;
	}
#endif
 // curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3);
 // curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	code = curl_easy_setopt(curl, CURLOPT_URL, szUrl);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_data_writer);

	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strData);
	code = curl_easy_perform(curl);
	if (code == CURLE_OK)
	{
		long responseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		if (responseCode < 200 || responseCode >= 300 || strData.empty())
		{
			std::cout << "code != CURLE_OK" << std::endl;
		}
	} else {
    printf("libcur error code %d\n", code);
  }
	curl_slist_free_all(header_list);
	curl_easy_cleanup(curl);
  if (strData.length() == 0) {
    printf("RecvOK but len is zero\n");
  }
	return strData;
}

CurlWrapper* CurlWrapper::wrapper_ = NULL;

CurlWrapper* CurlWrapper::get_instance()
{
	if (NULL == wrapper_) {
		wrapper_ = new CurlWrapper();
	}
	return wrapper_;
}

