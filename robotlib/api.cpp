#include <sstream>
#include <assert.h>
#include <stdarg.h>
#include "CoData.h"
#include "CoChannel.h"
#include "mc_transform_msg.h"
#include "qt_msg.h"
#include "qt_opt_conv.h"
#include "api.h"
#include "pcl.h"
#include "data_channel_mng.h"
#include "md5.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

extern int g_workernum;
extern int g_coronum;

CApi::CApi() {}
CApi::~CApi() {}

int CApi::sendMsg(string systemID, uint32_t sessionID, 
		string sessionTicket, uint8_t imtype,
		string	fromUserName, string toUserName, string content, uint32_t txBitmapFlag)
{
	mMcTransformMsg.mSubCmd = CMcTransformMsg::SUB_CMD_REQ_MSG;
	mMcTransformMsg.getHead()->seq = mMcTransformMsg.getSeq();
	mMcTransformMsg.mSystemID = systemID;
	mMcTransformMsg.mSessionID = sessionID;
	mMcTransformMsg.mSessionTicket = sessionTicket;
	mMcTransformMsg.mIMType = imtype;
	mMcTransformMsg.mSenderType = SENDER_TYPE_ROBOT;
	mMcTransformMsg.mFromUserName = fromUserName;
	mMcTransformMsg.mToUserName = toUserName;
	mMcTransformMsg.mContent = content;
	mMcTransformMsg.mTXBitmapFlag = txBitmapFlag;

	coroutine_t     co = co_current();

	mMcTransformMsg.mArgs = (uintptr_t *) co;

	int ret = 0;
	if((ret = mMcTransformMsg.processReq((CoConnection *)g_context->mMcConn)) < 0) {
		LOG(2, "mc transform msg fail[%d]", ret);
		return -1;
	}

	co_resume();

	uintptr_t *args = (uintptr_t *) co_get_data(co);

	uint8_t subCmd = *((uint8_t *)args[0]);
	uint8_t result = *((uint8_t *)args[1]);

	LOG(5, "co subcmd[%d], result[%d]", subCmd, result);
/*
	if(args != NULL) {
		cycle->mPool->free(args);
	}
	args = NULL;*/
	return result;
}


int CApi::shiftRoute(string systemID, uint8_t oldServerType, 
		uint16_t oldServerNo, uint8_t newServerType, 
		uint16_t newServerNo, uint32_t sessionID, 
		string openID, uint8_t imtype, string businessArgs,
		uint32_t cityID, uint32_t attrID, uint32_t ogzID,
		uint8_t leaveWordType, uint32_t txCityID, uint32_t txUin, uint32_t txBitmapFlag,uint32_t txKfID , string strContent)
{
	LOG(5, "api shift...");
	mMcShiftRoute.mSubCmd = CMcShiftRoute::SUB_CMD_REQ;
	mMcShiftRoute.getHead()->seq = mMcShiftRoute.getSeq();
	mMcShiftRoute.mSystemID = systemID;
	mMcShiftRoute.mType = 2;
	mMcShiftRoute.mOldServerType = oldServerType;
	mMcShiftRoute.mOldServerNo = oldServerNo;
	mMcShiftRoute.mNewServerType = newServerType;
	mMcShiftRoute.mNewServerNo = newServerNo;
	mMcShiftRoute.mSessionID = sessionID;
	mMcShiftRoute.mOpenID = openID;
	mMcShiftRoute.mBusinessSetID = 0;
	mMcShiftRoute.mBusinessSetResult = 0;
	mMcShiftRoute.mBusinessSetResultContent = businessArgs;
	mMcShiftRoute.mIMType = imtype;
	mMcShiftRoute.mUserName = "";
	mMcShiftRoute.mCity = cityID;
	mMcShiftRoute.mKFName = "";
	mMcShiftRoute.mAttrID = attrID;
	mMcShiftRoute.mOgzID = ogzID;
	mMcShiftRoute.mLeaveWordType = leaveWordType;
	mMcShiftRoute.mTXCityID = txCityID;
	mMcShiftRoute.mTXUin = txUin;
	mMcShiftRoute.mTXBitmapFlag = txBitmapFlag;
	mMcShiftRoute.mKfID = txKfID;
	mMcShiftRoute.mContent = strContent ;

	coroutine_t     co = co_current();

	mMcShiftRoute.mArgs = (uintptr_t *) co;

	int ret = 0;
	if((ret = mMcShiftRoute.processReq((CoConnection *)g_context->mMcConn)) < 0) {
		LOG(2, "mc shift route fail[%d]", ret);
		return -1;
	}

	co_resume();

	uintptr_t *args = (uintptr_t *) co_get_data(co);

	uint8_t result = *((uint8_t *)args[0]);

	LOG(5, "co result[%d]", result);
/*
	if(args != NULL) {
		cycle->mPool->free(args);
	}
	args = NULL;*/
	return result;
}

int CApi::dataChannelReq(string key, void * reqItem, void * respItem, bool isWaitResponse)
{
	CoCycle * cycle = g_cycle;
	CDataChannelMng	* dataChannelMng = (CDataChannelMng *) g_context->mDataChannelMngPtr;
	if(dataChannelMng == NULL)	return -1;

	map<string, DataQueue *>  *  reqMap = dataChannelMng->getReqMap();

	map<string, DataQueue *>::iterator iter;
	iter = reqMap->find(key);

	if(iter == reqMap->end())	{
		return -3;
	}

	DataQueue	* queue = iter->second;

	assert(queue != NULL);

	size_t reqItemSize = queue->itemSize - sizeof(uint32_t);

	LOG(7, "out buf");
	LOG_BUF(7, (char *)reqItem, reqItemSize);

	uint32_t seq = dataChannelMng->getSeq();
	int ret = 0;
	if((ret = dataChannelMng->put(key, reqItem, CDataChannelMng::DATA_CHANNEL_TYPE_REQ, seq)) < 0)	{
		LOG(2, "data channel put fail[%d]", ret);
		return -2;
	}

	if(isWaitResponse)	{
		DataChannelArg   * dataChannelArg = (DataChannelArg *) cycle->mPool->alloc(sizeof(DataChannelArg));

		if(dataChannelArg == NULL) {
			LOG(2, "data channel arg alloc fail");
			return -3;
		}

		coroutine_t     co = co_current();

		dataChannelMng->getArgMap()->insert(pair<uint32_t, void *>(seq, (void *) co));

		dataChannelArg->seq = seq;
		dataChannelArg->coptr = co;
		CoChannel * channel = (CoChannel *)g_context->mChannel;
		channel->addTimer(&(dataChannelArg->ev), g_context->mDataChannelTv, CApi::dataChannelTimeout, dataChannelArg);

		co_resume();

		channel->clearTimer((struct event *) &dataChannelArg->ev);

		map<uint32_t, void *>::iterator iter = dataChannelMng->getArgMap()->find(seq);
		if(iter != dataChannelMng->getArgMap()->end()) {
			dataChannelMng->getArgMap()->erase(iter);
		}

		cycle->mPool->free(dataChannelArg);
		dataChannelArg = NULL;

		uintptr_t *args = (uintptr_t *) co_get_data(co);

		size_t len = *((size_t *)args[0]);
		void * respItemRet = (void *)args[1];

		memcpy(respItem, respItemRet, len);
/*
		cycle->mPool->free(respItemRet);
		respItemRet = NULL;
		if(args != NULL) {
			cycle->mPool->free(args);
		}
		args = NULL;*/
	}

	return 0;

}

int CApi::passMsg(std::string systemID, uint8_t fromServerType, uint16_t fromServerNo,
		uint8_t toServerType, uint16_t toServerNo, uint32_t sessionID, 
		std::string content, uint8_t result, uint32_t seq,
		uint8_t subCmd, uint32_t businessCmd, uint8_t businessType)
{
	LOG(5, "pass msg");
	mMcPass.getHead()->seq = seq;
	mMcPass.mSubCmd = subCmd;
	mMcPass.mResult = result;
	mMcPass.mSystemID = systemID;
	mMcPass.mFromServerType = fromServerType;
	mMcPass.mFromServerNo = fromServerNo;
	mMcPass.mToServerType = toServerType;
	mMcPass.mToServerNo = toServerNo;
	mMcPass.mSessionID = sessionID;
	mMcPass.mContent = content;
	mMcPass.mBusinessCmd = businessCmd;
	mMcPass.mBusinessType = businessType;

	coroutine_t     co = co_current();

	mMcPass.mArgs = (uintptr_t *) co;

	int ret = 0;
	if((ret = mMcPass.processReq((CoConnection *)g_context->mMcConn)) < 0) {
		LOG(2, "mc pass msg fail[%d]", ret);
		return -1;
	}
/*
	co_resume();

	uintptr_t *args = (uintptr_t *) co_get_data(co);

	subCmd = *((uint8_t *)args[0]);
	result = *((uint8_t *)args[1]);

	LOG(5, "co subcmd[%d], result[%d]", subCmd, result);
	return result;*/
	return 0;
}

int CApi::replace(string &text, string oldstr, string newstr)
{
	do {
		if(text.find(oldstr) == string::npos) break;
		text.replace(text.find(oldstr), oldstr.length(), newstr);
	} while (1);

	return 0;

}

int CApi::replaceUrlVar(string &text, string systemID, string imqd, string province, 
		string city, time_t time, string openID, uint32_t uin, uint32_t cityID)
{
	stringstream ss;

	replace(text, URL_VAR_HOSTTEL, systemID);
	
	replace(text, URL_VAR_IMQD, imqd);
	replace(text, URL_VAR_PROVINCE, province);
	replace(text, URL_VAR_CITY, city);

	ss.str("");
	ss << time;
	
	replace(text, URL_VAR_TIME, ss.str());
	replace(text, URL_VAR_KEY, genKey(uin, time));
	replace(text, URL_VAR_KEY2, genKey(uin, time,cityID));
	replace(text, URL_VAR_OPENID, openID);

	ss.str("");
	ss << uin;
	replace(text, URL_VAR_QQ, ss.str());

	ss.str("");
	ss << cityID;
	replace(text, URL_VAR_CITYID, ss.str());

	return 0;
}
int CApi::replaceUrlVar(string &text, string systemID, string imqd, string province, 
		string city, time_t time, string key, string openID, string imno)
{
	stringstream ss;

	replace(text, URL_VAR_HOSTTEL, systemID);
	
	replace(text, URL_VAR_IMQD, imqd);
	replace(text, URL_VAR_PROVINCE, province);
	replace(text, URL_VAR_CITY, city);

	ss.str("");
	ss << time;
	
	replace(text, URL_VAR_TIME, ss.str());
	replace(text, URL_VAR_KEY, key);
	replace(text, URL_VAR_OPENID, openID);
	replace(text, URL_VAR_QQ, imno);
	return 0;
}

void CApi::dataChannelTimeout(int fd, short events, void *arg)
{
	LOG(2, "data channel timeout");
	
	CoCycle * cycle = g_cycle;
    CDataChannelMng * dataChannelMng = (CDataChannelMng *) g_context->mDataChannelMngPtr;
    assert(dataChannelMng != NULL);

	DataChannelArg * dataChannelArg = (DataChannelArg *) arg;

	assert(arg != NULL);


	((CoChannel *)(g_context->mChannel))->clearTimer((struct event *) &dataChannelArg->ev);

	coroutine_t co = (coroutine_t) dataChannelArg->coptr;

	map<uint32_t, void *>::iterator iter = dataChannelMng->getArgMap()->find(dataChannelArg->seq);
	if(iter != dataChannelMng->getArgMap()->end()) {
		dataChannelMng->getArgMap()->erase(iter);
	}

	cycle->mPool->free(dataChannelArg);
	dataChannelArg = NULL;

	g_coronum--;
	g_workernum--;
	co_delete(co);
}

string CApi::genKey(uint32_t uin)
{
	string key = "";

	stringstream    ss;
	ss << uin << time(NULL) << URLKEY;

	CMD5    md5;
	md5.GenerateMD5((unsigned char *)ss.str().c_str(), ss.str().length());

	key = md5.ToString();

	key = key.substr(key.length() - 10, 10);
	return key;
}   
string CApi::genKey(uint32_t uin, time_t time)
{
	string key = "";

	stringstream    ss;
	ss << uin << time << URLKEY;

	CMD5    md5;
	md5.GenerateMD5((unsigned char *)ss.str().c_str(), ss.str().length());

	key = md5.ToString();

	key = key.substr(key.length() - 10, 10);
	return key;
} 

string CApi::genKey(uint32_t uin, time_t time,uint32_t cityID)
{
	string key = "";

	stringstream    ss;
	ss << uin << cityID << time << URLKEY;

	CMD5    md5;
	md5.GenerateMD5((unsigned char *)ss.str().c_str(), ss.str().length());

	key = md5.ToString();

	key = key.substr(key.length() - 10, 10);
	return key;
} 

// ywy 2012-07-10 add

int  CApi::getTxSession(string systemID, string openID,uint8_t imtype,uint8_t forceRquest,uint_least32_t & txsessionid ,string & txSessionTicket){
	
	mMcGetTxsession.mSubCmd = 0;
	mMcGetTxsession.mSystemID=systemID;
	mMcGetTxsession.mOpenid = openID ;
	mMcGetTxsession.mImType = imtype ;
	mMcGetTxsession.mForceRequest = forceRquest ;

	coroutine_t     co = co_current();

	mMcGetTxsession.mArgs = (uintptr_t *) co;

	int ret = 0;
	if((ret = mMcGetTxsession.processReq((CoConnection *)g_context->mMcConn)) < 0) {
		LOG(2, "getTxSession fail[%d]", ret);
		return -1;
	}

	co_resume();
	uintptr_t *args = (uintptr_t *) co_get_data(co);
	if(args == NULL) {
        LOG(2, "api getTxSession args is null");
        return -3;
    }
	
    int result = -1;  
    result = *((uint8_t *)args[0]);  
    if(0 == result){
        txsessionid = *((uint32_t *)args[1]);
		txSessionTicket = *((string * )args[2]) ;	  
    } else {
        txsessionid = -1 ;
    }
 
	return result ;
}

void CApi::sendMsg2QT(struct _stMessage* message) {
    CQtMsg msg;
    msg.setCmd(message->uCmd);
    msg.setOpenID(&message->strOpenID);
    msg.setUserType(message->uUserType);
    msg.setTime(&message->strSendTime);
    msg.setContent(&message->strContent);
    int ret = 0;
    if((ret = msg.processReq((CoConnection *)g_context->mQtConn)) < 0) {
        LOG(2, "send msg to quantong failed :%d", ret);
        return;
    }
}

void CApi::sendOpt2QT(struct _stConversion* conversion) {
    CQtConversion pack;
    pack.setCmd(conversion->uCmd);
    pack.setConversionID(conversion->uConversionID);
    pack.setOpenID(&conversion->strOpenID);
    pack.setCityID(conversion->uCityID);
    pack.setChannelID(conversion->uChannnelID);
    pack.setOpt(conversion->uOpt);
    pack.setSendTime(&conversion->strTime);

    int ret = 0;
    if((ret = pack.processReq((CoConnection *)g_context->mQtConn)) < 0) {
        LOG(2, "send conversion operate to quantong failed :%d", ret);
        return;
    }
}

/*
CoConnection * CApi::webConnect(string domain, short port)
{
	CoCycle *cycle = g_cycle;

	CoConnection * conn = NULL;

	CWebHandler *handler = (CWebHandler *) cycle->mPool->newObj<CWebHandler>();

	assert(handler != NULL);

	handler->mWriteTimeoutSec = 5;
	handler->mReadTimeoutSec = 5;

	coroutine_t     co = co_current();

	handler->mArgs = (uintptr_t *) co;

	char * ip = CoData::getIPByDomain((char *)domain.c_str());

	conn = ((CoChannel *)(g_context->mChannel))->createClientChannel(ip,
			port, handler,
			CoConnection::CONN_MODE_TCP, false);

	if(conn == NULL)
	{
		LOG(2, "create channle fail");
		return NULL;
	}

	co_resume();

	LOG(5, "co result[%d]", handler->mResult);

	if(handler->mResult != WEB_API_SUC) {
		conn = NULL;
	}
	
	return conn;
}
*/
