#ifndef __CHAOS_HANDLER_H
#define __CHAOS_HANDLER_H

#include <stdint.h>
#include <map>

namespace chaos {

	class CoConnection;
	class CoHandler {
		public:
			CoHandler();
			virtual ~CoHandler();
			virtual int handle(CoConnection *c) = 0;

			virtual void close(CoConnection *c) = 0;

			virtual void ready(CoConnection *c) = 0;
			virtual void timeout(CoConnection *c) = 0;
			virtual void finish(CoConnection *c) = 0;

		public:
			virtual void handlePkgWithTimeout(uint32_t pkgID, void *arg) = 0;

			static void handlePkgWithTimeoutCB(int fd, short events, void *arg);

			int  addPkgWithTimeoutEvent(uint32_t pkgID, void *arg);
			void * clearPkgWithTimeoutEvent(uint32_t pkgID);

		public:
			int		mWriteTimeoutSec;
			int		mReadTimeoutSec;

			std::map<uint32_t, void *>	mPkgArgMap; 
	};

}

#endif
