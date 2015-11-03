#ifndef __MSG_CENTER_LOGIN_H
#define __MSG_CENTER_LOGIN_H

#include "mc_msg.h"

class CMcLogin : public CMcMsg
{
	public:
		enum {
			MC_LOGIN_CODE_SUC = 0x00,
			MC_LOGIN_CODE_FAIL = 0x01,
			MC_LOGIN_CODE_REDIRECT = 0x02,
		};

		CMcLogin() {}
		~CMcLogin() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	public:
		uint8_t			mServerType;
		uint8_t			mLoadLevel;
		uint16_t		mServerNo;
		uint8_t			mResult;
		uint32_t		mRedirectIP;
		uint16_t		mRedirectPort;

};

#endif
