#include <string.h>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include "openid_converter.h"

using namespace std;

int COpenIDConverter::ConvertToBuf(char *sOutput,int iOutLen,char *psInput)
{
	int i;
	int iLen;
	char sChar[5];

	iLen = strlen(psInput);
	if(iLen%2 != 0) return -1;

	memset(sOutput, 0, iOutLen);
	memset(sChar, 0, sizeof(sChar));

	for(i=0; i<(iLen/2); i++)
	{
		strcpy(sChar,"");
		strncat(sChar, psInput+i*2, 2);
		sscanf(sChar,"%x", (unsigned int*)(sOutput+i));
	}

	return 0;
}

int COpenIDConverter::ConvertToText(char *sOutput,int iOutLen, char *sBuf, int iLen)
{
	int i;

	if (iLen*2>iOutLen) return -1;

	memset(sOutput, 0, iOutLen);
	for(i=0; i<iLen; i++)
	{
		sprintf(sOutput+i*2, "%02x", *((unsigned char *)(sBuf+i)));
	}
	return 0;
}

int COpenIDConverter::ChangeIMUserID4Client(string& IMUserID)
{
	//转换客户端转过来的im号码
	//把"8678" ，转成00 00 00 00 00 00 00 00 00 00 00 00 00 00 86 78,共16位
	
	return 0;

	char sOutput[100];
	memset( sOutput, 0, 100);

	int iIndex = IMUserID.length()/2;
	if( iIndex>16)
		iIndex=16;
	
	ConvertToBuf( sOutput+(16-iIndex) , 16 ,(char*)IMUserID.c_str() );
	IMUserID= string( sOutput ,16);

	return 0;
}

int COpenIDConverter::ChangeIMUserID2Client(string& IMUserID)
{
	//转换客户端转过来的im号码
	//把"8678" ，转成00 00 00 00 00 00 00 00 00 00 00 00 00 00 86 78,共16位
	
	return 0;

	char sBuff[10];

	uint32_t iIndex=0;
	for( ;iIndex<IMUserID.length();iIndex++)
	{
		if( IMUserID[iIndex]!=0)
			break;
	}
	
	if( iIndex>16)
		iIndex = 16;

	ConvertToText( sBuff ,sizeof(sBuff), (char*)(IMUserID.c_str()+iIndex),16-iIndex );
	IMUserID = string(sBuff, 2*(16-iIndex));

	for( uint32_t i=0;i<2*(16-iIndex);i++)
	{
		if( IMUserID[i] >='a' && IMUserID[i]<='z')
			IMUserID[i] = IMUserID[i] - ('a'-'A');
	}
	return 0;
}

