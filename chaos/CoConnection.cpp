#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "CoMmpool.h"
#include "CoCycle.h"
#include "CoConstants.h"

using namespace chaos;

extern CoCycle	*g_cycle;

CoConnection::CoConnection()
{
    mRecvBuf = new  CoBuffer();
    mSendBuf = new  CoBuffer();
}

CoConnection::~CoConnection()
{
	if(mRecvBuf != NULL) {
		delete mRecvBuf;
		mRecvBuf = NULL;
	}

	if(mSendBuf != NULL) {
		delete mSendBuf;
		mSendBuf = NULL;
	}
}

CoConnection *CoConnection::getConn(int fd)
{
	CoConnection    *c;

	c = g_cycle->mFreeConns;

	if (c == NULL) {
		return NULL;
	}

//	g_cycle->mLoger->doLog("get connection");
	LOG(CO_LOG_DEBUG, "get connection");

	g_cycle->mFreeConns = (CoConnection *)c->mNext;
	g_cycle->mFreeConnCount--;

	c->mNext = NULL;
	memset(&c->mAddr, 0, sizeof(c->mAddr));
	c->mType = CONN_TYPE_UNKNOW;
	c->mHandler = NULL;
	c->mClientIndex = 0;
	c->mRetryIndex = 0;
	c->mStatus = CONN_STATUS_READY;
	c->mIsReconnect = false;
	return c;
}

void CoConnection::closeConn()
{
	int fd;

	CoCycle * cycle = g_cycle;

//	cycle->mLoger->doLog("close conn");
	LOG(CO_LOG_DEBUG, "close conn");

	if(mFD == -1) {
		return;
	}

	event_del(mRead);
	event_del(mWrite);

	mStatus = CONN_STATUS_CLOSED;
	
	mHandler->close(this);

	free();

	fd = mFD;
	mFD = (int) -1;

	if (::close(fd) == -1) {
		cycle->mLoger->doLog("close connection fail");
	}

}
void CoConnection::close()
{
	CoCycle * cycle = g_cycle;
	mSendBuf->reset();
	mRecvBuf->reset();

//	cycle->mLoger->doLog("close conn inner");
	if(mConnMode == CONN_MODE_UDP)	return ;

	int fd;

	if(mFD == -1) {
		return;
	}

	event_del(mRead);
	event_del(mWrite);

	mStatus = CONN_STATUS_CLOSED;
	
	mHandler->close(this);

	if(mType == CONN_TYPE_CLIENT && mIsReconnect) {
		addToRetryList();
	} else {
		free();
	}

	fd = mFD;
	mFD = (int) -1;

	LOG(CO_LOG_DEBUG, "close socket[%d]", fd);
	//cycle->mLoger->doLog("close socket[%d]", fd);
	if (::close(fd) == -1) {
		cycle->mLoger->doLog("close connection fail");
	}

}

void CoConnection::free()
{
	mNext = g_cycle->mFreeConns;
	g_cycle->mFreeConns = this;
	g_cycle->mFreeConnCount++;

}

void CoConnection::addEvent(CoConnection *c, short events) 
{
	struct timeval tv; 

//	CoCycle *cycle = g_cycle;
//	cycle->mLoger->doLog("add event[%d]", c->mFD);
	if(events & EV_WRITE) { 

//		event_set(c->mWrite, c->mFD, EV_WRITE, writeCB, c);
//		event_base_set(cycle->mBase, c->mWrite);
		if(c->mHandler->mWriteTimeoutSec > 0) { 
			tv.tv_usec = 0; 
			tv.tv_sec = c->mHandler->mWriteTimeoutSec; 
			event_add(c->mWrite, &tv); 
		} else { 
			event_add(c->mWrite, NULL); 
		} 
	} 

	if(events & EV_READ) { 
//		cycle->mLoger->doLog("mReadTimeoutSec[%d]", c->mHandler->mReadTimeoutSec);
	//	event_set(c->mRead, c->mFD, EV_READ, readCB, c);
	//	event_base_set(cycle->mBase, c->mRead);
		if(c->mHandler->mReadTimeoutSec > 0 ) {
			tv.tv_usec = 0; 
			tv.tv_sec = c->mHandler->mReadTimeoutSec; 
			event_add(c->mRead, &tv); 
		} else { 
			event_add(c->mRead, NULL); 
		}
	} 
}

void CoConnection::addToRetryList()
{
	CoCycle * cycle = g_cycle;
	
//	cycle->mLoger->doLog("addToRetryList:%d", i++);

	cycle->mClientChannelList->del(mClientIndex);
	mClientIndex = -1;

	cycle->mClientRetryList->append(this);

	mRecvBuf->reset();
	mSendBuf->reset();

	mStatus = CONN_STATUS_RETRYING;

	mRetryIndex = cycle->mClientRetryList->mCount - 1;


}

void CoConnection::readCB(int fd, short events, void *arg)
{
	CoCycle * cycle = g_cycle;

	CoConnection *c = (CoConnection *)arg;

//	cycle->mLoger->doLog("default read[%d]", fd);

	if(c == NULL)   return;

	CoHandler *handler = c->mHandler;
	//unsigned short port = ntohs(c->mAddr.sin_port);

	if (events & EV_TIMEOUT) {
		cycle->mLoger->doLog("read timeout[%d, %d]", c->mType, c->mStatus);

		c->mStatus = CONN_STATUS_TIMEOUT;
		handler->timeout(c);
		return;
	}

	ssize_t len = 0;

	c->mStatus = CONN_STATUS_ESTABLISH;

	len = c->mRecvBuf->read(fd);

	//cycle->mLoger->doLog("read[%d]", len);

	if(len == 0) {  //another side close
		cycle->mLoger->doLog("type[%d], status[%d]", c->mType, c->mStatus);
		c->close();
			
		return;
	}

	if(len == -1) {
		if(errno != EAGAIN && errno != EWOULDBLOCK) {
			cycle->mLoger->doLog("errno[%d]", errno);
			c->close();
			return;
		}

		addEvent(c, EV_READ);
		return;
	}

	struct in_addr in;
	in.s_addr = c->mAddr.sin_addr.s_addr;
	char *ip = inet_ntoa(in);

//	cycle->mLoger->doLog("type[%d], mode[%d], [%s:%d]", c->mType, c->mConnMode, ip, port);
	if(c->mConnMode == CONN_MODE_UDP) {
		c->mAddr = *(c->mRecvBuf->getClientAddr());
	}

	in.s_addr = c->mAddr.sin_addr.s_addr;

	ip = inet_ntoa(in);

//	cycle->mLoger->doLog("read cb [%s:%d]", ip, port);
	//  do_log_buf(cycle->log, (char *)c->mRecvBuf, c->mRecvPos);
	int ret = 0;

	do {
		ret = handler->handle(c);
		
		if(ret == 0) {
			break;
		}

		if(ret < 0) {
			cycle->mLoger->doLog("handle fail[%d]", ret);
			c->close();
			if(c->mConnMode == CONN_MODE_TCP) {
				return;
			}
		}

		if(c->mStatus == CONN_STATUS_CLOSED)	return;

//		cycle->mLoger->doLog("ret[%d], size[%d]", ret, c->mRecvBuf->getSize());
	} while(ret > 0 && c->mRecvBuf->getSize() > 0);

//	event_set(c->mRead, c->mFD, EV_READ, readCB, (void *)aa);
//	event_base_set(cycle->mBase, c->mRead);
	event_add(c->mRead, NULL);
}

void CoConnection::writeCB(int fd, short events, void *arg)
{
	CoCycle * cycle = g_cycle;
	CoConnection *c = (CoConnection *)arg;

	if(c == NULL)   return;

	struct in_addr in;
	in.s_addr = c->mAddr.sin_addr.s_addr;

	//char *ip = inet_ntoa(in);

//	cycle->mLoger->doLog("write cb [%s:%d, %d]", ip, ntohs(c->mAddr.sin_port), fd);

	CoHandler *handler = c->mHandler;

	if (events & EV_TIMEOUT) {
		cycle->mLoger->doLog("timeout");

		c->mStatus = CONN_STATUS_TIMEOUT;
		handler->timeout(c);
		return;
	}

	int error = 0;
	socklen_t socklen = sizeof(int);
	if(getsockopt(c->mFD, SOL_SOCKET, SO_ERROR, &error, &socklen) < 0) {
		cycle->mLoger->doLog("getsockopt fail[%d, %d]", c->mFD, fd);
		perror("err:");
		return;
	}

	if(error != 0) 	{
		c->close();
		
		cycle->mLoger->doLog("error[%d]", error);
		return;
	}

	if(c->mStatus == CONN_STATUS_READY || 
			c->mStatus == CONN_STATUS_RETRYING ||
			c->mStatus == CONN_STATUS_TIMEOUT) {
//		cycle->mLoger->doLog("ESTABLISH");
		c->mStatus = CONN_STATUS_ESTABLISH;
		
		addEvent(c, EV_WRITE);

		if(c->mConnMode == CONN_MODE_TCP) {
			if(c->mType == CONN_TYPE_CLIENT) {
				handler->ready(c);
			}

			event_add(c->mRead, NULL);
			return;
		}
	}

	if(c->mStatus == CONN_STATUS_ESTABLISH) {
		ssize_t len = 0;

		c->mSendBuf->setClientAddr(c->mAddr);
		len = c->mSendBuf->write(fd);
//		cycle->mLoger->doLog("write fd[%d], len[%d]", fd, len);

		if(len == -1) {
			if(errno == EAGAIN) {
				addEvent(c, EV_WRITE);
				return;
			}

			cycle->mLoger->doLog("error[%d, %d, %d]", c->mType, c->mStatus, errno);
			c->close();
			return;
		}

		if(len > 0) {
			if(c->mSendBuf->getSize() <= 0) {
				handler->finish(c);
			}
			if(c->mSendBuf->getSize() > 0) {
				addEvent(c, EV_WRITE);
			}
		}
	}
//	cycle->mLoger->doLog("w 3");
}

void CoConnection::acceptCB(int fd, short event, void *arg)
{
	CoCycle * cycle = g_cycle;

	CoConnection    *c = NULL;

	cycle->mLoger->doLog("default accept");

	struct sockaddr_in s_in;
	socklen_t len = sizeof(s_in);
	int clientFD;

	ConnMode cm = (ConnMode )((uintptr_t *)arg)[1];

//	cycle->mLoger->doLog("cm[%d, %d, %d]", cm, CONN_MODE_TCP, CONN_MODE_UDP);

	if(cm == CONN_MODE_TCP) {
		clientFD = accept(fd, (struct sockaddr *) &s_in, &len);
		if (clientFD < 0) {
			perror("accept");
			return;
		}
//		cycle->mLoger->doLog("accept[%d]", clientFD);
	} else {
		clientFD = fd;
	}

	c = getConn(clientFD);

	if (c == NULL) {
		perror("connection overflow");
		return ;
	}

	c->mType = CONN_TYPE_SERVER;
	c->mStatus = CONN_STATUS_ESTABLISH;

	c->mHandler = (CoHandler *) ((uintptr_t *)arg)[0];
	c->mHandler->ready(c);

	c->mConnMode = cm;

	c->mFD = clientFD;
	c->mAddr = s_in;

	event_set(c->mRead, clientFD, EV_READ | EV_PERSIST, readCB, c);
	event_base_set(cycle->mBase, c->mRead);

	event_set(c->mWrite, clientFD, EV_WRITE, writeCB, c);
	event_base_set(cycle->mBase, c->mWrite);

	printf("acceptadd[%d,%d]", (int) c->mRead, c->mRead->ev_flags);
	event_add(c->mRead, NULL);

//	cycle->mLoger->doLog("accept end");
}

ssize_t CoConnection::sendData()
{
	//CoCycle * cycle = g_cycle;

//	cycle->mLoger->doLog("type[%d, %d, %d]", mType, isRecv?0:1, mStatus);
	if(mStatus != CONN_STATUS_ESTABLISH)	return -1;

	addEvent(this, EV_WRITE);
/*
	if(isRecv && mType == CONN_TYPE_CLIENT) {
		addEvent(this, EV_READ);
	}*/
	return 0;
}

int CoConnection::put(char data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(char &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(int8_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(int8_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(uint8_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(uint8_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(int16_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(int16_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(uint16_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(uint16_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(int32_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(int32_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(uint32_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(uint32_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(int64_t data)
{
	return mSendBuf->put(data);
}

int CoConnection::get(int64_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(uint64_t data)
{
	return mSendBuf->put(data);
	
}

int CoConnection::get(uint64_t &data)
{
	return mRecvBuf->get(data);
}

int CoConnection::put(const void *data, size_t dataLen)
{
	return mSendBuf->put(data, dataLen);
}

int CoConnection::get(char *data, size_t dataLen)
{
	return mRecvBuf->get(data, dataLen);
}

int CoConnection::put(string &data, int prefixType)
{
	return mSendBuf->put(data, prefixType);
}

int CoConnection::get(string &data, int prefixType)
{
	return mRecvBuf->get(data, prefixType);
}

void CoConnection::logBuf(int level, int type)
{
	CoCycle * cycle = g_cycle;
	if(level < cycle->mLogLevel) {

		if(type == 0)
			cycle->mLoger->doLogBuf((char *)mRecvBuf->getData(), mRecvBuf->getSize());
		else
			cycle->mLoger->doLogBuf((char *)mSendBuf->getData(), mSendBuf->getSize());

	}
}
