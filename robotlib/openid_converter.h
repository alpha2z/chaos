#ifndef __OPENID_CONVERTER_H
#define __OPENID_CONVERTER_H

#include <string>

class COpenIDConverter
{
	public:
		COpenIDConverter() {}
		~COpenIDConverter() {}

	public:
		int ChangeIMUserID4Client(std::string& IMUserID);
		int ChangeIMUserID2Client(std::string& IMUserID);

		int ConvertToText(char *sOutput,int iOutLen, char *sBuf, int iLen);
		int ConvertToBuf(char *sOutput,int iOutLen,char *psInput);
};

#endif

