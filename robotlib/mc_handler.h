#ifndef __MSG_CENTER_HANDLER_H
#define __MSG_CENTER_HANDLER_H

#include <iostream>

#include "CoHandler.h"

#include "struct.h"
#include "mc_msg.h"
#include "mc_login.h"
#include "mc_transform_msg.h"
#include "mc_shift_route.h"
#include "mc_pass.h"
#include "mc_getTxsession.h"

class CMcHandler : public chaos::CoHandler {
	public:
		CMcHandler();
		virtual ~CMcHandler();
	public:
		virtual int handle(chaos::CoConnection *c); 
		virtual void close(chaos::CoConnection *c);
		virtual void ready(chaos::CoConnection *c);
		virtual void timeout(chaos::CoConnection *c);
		virtual void finish(chaos::CoConnection *c) { return ; }

		virtual void handlePkgWithTimeout(uint32_t pkgID, void *arg);

		static void doHello(int fd, short events, void *arg);

	private:
		int parseHead(chaos::CoConnection *c);

	private:
		CMcSimpleMsg				mSimpleMsg;
		CMcLogin				mMcLogin;
		CMcTransformMsg			mMcTransformMsg;
		CMcShiftRoute			mMcShiftRoute;
		CMcPass					mMcPass;
		CMcGetTxSession             mMCGetTxsession ;
};
#endif
