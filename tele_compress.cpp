#include "tele_compress.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <glog/logging.h>

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <zlib.h>


#include <json/json.h>
#include <json/reader.h>


#define MAX_CONTENT_LENGTH (1024 * 1024)
#define SEGMENT_SIZE 4096

TeleCompress::TeleCompress() {
	base_result = (unsigned char*)malloc(MAX_CONTENT_LENGTH);
	gzip_result = (unsigned char*)malloc(MAX_CONTENT_LENGTH);
}

TeleCompress::~TeleCompress() {
	free(base_result);
	free(gzip_result);
}

int TeleCompress::Uncompressbase64(const char*input, int inlen, unsigned char *output, int outlen) {
	int len = 0;
	BIO *b64,*bmem;
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf((void*)input, inlen);
	bmem = BIO_push(b64, bmem);
	len = BIO_read(bmem, output, outlen);
	output[len] = 0;
	BIO_free_all(bmem);
	return len;
}

int TeleCompress::ReplaceTag(std::string &str, std::string old_value, std::string new_value) {
	for(std::string::size_type   pos(0);   pos!= std::string::npos;   pos += new_value.length()) {   
        if((pos=str.find(old_value,pos))!=std::string::npos)   
          str.replace(pos,old_value.length(), new_value);   
        else   
		  		break;   
    }   
    return   0;   
}

char* TeleCompress::ungzip(const char* source,int len)  {  
	int err;  
	z_stream d_stream;  
	Byte *compr = NULL, *uncompr = NULL; 
	char *b = NULL;
	if (len >= SEGMENT_SIZE)
		return NULL;
	if ( (compr = (Byte*)malloc(len)) == NULL)
		return NULL;
	memcpy(compr,(Byte*)source,len);  
	uLong comprLen, uncomprLen;  
	comprLen = SEGMENT_SIZE;  
	uncomprLen = 4*SEGMENT_SIZE;
	if ((uncompr = (Byte*)malloc(uncomprLen)) == NULL) {
		goto FAILED;
	}  
	strcpy((char*)uncompr, "garbage");  
	  
	d_stream.zalloc = (alloc_func)0;  
	d_stream.zfree = (free_func)0;  
	d_stream.opaque = (voidpf)0;  
	  
	d_stream.next_in = compr;  
	d_stream.avail_in = 0;  
	d_stream.next_out = uncompr;  
  
	err = inflateInit2(&d_stream,47);  
	if(err!=Z_OK)  {  
	   LOG(ERROR) << "inflateInit2 error:" << err;
	   goto FAILED;
	}  
	while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {  
	d_stream.avail_in = d_stream.avail_out = 1;  
	err = inflate(&d_stream,Z_NO_FLUSH);  
	    if(err == Z_STREAM_END) break;  
	    if(err!=Z_OK) {  
	    	LOG(ERROR) << "inflate error" << err;
	   	 goto FAILED;
	   }  
	}  
	err = inflateEnd(&d_stream);  
	if(err!=Z_OK) {  
	   LOG(ERROR) << "inflateEnd error:" << err;  
	   goto FAILED; 
	}  
	 b = new char[d_stream.total_out+1];  
	memset(b, 0, d_stream.total_out+1);
	memcpy(b, (char*)uncompr,d_stream.total_out);  
	free(compr);
	free(uncompr);
	return b;
	FAILED:
		if (compr)
			free(compr);
		if (uncompr)
			free(uncompr);
	return NULL;	
}  

int TeleCompress::Deflate_gzip(unsigned char *input, int inlen, unsigned char *output, int *outlen) {
   int  ret;   
   char *r = ungzip((char*)input, inlen);
   if (!r)
      return -1;
   strcpy((char*)output,r);
   delete [] r;
   *outlen = strlen((char*)output);
   return 0;
}

std::string TeleCompress::GetProcessResult(std::string input) {
	int rc;
	int len;
	Json::Value root;
	std::string result = "";
	std::string value;
	Json::Reader reader; 
	std::string json_value;
	if (input.length() == 0)
		return result;
	len = Uncompressbase64(input.c_str(), input.length(), base_result, MAX_CONTENT_LENGTH);
	if (len <= 0) {
		LOG(ERROR) << "Uncompress64 error len:" << len;
		return result;
	} else {
		//printf("unpack len %d\n", len);
		//printf("%s\n", base_result);
	}
	base_result[len] = 0;
	json_value = (const char*)base_result;
	int pos = json_value.rfind("_kunyan_");
	if (pos == std::string::npos)
		return result;
	std::string key = json_value.substr(pos + strlen("_kunyan_"));
	if (!reader.parse(json_value.substr(0, pos), root)) 
		return result;
	if (root["value"].isNull())
		return result;
	value = root["value"].asString();
	if (value.length() == 0)
		return result;
	ReplaceTag(value, "-<", "");
	memset(base_result, 0, MAX_CONTENT_LENGTH);
	len = Uncompressbase64(value.c_str(), value.length(), base_result, MAX_CONTENT_LENGTH);
	if (len <= 0) {
		LOG(ERROR) << "second unbase64 error len:" << len;
		return result;
	}
	int outlen = MAX_CONTENT_LENGTH;
	rc = Deflate_gzip(base_result, len, gzip_result, &outlen);
	if (rc != 0) {
		LOG(ERROR) << "unzip error rc:" << rc;
		return result;
	}
	//gzip_result[outlen] = 0;
	
	result = (char*)gzip_result;
	result += "\t" + key;
	return result;
}

#if 0
int test_telecomress(std::string input) {
	TeleCompress telecom;
	std::string result = telecom.GetProcessResult(input);
	if (result.length() > 0)
		printf("The result is %s\n", result.c_str());
	return 0;
}

int main(int argc , char *argv[]) {
	int rc = 0;
	google::InitGoogleLogging(argv[0]);
  //为不同级别的日志设置不同的文件basename。
    google::SetLogDestination(google::INFO,"/home/panghao/loginfo");   
    google::SetLogDestination(google::WARNING,"/home/panghao/logwarn");   
    google::SetLogDestination(google::GLOG_ERROR,"/home/panghao/logerror"); 
	FLAGS_logbufsecs = 10;
	char *buf = (char*)malloc(1024*1024);
	FILE *fp = fopen("err.bz", "rb");
	int len = fread(buf, 1, 1024*1024, fp);
	buf[len] = 0;
	fclose(fp);
	if (argc == 2)
		test_telecomress(argv[1]);
	return rc;
}
#endif
