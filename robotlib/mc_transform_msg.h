#ifndef __MSG_CENTER_TRANSFORM_MSG_H
#define __MSG_CENTER_TRANSFORM_MSG_H

#include "mc_msg.h"

class CMcTransformMsg : public CMcMsg
{
	public:
		enum {
			SUB_CMD_REQ_MSG = 0,
			SUB_CMD_RESP_MSG = 1,
		};

		CMcTransformMsg() {}
		~CMcTransformMsg() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	public:
		uint8_t			mSubCmd;
		string			mSystemID;
		uint32_t		mSessionID;
		uint32_t		mSeq;
		string          mSessionTicket;
		uint8_t			mIMType;
		uint8_t			mSenderType;
		string			mFromUserName;
		string			mToUserName;
		string			mContent;
		uint8_t			mResult;
		uint32_t        mTXCityID;
		uint32_t        mTXUin;
		uint32_t		mTXBitmapFlag;
};

#endif
