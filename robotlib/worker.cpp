#include <stdarg.h>
#include "CoData.h"
#include "CoChannel.h"
#include <sys/timeb.h>

#include "mc_transform_msg.h"

#include "worker.h"
#include "pcl.h"
#include "api.h"

using namespace chaos;
using namespace robot;

extern CoCycle * g_cycle;
extern RobotContext	* g_context;
extern int g_workernum;
extern int g_coronum;

CWorker::CWorker() {}
CWorker::~CWorker() {}

void CWorker::work(void *args)
{
	timeb bt;                                                                  
	ftime(&bt); 


	CWorker * worker = (CWorker *) args;
	worker->doWork(worker->mContent);

	timeb et;                                                                  
	ftime(&et); 

	LOG(2, "worktime:%d", et.time * 1000 + et.millitm - bt.time * 1000 - bt.millitm);
	destroy_worker_t * destroyWorker;
	destroyWorker = (destroy_worker_t *) g_context->mDestroyWorker;

	destroyWorker(worker);
	g_workernum--;
	g_coronum--;
//	co_exit();

}

void CWorker::welcome(void *args)
{
	CWorker * worker = (CWorker *) args;
	worker->doWelcome();

	destroy_worker_t * destroyWorker;
	destroyWorker = (destroy_worker_t *) g_context->mDestroyWorker;

	destroyWorker(worker);
	g_workernum--;
	g_coronum--;
//	co_exit();
}

void CWorker::pass(void *args)
{
	CWorker * worker = (CWorker *) args;
	worker->doPass();

	destroy_worker_t * destroyWorker;
	destroyWorker = (destroy_worker_t *) g_context->mDestroyWorker;

	destroyWorker(worker);
	g_workernum--;
	g_coronum--;
//	co_exit();
}
/*
CoConnection * CWorker::doWebConnect(std::string domain, short port)
{
	return mApi.webConnect(domain, port);
}
*/

int CWorker::sendMsg(string content)
{
	return mApi.sendMsg(mSystemID, mSessionID, mSessionTicket, mIMType,
			mToUserName, mFromUserName, content, mTXBitmapFlag);
}

int CWorker::shiftRoute(uint8_t serverType, uint16_t serverNo, string businessArgs,
		uint32_t cityID, uint32_t attrID, uint32_t ogzID,  string strContent,uint32_t txKfID , uint8_t leaveWordType,
		uint32_t txCityID, uint32_t txUin)
{
	RobotContext *context = getContext();
	return mApi.shiftRoute(mSystemID, context->mServerType, context->mServerNo,
			serverType, serverNo, mSessionID, mFromUserName, mIMType, businessArgs,
			cityID, attrID, ogzID, leaveWordType, txCityID, txUin, mTXBitmapFlag,txKfID,strContent);
}

int CWorker::dataChannelReq(string key, void * reqItem, void * respItem, bool isWaitResponse)
{
	return mApi.dataChannelReq(key, reqItem, respItem, isWaitResponse);
}

int CWorker::passMsg(uint8_t subCmd, uint32_t businessCmd, uint8_t businesType,
		string content, uint8_t result)
{
	return mApi.passMsg(mSystemID, mOldServerType, mOldServerNo,
			mNewServerType, mNewServerNo, mSessionID, content, result, mSeq, 
			subCmd, businessCmd, businesType);
}

int CWorker::replaceUrlVar(string &text, string systemID, string imqd,
		string province, string city, time_t time, string key, string openID, string imno)
{
	return mApi.replaceUrlVar(text, systemID, imqd, province, city, time, key, openID, imno);
}

void CWorker::setContext(RobotContext *context) 
{
	mContext = context;
}

RobotContext * CWorker::getContext()
{
	return mContext;
}

uintptr_t CWorker::getWorkerContext()
{
	RobotContext *context = getContext();
	
	return context->mWorkerContext;
}

void CWorker::setCycle(CoCycle *cycle)
{
	mCycle = cycle;
}

CoCycle * CWorker::getCycle()
{
	return mCycle;
}

string CWorker::genKey(uint32_t uin)
{
	return mApi.genKey(uin);
}

int  CWorker::getTxSession(std::string systemID, std::string openID,uint8_t imtype,uint8_t forceRquest,uint_least32_t & txsessionid,std::string & txSessionTicket ){
    return mApi.getTxSession(systemID, openID,imtype,forceRquest,txsessionid ,txSessionTicket ) ;
}

void CWorker::sendMsg2QT(struct _stMessage* message) {
    mApi.sendMsg2QT(message);
}

void CWorker::sendOpt2QT(struct _stConversion* conversion) {
    mApi.sendOpt2QT(conversion);
}
