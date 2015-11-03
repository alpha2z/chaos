#ifndef __REQ_SEARCH_H
#define __REQ_SEARCH_H

#include <sstream>

#include "CoConnection.h"

#include "struct.h"

#include "tinyxml.h"

typedef struct {
	float			score;
	std::string		id;
	std::string		keywords;
	std::string		question;
	std::string		anwser;
}SearchResult;

class CReqSearch 
{
	public:
		CReqSearch() {}
		~CReqSearch() {}

		int encode();
		int decode();

		int processReq(chaos::CoConnection *c);
		int processResp(chaos::CoConnection *c);

	private:
		std::string URLEncode(const string &c);
		std::string char2hex( char dec );
	public:
		std::string			mURL;
		std::string			mIndent;
		std::string			mVer;
		std::string			mQ;
		std::string			mFQ;
		std::string			mStart;
		std::string			mRows;
		std::string			mFL;
		std::string			mQT;
		std::string			mWT;
		std::string			mExplainOther;
		std::string			mHL;
		std::string			mHlFl;
		std::string			mTV;
		std::string			mTvAll;

		std::stringstream	mReqData;
		
		TiXmlDocument mDoc;

		std::string			mStatus;
		std::string			mNum;
		
		map<std::string, SearchResult>		mResult;
		chaos::CoConnection	*mConn;
};

#endif
