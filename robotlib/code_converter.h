#ifndef __CODE_CONVERTER_H
#define __CODE_CONVERTER_H

#include <string.h>
#include <iconv.h>
#include <iostream>

using namespace std;

class CodeConverter {
	private:
		iconv_t cd;
	public:
		CodeConverter(const char *from_charset,const char *to_charset) {
			cd = iconv_open(to_charset,from_charset);
		}
		
		~CodeConverter() {
			iconv_close(cd);
		}
		
		int convert(char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
			char **pin = &inbuf;
			char **pout = &outbuf;

			memset(outbuf,0,outlen);
			return iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
		}
};

#endif
