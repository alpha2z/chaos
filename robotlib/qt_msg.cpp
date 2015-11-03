#include <assert.h>
#include "CoData.h"
#include "CoChannel.h"
#include <sys/timeb.h>
#include "qt_msg.h"

#include "worker.h"
#include "pcl.h"
#include "solo_hexshow.h"
#include "memlog.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

int CQtMsg::encode()
{
	int offset = 0;

	CQtHead *head = getHead();
	head->len = getHeadLen() + sizeof(m_uCmd);

	if (m_uCmd == SUB_CMD_REQ_MSG) {
        head->len += sizeof(uint16_t) + m_strOpenID.length()
				  + sizeof(uint8_t)
				  + sizeof(uint16_t) + m_strSendTime.length()
				  + sizeof(uint16_t) + m_strContent.length();
	} else {
		head->len += sizeof(m_uResult)
				  + sizeof(uint16_t) + m_strOpenID.length();
	}

	if(m_uCmd == SUB_CMD_REQ_MSG)
		head->seq = getSeq();

	if((offset = encodeHead(CMD_QT_MSG)) < 0)
		return -1;

	offset += mConn->put(m_uCmd);
	if (m_uCmd == SUB_CMD_REQ_MSG) {
		offset += mConn->put(m_strOpenID);
		offset += mConn->put(m_uUserType);
		offset += mConn->put(m_strSendTime);
		offset += mConn->put(m_strContent);	 
	} else {
		offset += mConn->put(m_uResult);
		offset += mConn->put(m_strOpenID);
	}

	return offset;
}

int CQtMsg::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) 
		return -1;
	
	offset += mConn->get(m_uCmd);
	if(m_uCmd == SUB_CMD_REQ_MSG) {
		offset += mConn->get(m_strOpenID);
		offset += mConn->get(m_uUserType);
		offset += mConn->get(m_strSendTime);
		offset += mConn->get(m_strContent);
	} else {
		offset += mConn->get(m_uResult);
		offset += mConn->get(m_strOpenID);
	}

	return offset;
}

int CQtMsg::processReq(CoConnection *c)
{
	mConn = c;
	
	int offset = 0;

	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	mConn->logBuf(6, 1);

	if(mConn->sendData() < 0) {
		LOG(2, "send data fail");  
		return -2;
	}

	return 0;
}

int CQtMsg::processResp(CoConnection *c)
{
	// CoCycle *cycle = g_cycle;

	mConn = c;

	int ret = 0;
	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 		
		return -1; 
	}

	LOG(5, "subcmd[%d, %d]", m_uCmd, getHead()->seq);
	if(m_uCmd == SUB_CMD_REQ_MSG) {
		m_uCmd = SUB_CMD_RESP_MSG;
		m_uResult = 0;

		if((ret = processReq(c)) < 0) {
			LOG(2, "process req fail[%d]", ret); 
			return offset; 
		}

		// create_worker_t     * createWorker ;
		// createWorker = (create_worker_t *) g_context->mCreateWorker;
		
		// CWorker * worker = createWorker();

		// g_workernum++;

		// assert(worker != NULL);
		
		// worker->setContext(g_context);
		// worker->setCycle(cycle);

		// worker->mSystemID = mSystemID;
		// worker->mSessionID = mSessionID;
		// worker->mSessionTicket = mSessionTicket;
		// worker->mIMType = mIMType;
		// worker->mFromUserName = mFromUserName;
		// worker->mToUserName = mToUserName;
		// worker->mContent = mContent;
		// worker->mTXUin = mTXUin;
		// worker->mTXCityID = mTXCityID;
		// worker->mTXBitmapFlag = mTXBitmapFlag;

		// timeb bt;
		// ftime(&bt);

		// coroutine_t     co;
		// if((co = co_create(CWorker::work, (void *)worker, 0, g_context->mCoroStackSize)) == NULL) {
		// 	LOG(2, "coro create fail");
		// 	fprintf(stderr, "coro fatal:");
		// } else {
		// 	timeb et;
		// 	ftime(&et);
		// 	LOG(2, "cocreate time:%d", 1000 * et.time + et.millitm - 1000 * bt.time - bt.millitm );
		// 	g_coronum++;
		// 	co_call(co);
		// }
		return offset;
	}

	return offset;
}

void CQtMsg::setCmd(uint8_t cmd) {
	m_uCmd = cmd;
}

void CQtMsg::setOpenID(string* arrOpenID) {
	m_strOpenID = *arrOpenID;
}

void CQtMsg::setUserType(uint8_t uUserType) {
	m_uUserType = uUserType;
}

void CQtMsg::setTime(string *arrTime) {
	m_strSendTime = *arrTime;
}

void CQtMsg::setContent(string *pTemp) {
	m_strContent = *pTemp;
}


