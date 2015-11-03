#ifndef __BUSINESS_NOTIFY_RESULT_H
#define __BUSINESS_NOTIFY_RESULT_H

#include "mc_msg.h"

class CBusinessNotifyResult : public CMcMsg
{
	public:
		enum {
			RESULT_SUC = 0,
			RESULT_FAIL = 1,
		};

		enum {
			BUSINESS_RESULT_SUC = 0,
			BUSINESS_RESULT_FAIL = 1,
		};

		CBusinessNotifyResult() {}
		~CBusinessNotifyResult() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	private:
		string getField(string text, string delimiter, size_t beginPos, size_t &endPos);
	public:
		string			mSystemID;
		uint8_t			mFromServerType;
		uint8_t			mToServerType;
		uint32_t		mSessionID;
		uint32_t		mBusinessSetID;
		uint8_t			mBusinessSetResult;
		string          mBusinessSetResultContent;
		string			mContent;
		uint8_t			mResult;
};

#endif
