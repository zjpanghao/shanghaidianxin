#include "base64.h"
#include <openssl/pem.h>                                                        
#include <openssl/bio.h>                                                        
#include <openssl/evp.h> 

int Uncompressbase64(const char*input, int inlen, unsigned char *output, int outlen) {                 
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

