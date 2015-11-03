#ifndef _BASE64_H
#define _BASE64_H

class Base64{
public:
	int to64frombits(unsigned char *out, const unsigned char *in, int inlen);
	int from64tobits(char *out, const char *in, int maxlen);
};

#endif

