#include <assert.h>
#include "CoData.h"
#include "CoChannel.h"
#include <sys/timeb.h>
#include "qt_opt_conv.h"

#include "worker.h"
#include "pcl.h"
#include "solo_hexshow.h"
#include "memlog.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

int CQtConversion::encode()
{
	int offset = 0;

	CQtHead *head = getHead();
	head->len = getHeadLen() + sizeof(m_uCmd);

    if (m_uCmd == SUB_CMD_REQ_MSG) {
        head->len += sizeof(m_uOpt) 
                   + sizeof(m_uConversionID)
                   + sizeof(m_uChannelID)
                   + sizeof(m_uCityID)
                   + sizeof(uint16_t) + m_strOpenID.length()
                   + sizeof(uint16_t) + m_strSendTime.length();
    } else {
        head->len += sizeof(m_uResult)
            + sizeof(uint16_t) 
            + m_strOpenID.length();
    }

    if(m_uCmd == SUB_CMD_REQ_MSG)
        head->seq = getSeq();

    if((offset = encodeHead(CMD_QT_CONV)) < 0)
        return -1;

    offset += mConn->put(m_uCmd);
    if (m_uCmd == SUB_CMD_REQ_MSG) {
        offset += mConn->put(m_uOpt);
        offset += mConn->put(m_uConversionID);
        offset += mConn->put(m_uChannelID);
        offset += mConn->put(m_uCityID);
        offset += mConn->put(m_strOpenID);
        offset += mConn->put(m_strSendTime);
    } else {
        offset += mConn->put(m_uResult);
        offset += mConn->put(m_strOpenID);
    }

    return offset;
}

int CQtConversion::decode()
{
    int offset = 0;

    if((offset = decodeHead(0)) < 0) 
        return -1;

    offset += mConn->get(m_uCmd);
    if(m_uCmd == SUB_CMD_REQ_MSG) {
        offset += mConn->get(m_uOpt);
        offset += mConn->get(m_uConversionID);
        offset += mConn->get(m_uChannelID);
        offset += mConn->get(m_uCityID);
        offset += mConn->get(m_strOpenID);
        offset += mConn->get(m_strSendTime);
    } else {
        offset += mConn->get(m_uResult);
        offset += mConn->get(m_strOpenID);
    }

    return offset;
}

int CQtConversion::processReq(CoConnection *c)
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

int CQtConversion::processResp(CoConnection *c)
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


void CQtConversion::setCmd(uint8_t cmd) {
	m_uCmd = cmd;
}

void CQtConversion::setOpenID(string* arrOpenID) {
	m_strOpenID = *arrOpenID;
}

void CQtConversion::setSendTime(string* time) {
    m_strSendTime = *time;
}
 
void CQtConversion::setOpt(uint8_t opt) {
	m_uOpt = opt;
}

void CQtConversion::setConversionID(uint32_t uConversionID) {
    m_uConversionID = uConversionID;
}

void CQtConversion::setChannelID(uint32_t id) {
    m_uChannelID = id;
}

void CQtConversion::setCityID(uint32_t uCityID) {
    m_uCityID = uCityID;
}





