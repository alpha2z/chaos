#include <new>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#include "CoChannel.h"
#include "CoCycle.h"
#include "CoConstants.h"

using namespace std;
using namespace chaos;

extern CoCycle *g_cycle;

int CoChannel::setNonBlock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if(flags < 0) return flags;

	flags |= O_NONBLOCK;
	if(fcntl( fd, F_SETFL, flags ) < 0) return -1;

	return 0;
}

void CoChannel::clientRetry(int fd, short events, void *arg)
{
	CoCycle *cycle = g_cycle;

	CoChannel	*channel = (CoChannel *)arg;

//	do_log(cycle->log, "check client channel");
	
	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = cycle->mClientRetrySec;

	CoArray *retryList = cycle->mClientRetryList;

	CoConnection * c = NULL;
	
	//cycle->mLoger->doLog("count[%d]", cycle->mClientRetryList->mCount);
	while(retryList->mCount > 0)
	{
		c = (CoConnection *)retryList->del(0);
		if(c == NULL)	continue;

		c->mRetryIndex = -1;

	//	do_log(cycle->log, "client retry[%s:%d]", ip, port);

		if(channel->createClientChannelWithConn(c) < 0) {
			cycle->mLoger->doLog("client retry fail");
			continue;
		}

	}

	evtimer_add(cycle->mClientRetryEvents, &tv);

}

intptr_t CoChannel::init(CoLoger *loger)
{
	CoCycle *cycle = g_cycle;
	cycle->mLoger = loger;

	uintptr_t i;
	//  struct event *rev, *wev;

	CoConnection    *next;
	CoConnection    *c;

	cycle->mConnCount = MAX_CONNECTION_NUM;

//	cycle->mPool = new CoMmpool();

/*	if(cycle->mPool->create(16384) < 0) {
		cout<<"create pool fail"<<endl;
		return -1;
	}
*/
	try {

		cycle->mPool = new CoMmpool();
//		cycle->mLoger->doLog("alloc:%d, %d", cycle->mPool->mAllocNum, cycle->mPool->mFreeNum);

		cycle->mClientChannelList = new CoArray(100);
		cycle->mClientRetryList = new CoArray(100);

		cycle->mConns = new CoConnection[cycle->mConnCount];

		c = cycle->mConns;

//cycle->mAcceptEvent = new struct event;

		cycle->mReadEvents =  new struct event[cycle->mConnCount];

		memset(cycle->mReadEvents, 0, sizeof(struct event) * cycle->mConnCount);

		cycle->mWriteEvents =  new struct event[cycle->mConnCount];

		memset(cycle->mWriteEvents, 0, sizeof(struct event) * cycle->mConnCount);

		cycle->mTimeoutEvents = new struct event[cycle->mConnCount];

		memset(cycle->mTimeoutEvents, 0, sizeof(struct event) * cycle->mConnCount);

		cycle->mClientRetrySec = 5;

		cycle->mClientRetryEvents = new struct event;

		i = cycle->mConnCount;
		next = NULL;

		do {
			i--;

			c[i].mNext = next;
			c[i].mRead = &cycle->mReadEvents[i];
			c[i].mWrite = &cycle->mWriteEvents[i];
			c[i].mTimeout = &cycle->mTimeoutEvents[i];
			c[i].mFD = (int) -1;
			c[i].mType = CoConnection::CONN_TYPE_UNKNOW;

			c->mClientIndex = 0;
			c->mRetryIndex = 0;

			c->mStatus = CoConnection::CONN_STATUS_READY;
			c->mIsReconnect = false;

			next = &c[i];
		}while(i);

	} catch (bad_alloc e) {
		cout<<"allocation failure"<<endl;
		return -2;
	}

//	cycle->mLoger->doLog("alloc:%d, %d", cycle->mPool->mAllocNum, cycle->mPool->mFreeNum);
	cycle->mFreeConns = next;
	cycle->mFreeConnCount = cycle->mConnCount;

	event_init();

	cycle->mBase = event_base_new();

	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = cycle->mClientRetrySec;
	
	evtimer_set(cycle->mClientRetryEvents, clientRetry, this);
	event_base_set(cycle->mBase, cycle->mClientRetryEvents);

	evtimer_add(cycle->mClientRetryEvents, &tv);

//	cycle->mLoger->doLog("alloc:%d, %d", cycle->mPool->mAllocNum, cycle->mPool->mFreeNum);
	return 0;
}

void CoChannel::addTimer(struct event *ev, struct timeval tv, void (*callback)(int, short, void *), void *arg)
{
	CoCycle *cycle = g_cycle;

	evtimer_set(ev, callback, arg);
	event_base_set(cycle->mBase, ev);

	evtimer_add(ev, &tv);
}

void CoChannel::clearTimer(struct event * ev)
{
	evtimer_del(ev);
}

int CoChannel::createSocket(CoConnection::ConnMode cm)
{
	CoCycle *cycle = g_cycle;            

	int s = -1;                          

	if(cm == CoConnection::CONN_MODE_TCP)
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		s= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (s < 0) {
		cycle->mLoger->doLog("socket");
		return -1;
	}

//	cycle->mLoger->doLog("create socket[%d]", s);

	if(setNonBlock(s) < 0) {
		cycle->mLoger->doLog("set nonblock fail");
		return -1;
	}

	int flags = 1;
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags)) < 0) {
		::close(s);
		return -1;
	}

	if(cm == CoConnection::CONN_MODE_TCP) {
		if(setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags)) < 0) {
			::close(s);
			return -1;
		}
	}
	return s;
}

int CoChannel::createServerChannel(char *IP, short port, 
		CoHandler *handler, CoConnection::ConnMode cm)
{
	CoCycle *cycle = g_cycle;

	if(IP == NULL)	return -1;

	int s = createSocket(cm);

	if (s < 0) {
		cycle->mLoger->doLog("socket");
		return -1;
	}

	struct sockaddr_in s_in;
	bzero(&s_in, sizeof(s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(port);
	struct in_addr in;
	inet_aton(IP, &in);

	s_in.sin_addr.s_addr = in.s_addr;

	cycle->mLoger->doLog("fd[%d], ip[%s], port[%d]", s,
			                inet_ntoa(s_in.sin_addr), ntohs(s_in.sin_port));

	if (bind(s, (struct sockaddr *) &s_in, sizeof(s_in)) < 0) {
		cycle->mLoger->doLog("bind[%d]", errno);
		return -1;
	}

	if(cm == CoConnection::CONN_MODE_TCP) {
		if (listen(s, 1024) < 0) {
			cycle->mLoger->doLog("listen");
			return -1;
		}
	}

//	struct event *ev = cycle->mAcceptEvent;
	struct event *ev = new struct event;
	uintptr_t * arg = new uintptr_t[3];
	
	arg[0] = (uintptr_t )handler;
	arg[1] = (uintptr_t )cm;
	arg[2] = (uintptr_t )ev;

	event_set(ev, s, EV_READ | EV_PERSIST, CoConnection::acceptCB, (void *)arg);
	event_base_set(cycle->mBase, ev);

	event_add(ev, NULL);

	return 0;
}

CoConnection *CoChannel::createClientChannel(char *IP, short port,
		        CoHandler *handler, CoConnection::ConnMode cm, bool isReconnect)
{
	CoCycle *cycle = g_cycle;

	if(IP == NULL)  return NULL;

	int s = createSocket(cm);

	if (s < 0) {
		cycle->mLoger->doLog("socket");
		return NULL;
	}

	CoConnection  *c;
	c = CoConnection::getConn(s);

	if (c == NULL) {
		cycle->mLoger->doLog("get connection fail");
		return NULL;
	}

	struct sockaddr_in s_in;
	bzero(&s_in, sizeof(s_in));
	s_in.sin_family = AF_INET;
	s_in.sin_port = htons(port);
	struct in_addr in;
	inet_aton(IP, &in);

	s_in.sin_addr.s_addr = in.s_addr;

	c->mFD = s;
	c->mAddr = s_in;
	c->mType    = CoConnection::CONN_TYPE_CLIENT;
	c->mHandler = handler;
	c->mConnMode = cm;
	c->mIsReconnect = isReconnect;
	
	if(c->mConnMode == CoConnection::CONN_MODE_TCP) {
		if (connect(c->mFD, (struct sockaddr *) &(c->mAddr), sizeof(c->mAddr)) == -1) {
            cycle->mLoger->doLog("connect err[%d] %s, %s:%d", errno, strerror(errno), IP, port);
			if(EINPROGRESS != errno) {
                perror("connect failed");
				cycle->mLoger->doLog("connect fail");
				return NULL;
			}
		}
	}

	event_set(c->mRead, c->mFD, EV_READ | EV_PERSIST, CoConnection::readCB, c);
	event_base_set(cycle->mBase, c->mRead);
	event_set(c->mWrite, c->mFD, EV_WRITE, CoConnection::writeCB, c);
	event_base_set(cycle->mBase, c->mWrite);

	if(c->mHandler->mWriteTimeoutSec > 0) {
		struct timeval tv;
		tv.tv_usec = 0;
		tv.tv_sec = c->mHandler->mWriteTimeoutSec;
		event_add(c->mWrite, &tv);
	} else {

		event_add(c->mWrite, NULL);
	}

	cycle->mClientChannelList->append(c);

	c->mClientIndex = cycle->mClientChannelList->mCount - 1;
	return c;
}

int CoChannel::createClientChannelWithConn(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	if(c->mStatus == CoConnection::CONN_STATUS_RETRYING
			|| c->mStatus == CoConnection::CONN_STATUS_TIMEOUT) {
		int s = createSocket(c->mConnMode);

	//	cycle->mLoger->doLog("socket[%d]", s);
		if (s < 0) {
			return -1;
		}

		c->mFD = s;
	}

	if(c->mConnMode == CoConnection::CONN_MODE_TCP) {
		struct in_addr in;
		in.s_addr = c->mAddr.sin_addr.s_addr;

		char *ip = inet_ntoa(in);
		if (connect(c->mFD, (struct sockaddr *) &(c->mAddr), sizeof(c->mAddr)) == -1) {
			cycle->mLoger->doLog("connect err[%d, %s], %s:%d", errno, strerror(errno), ip, ntohs(c->mAddr.sin_port));
			if(EINPROGRESS != errno) {
				cycle->mLoger->doLog("connect fail");
				return -2;
			}
		}
	}

	event_set(c->mRead, c->mFD, EV_READ | EV_PERSIST, CoConnection::readCB, c);
	event_base_set(cycle->mBase, c->mRead);
	event_set(c->mWrite, c->mFD, EV_WRITE, CoConnection::writeCB, c);
	event_base_set(cycle->mBase, c->mWrite);

	if(c->mHandler->mWriteTimeoutSec > 0) {
		struct timeval tv;
		tv.tv_usec = 0;
		tv.tv_sec = c->mHandler->mWriteTimeoutSec;
		event_add(c->mWrite, &tv);
	} else {

		event_add(c->mWrite, NULL);
	}

	cycle->mClientChannelList->append(c);

	c->mClientIndex = cycle->mClientChannelList->mCount - 1;
	return 0;
}

void CoChannel::runOnce()
{
	CoCycle *cycle = g_cycle;

	event_base_loop(cycle->mBase, EVLOOP_ONCE);
}
