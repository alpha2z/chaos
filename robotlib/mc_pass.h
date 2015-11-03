#ifndef __MSG_PASS_H
#define __MSG_PASS_H

#include "mc_msg.h"

class CMcPass : public CMcMsg
{
	public:
		CMcPass() {}
		~CMcPass() {}

		enum {
			SUB_CMD_REQ_MSG = 0,
			SUB_CMD_RESP_MSG = 1,
		};

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	public:
		uint8_t			mSubCmd;
		string			mSystemID;
		uint8_t			mFromServerType;
		uint16_t		mFromServerNo;
		uint8_t			mToServerType;
		uint16_t		mToServerNo;
		uint32_t		mSessionID;
		string			mContent;
		uint8_t			mResult;
		uint32_t		mBusinessCmd;
		uint8_t			mBusinessType;

};

#endif
