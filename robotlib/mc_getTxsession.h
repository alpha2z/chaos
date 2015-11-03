#ifndef __CMcGetTxSession_H
#define __CMcGetTxSession_H

#include "mc_msg.h"

class CMcGetTxSession : public CMcMsg
{
	public:
		CMcGetTxSession() {}
		~CMcGetTxSession() {}

		enum {
			SUB_CMD_REQ_MSG = 0,
			SUB_CMD_RESP_MSG = 1,
		};

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

    public:
        /**
         * «Î«Û: systemId+openId+imType+forceRequest
         * ªÿ∏¥: systemId+openId+imType+cResult+qqSessionId+qqSessionTicket
         */

        uint8_t			mSubCmd;
        string			mSystemID;
        string          mOpenid ;
        uint8_t			mImType;
        uint8_t		    mForceRequest;	
        uint8_t			mResult;
        string          mSessionTicket;
        char            mcSessionTicket[180] ;
        uint_least32_t          mTxSessionId;

};

#endif
