#include <iostream>

#include "CoCycle.h"
#include "CoChannel.h"
#include "pcl.h"
#include "mc_handler.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

extern int g_workernum;
extern int g_coronum;

CMcHandler::CMcHandler(){}
CMcHandler::~CMcHandler(){}

int CMcHandler::parseHead(CoConnection *c)
{
    mSimpleMsg.processReq(c);

	if(c->mRecvBuf->getSize() < (size_t)mSimpleMsg.getHeadLen())    return 0;

	if(mSimpleMsg.decodeHead(1) < 0) return -1;

	if(c->mRecvBuf->getSize() < (size_t)mSimpleMsg.getHead()->len)  return 0;
	return mSimpleMsg.getHead()->len;

}

int CMcHandler::handle(CoConnection *c) 
{
	//LOG(6, "to handle");
	int ret = 0;
	
	mSimpleMsg.clear();

	ret= parseHead(c);

	//c->logBuf(0);
	if(ret < 0)	{ LOG(2, "parse head fail[%d]", ret); return -1;}
	if(ret == 0)	return 0;

	if(mSimpleMsg.getHead()->innerCmd != CMD_MC_LOGIN) {
		LOG(6, "CMD[%d]", mSimpleMsg.getHead()->innerCmd);
		c->logBuf(6, 0);
	}

	CMcMsg		*msgBase = NULL;

	switch(mSimpleMsg.getHead()->innerCmd)
	{
		case CMD_MC_LOGIN:
			msgBase = &mMcLogin;
			break;
		case CMD_MC_TRANSFORM_MSG:
			msgBase = &mMcTransformMsg;
			break;
		case CMD_MC_SHIFT_ROUTE:
			msgBase = &mMcShiftRoute;
			break;
		case CMD_MC_PASS_MSG:
			msgBase = &mMcPass;
			break;
		
		case CMD_MC_QUERYTXSESSION:
			msgBase = &mMCGetTxsession;
			break;
			
		default:
			return -1;
	}

	ret = msgBase->processResp(c);

//	c->closeConn();
	if(ret < 0) return -1;

//	LOG(6, "finish[%d]", ret);
	return ret;
}

void CMcHandler::close(CoConnection *c) {
	LOG(6, "close[%d]", c->mFD);

	RobotContext	*context = g_context;

	if(context->mMcHelloStatus == MC_HELLO_STATUS_DOING) {
		((CoChannel *)(context->mChannel))->clearTimer((struct event *)context->mMcHelloEvent);
		context->mMcHelloStatus = MC_HELLO_STATUS_DONE;
	}
}

void CMcHandler::ready(CoConnection *c) {
	LOG(5, "msgcenter conn ready.");
	
	RobotContext *context = g_context;

	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = context->mMcHelloTime;

	((CoChannel *)(context->mChannel))->addTimer(
		(struct event *)context->mMcHelloEvent, 
		tv,
		CMcHandler::doHello, c);

	context->mMcHelloStatus = MC_HELLO_STATUS_DOING;
}

void CMcHandler::timeout(CoConnection *c) {
	LOG(5, "mc connect timeout");
	c->close();
}

void CMcHandler::handlePkgWithTimeout(uint32_t pkgID, void *arg) {
    LOG(5, "pkg handle timeout[%d]", pkgID);

	if(arg == NULL) {
		LOG(2, "pkg arg error");
		return ;
	}

	coroutine_t     co = (coroutine_t) arg;

	g_workernum--;
	g_coronum--;
	co_delete(co);

}

void CMcHandler::doHello(int fd, short events, void *arg)
{
	CoConnection *c = (CoConnection *)arg;
	if(c == NULL) return;

	RobotContext	*context = g_context;

	CMcHandler * handler = (CMcHandler *) c->mHandler;

	if(handler == NULL) return;

//	LOG(5, "msg center hello");

	CMcLogin	*mcLogin = &handler->mMcLogin;

	mcLogin->mServerType = context->mServerType;
	mcLogin->mServerNo = context->mServerNo;
	mcLogin->mLoadLevel = context->mLoadLevel;

	int ret = mcLogin->processReq(c);

	if(ret < 0) {
		LOG(2, "do hello fail[%d]", ret);
	}

	struct timeval tv;

	tv.tv_usec = 0;
	tv.tv_sec = context->mMcHelloTime;
	((CoChannel *)(context->mChannel))->addTimer(
		(struct event *)context->mMcHelloEvent,
		tv,
		CMcHandler::doHello, c);

}
