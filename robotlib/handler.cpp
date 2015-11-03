#include <iostream>

#include "CoCycle.h"
#include "CoChannel.h"

#include "handler.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

CHandler::CHandler(){}
CHandler::~CHandler(){}

int CHandler::parseHead(CoConnection *c)
{
    mSimpleMsg.processReq(c);

	if(c->mRecvBuf->getSize() < (size_t)mSimpleMsg.getHeadLen())    return 0;

	if(mSimpleMsg.decodeHead(1) < 0) return -1;

	if(c->mRecvBuf->getSize() < (size_t)mSimpleMsg.getHead()->len)  return 0;

	return mSimpleMsg.getHead()->len;

}

int CHandler::handle(CoConnection *c) 
{
	//LOG(6, "to handle");
	int ret = 0;
	
	mSimpleMsg.clear();

	ret= parseHead(c);

	//c->logBuf(0);
	if(ret < 0)	{ LOG(2, "parse head fail[%d]", ret); return -1;}
	if(ret == 0)	return 0;

	LOG(6, "CMD[%d]", mSimpleMsg.getHead()->innerCmd);
	c->logBuf(6, 0);

	CMcMsg		*msgBase = NULL;

	switch(mSimpleMsg.getHead()->innerCmd)
	{
		case CMD_MC_BUSINESS_NOTIFY_RESULT:
			msgBase = &mBusinessNotifyResult;
			break;
		default:
			return -1;
	}

	ret = msgBase->processReq(c);

//	c->closeConn();
	if(ret < 0) return -1;

//	LOG(6, "finish[%d]", ret);
	return ret;
}

void CHandler::close(CoConnection *c) {
	LOG(6, "close[%d]", c->mFD);

}

void CHandler::ready(CoConnection *c) {
	LOG(5, "conn1[%d,%d, %d, %d]", c, c->mRead, c->mWrite, c->mRead->ev_flags);
	
}

void CHandler::timeout(CoConnection *c) {
	LOG(5, "timeout");

}

void CHandler::handlePkgWithTimeout(uint32_t pkgID, void *arg) {
    LOG(5, "pkg timeout[%d]", pkgID);
}
