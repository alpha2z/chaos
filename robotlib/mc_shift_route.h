#ifndef __MSG_CENTER_SHIFT_ROUTE_H
#define __MSG_CENTER_SHIFT_ROUTE_H

#include "mc_msg.h"

class CMcShiftRoute : public CMcMsg
{
	public:
		enum {
			SUB_CMD_REQ = 0,
			SUB_CMD_RESP = 1,
		};

		enum {
			IM_TYPE_QQ = 1,
		};

		enum {
			SHIFT_TYPE_TO_ROBOT = 1,
			SHIFT_TYPE_TO_MANUAL = 2,
		};

		enum {
			BUSINESS_SET_RESULT_SUC = 0,
			BUSINESS_SET_RESULT_FAIL = 1,
		};

		enum {
			RESULT_SUC = 0,
			RESULT_FAIL = 1,
		};

		CMcShiftRoute() {}
		~CMcShiftRoute() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	public:
		uint8_t			mSubCmd;
		string			mSystemID;
		uint8_t			mType;
		uint8_t			mOldServerType;
		uint16_t		mOldServerNo;
		uint8_t			mNewServerType;
		uint16_t		mNewServerNo;
		uint32_t		mSessionID;
		string			mOpenID;
		string			mUserNick;
		uint8_t			mGender;
		uint32_t		mBirthday;
		uint32_t		mBusinessSetID;
		uint8_t			mBusinessSetResult;
		string          mBusinessSetResultContent;
		uint8_t			mIMType;
		string			mUserName;
		uint16_t		mCity;
		string			mKFName;
		uint16_t		mAttrID;
		uint16_t		mOgzID;
		uint8_t			mLeaveWordType;
		uint16_t		mTXCityID;
		uint32_t		mTXUin;
		uint8_t			mResult;
		uint32_t		mTXBitmapFlag;
		uint32_t		mNewSessionID;
		uint32_t        mKfID ;
		string          mContent;
};

#endif
