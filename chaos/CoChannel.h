#ifndef __CHAOS_CHANNEL_H
#define __CHAOS_CHANNEL_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "CoLoger.h"
#include "CoHandler.h"
#include "CoConnection.h"

namespace chaos {
	class CoChannel {
		public:
			CoChannel() {}
			~CoChannel() {}
			
			intptr_t init(CoLoger *loger);
			int createServerChannel(char *IP, short port, 
					CoHandler *handler, CoConnection::ConnMode cm);
			CoConnection *createClientChannel(char *IP, short port, 
					CoHandler *handler, CoConnection::ConnMode cm, bool isReconnect);

			void addTimer(struct event *ev, struct timeval tv, void(*callback)(int, short, void *), void *arg);
			void clearTimer(struct event * ev);

			void runOnce();

		private:
			int setNonBlock(int fd);
			static void clientRetry(int fd, short events, void *arg);

			int createSocket(CoConnection::ConnMode cm);
			int createClientChannelWithConn(CoConnection *c);

	};
};

#endif

