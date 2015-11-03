#ifndef __HANDLER_H
#define __HANDLER_H

#include <iostream>

#include "CoHandler.h"

#include "struct.h"
#include "mc_msg.h"
#include "business_notify_result.h"

class CHandler : public chaos::CoHandler {
	public:
		CHandler();
		virtual ~CHandler();
	public:
		virtual int handle(chaos::CoConnection *c); 
		virtual void close(chaos::CoConnection *c);
		virtual void ready(chaos::CoConnection *c);
		virtual void timeout(chaos::CoConnection *c);
		virtual void finish(chaos::CoConnection *c) { return ; }

		void handlePkgWithTimeout(uint32_t pkgID, void *arg);
	private:
		int parseHead(chaos::CoConnection *c);

	private:
		CMcSimpleMsg				mSimpleMsg;
		CBusinessNotifyResult		mBusinessNotifyResult;
};
#endif
