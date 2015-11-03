#ifndef __CHAOS_CYCLE_H
#define __CHAOS_CYCLE_H

#include <event.h>

#include "CoMmpool.h"
#include "CoConnection.h"
#include "CoLoger.h"
#include "CoArray.h"

#ifndef LOG
#define LOG(lv, fmt, args...) do{ \
	if(g_cycle != NULL && g_cycle->mLoger) {   \
		if(g_cycle->mLogLevel > lv) {  \
			g_cycle->mLoger->doLog("%s:%d(%s) - " fmt, __FILE__, __LINE__, __FUNCTION__ , ## args); \
		}\
	} \
} while(0)
#endif

#ifndef LOG_BUF
#define LOG_BUF(lv, buf, len) do{ \
	if(g_cycle != NULL && g_cycle->mLoger) { \
		if(g_cycle->mLogLevel > lv) {  \
			g_cycle->mLoger->doLogBuf(buf, len); \
		} \
	} \
} while(0)
#endif

namespace chaos {
	class CoCycle {
		public:
            CoCycle():mConns(NULL),
            mConnCount(0),
            mFreeConns(NULL),
            mFreeConnCount(0),
            mReadEvents(NULL),
            mWriteEvents(NULL),
            mTimeoutEvents(NULL),
            mClientRetrySec(0),
            mClientRetryTimeval(NULL),
            mClientRetryEvents(NULL),
            mBase(NULL),
            mAcceptEvent(NULL),
            mClientChannelList(NULL),
            mClientRetryList(NULL),
            mPool(NULL),
            mLoger(NULL){}
            ~CoCycle() {}

            void daemon();
        public:
            CoConnection		*mConns;
            uintptr_t			mConnCount;
            CoConnection		*mFreeConns;
            uintptr_t			mFreeConnCount;

            struct event		*mReadEvents;
            struct event		*mWriteEvents;
            struct event		*mTimeoutEvents;

            int			 		mClientRetrySec;
			struct timeval		*mClientRetryTimeval;

			struct event		*mClientRetryEvents;

			struct event_base	*mBase;
			struct event		*mAcceptEvent;

			CoArray				*mClientChannelList;
			CoArray				*mClientRetryList;

			CoMmpool			*mPool;
			CoLoger				*mLoger;
			int					mLogLevel;
	
	};
};

#endif
