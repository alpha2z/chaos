#include <iostream>

#include "CoCycle.h"
#include "web_handler.h"
#include "struct.h"
#include "pcl.h"
#include "api.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext * g_context;

CWebHandler::CWebHandler(){}
CWebHandler::~CWebHandler(){}

int CWebHandler::parseHead(CoConnection *c)
{
	//LOG_BUF(6, (char *)c->mRecvBuf, c->mRecvPos);

	string endFlag = "</" + WEB_API_XML_ITEM_STR + ">";

	u_char *end = c->mRecvBuf->find((u_char *)endFlag.c_str(), endFlag.length());

	if(end == NULL) {
		return 0;
	}
	
	char * beginFlag = "<?xml";
	u_char *begin = c->mRecvBuf->find((u_char *)beginFlag, strlen(beginFlag));
	
	if(begin == NULL) {
		return -1;
	}

	LOG(5, "begin[%d], end[%d]", begin, end);
	return end - begin + endFlag.length() + 1;
}

int CWebHandler::handle(CoConnection *c) 
{
//	LOG(6, "to handle");
	int ret = 0;
	
	ret= parseHead(c);

	//c->logBuf(0);
	if(ret < 0)	{ LOG(2, "parse head fail[%d]", ret); return -1;}
	if(ret == 0)	return 0;

	coroutine_t     co = (coroutine_t) mArgs;

	mResult = CApi::WEB_API_SUC;

	co_call(co);

	return ret;
}

void CWebHandler::close(CoConnection *c) {
	CoCycle *cycle = g_cycle;
	LOG(6, "close[%d]", c->mFD);
/*	
	CWebWebObjMng * sosoObjMng = getContext()->mWebsoObjMng;
	
	WebWebObject * os = sosoObjMng->get(c);
	if(os == NULL) {
		LOG(2, "so fatal");
		return;
	}
	sosoObjMng->remove(c);
	cycle->mPool->delObj(os);*/
}

void CWebHandler::timeout(CoConnection *c) {
	CoCycle *cycle = g_cycle;
	LOG(6, "timeout [%d]", c->mFD);

	close(c);

	coroutine_t     co = (coroutine_t) mArgs;

	mResult = CApi::WEB_API_FAIL;
	co_call(co);
}

void CWebHandler::ready(CoConnection *c) {
	int ret = 0;
	LOG(6, "web handler ready");
	coroutine_t     co = (coroutine_t) mArgs;

	mResult = CApi::WEB_API_SUC;
	co_call(co);
}

void CWebHandler::handlePkgWithTimeout(uint32_t pkgID, void *arg)
{
}
