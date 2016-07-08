#ifndef TELE_COMPRESS_H
#define TELE_COMPRESS_H
#include <string>
class TeleCompress {
 public:
    TeleCompress();
	~TeleCompress();
    std::string GetProcessResult(std::string input);
 private:
 
    int Uncompressbase64(const char*input, int inlen, unsigned char *output, int outlen);

    int Deflate_gzip(unsigned char*input, int inlen, unsigned char *output, int *outlen);
   
    int ReplaceTag(std::string &str, std::string old_value, std::string new_value);

    char* ungzip(const char* source,int len);  
	unsigned char *base_result ;
	
	unsigned char *gzip_result ;
};

#endif
