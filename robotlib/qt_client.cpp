#include <iostream>

#include "CoCycle.h"
#include "CoChannel.h"
#include "pcl.h"
#include "qt_client.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

extern int g_workernum;
extern int g_coronum;

CQtClient::CQtClient(){


}
CQtClient::~CQtClient(){


}

int CQtClient::parseHead(CoConnection *c)
{
    mCheck.processReq(c);

	if(c->mRecvBuf->getSize() < (size_t)mCheck.getHeadLen())  
		return 0;

	if(mCheck.decodeHead(1) < 0) 
		return -1;

	if(c->mRecvBuf->getSize() < (size_t)mCheck.getHead()->len) 
		return 0;
	
	return mCheck.getHead()->len;

}

int CQtClient::handle(CoConnection *c) 
{
	int ret = 0;	
	mCheck.clear();

	ret= parseHead(c);
	if (ret < 0) {
		LOG(2, "parse head fail[%d]", ret);
		return -1;
	}

	if (ret == 0)
		return 0;

	CQtBase *msgBase = NULL;
	switch(mCheck.getHead()->cmd)
	{
		case CMD_QT_SYNC:
			msgBase = &mQtSync;
			break;
		case CMD_QT_MSG:
			msgBase = &mQtMsg;
			break;
		default:
			return -1;
	}

	ret = msgBase->processResp(c);

	if(ret < 0) 
		return -1;

	return ret;
}

void CQtClient::close(CoConnection *c) {
	LOG(6, "close[%d]", c->mFD);

	RobotContext	*context = g_context;

	if(context->mQtHelloStatus == QT_HELLO_STATUS_DOING) {
		((CoChannel *)(context->mChannel))->clearTimer((struct event *)context->mQtHelloEvent);
		context->mQtHelloStatus = QT_HELLO_STATUS_DONE;
	}
}

void CQtClient::ready(CoConnection *c) {
	LOG(5, "qt conn ready.");
	
	RobotContext *context = g_context;

	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = context->mMcHelloTime;

	((CoChannel *)(context->mChannel))->addTimer(
		(struct event *)context->mQtHelloEvent, 
		tv,
		CQtClient::doHello, c);

	context->mQtHelloStatus = QT_HELLO_STATUS_DOING;
}

void CQtClient::timeout(CoConnection *c) {
	LOG(5, "timeout");

	c->close();
}

void CQtClient::handlePkgWithTimeout(uint32_t pkgID, void *arg) {
    LOG(5, "pkg timeout[%d]", pkgID);

	if(arg == NULL) {
		LOG(2, "pkg arg error");
		return ;
	}

	coroutine_t co = (coroutine_t) arg;

	g_workernum--;
	g_coronum--;
	co_delete(co);

}

void CQtClient::doHello(int fd, short events, void *arg)
{
	CoConnection *c = (CoConnection *)arg;
	if(c == NULL) return;

	RobotContext	*context = g_context;

	CQtClient * handler = (CQtClient *) c->mHandler;

	if (handler == NULL) 
		return;

	CQtSync *qtSync = &handler->mQtSync;
	qtSync->mServerType = context->mServerType;
	qtSync->mServerNo = context->mServerNo;
	qtSync->mLoadLevel = context->mLoadLevel;

	int ret = qtSync->processReq(c);

	if(ret < 0) {
		LOG(2, "do hello fail[%d]", ret);
	}

	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = context->mMcHelloTime;
	((CoChannel *)(context->mChannel))->addTimer(
		(struct event *)context->mQtHelloEvent,
		tv,	CQtClient::doHello, c);
}



