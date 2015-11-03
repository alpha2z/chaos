#ifndef __WEB_HANDLER_H
#define __WEB_HANDLER_H

#include <iostream>

#include "CoHandler.h"

//#include "req_search.h"

class CWebHandler : public chaos::CoHandler {
	public:
		CWebHandler();
		virtual ~CWebHandler();
	public:
		virtual int handle(chaos::CoConnection *c); 
		virtual void close(chaos::CoConnection *c);
		virtual void ready(chaos::CoConnection *c);
		virtual void timeout(chaos::CoConnection *c);

		virtual void handlePkgWithTimeout(uint32_t pkgID, void *arg);
	private:
		int parseHead(chaos::CoConnection *c);

	public:
		uintptr_t       *mArgs;
		uint8_t			mResult;
};
#endif
