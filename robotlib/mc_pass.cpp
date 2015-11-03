#include "CoData.h"
#include "CoChannel.h"
#include "mc_pass.h"
#include "mc_handler.h"
#include "worker.h"
#include "pcl.h"
#include <assert.h>

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

extern int g_workernum;
extern int g_coronum;

int CMcPass::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = getHeadLen() + sizeof(mSubCmd);

	if(mSubCmd != SUB_CMD_REQ_MSG) {
		head->len += sizeof(mResult);
	}

	uint16_t contentLen = sizeof(mBusinessCmd) + sizeof(mBusinessType)
		+ sizeof(uint16_t) + mContent.length();

	head->len += sizeof(uint16_t) + mSystemID.length()
		+ sizeof(mFromServerType) + sizeof(mFromServerNo)
		+ sizeof(mToServerType) + sizeof(mToServerNo)
		+ sizeof(mSessionID)
		+ sizeof(uint16_t)
		+ contentLen
		+ sizeof(MC_END);

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_PASS_MSG;
	
//	head->seq = getSeq();

	if((offset = encodeHead()) < 0) return -1;
	offset += mConn->put(mSubCmd);

	if(mSubCmd == SUB_CMD_RESP_MSG) {
		offset += mConn->put(mResult);
	}

	offset += mConn->put(mSystemID);
	offset += mConn->put(mFromServerType);
	offset += mConn->put(mFromServerNo);
	offset += mConn->put(mToServerType);
	offset += mConn->put(mToServerNo);
	offset += mConn->put(mSessionID);
	offset += mConn->put(contentLen);
	offset += mConn->put(mBusinessCmd);
	offset += mConn->put(mBusinessType);
	offset += mConn->put(mContent);
	offset += mConn->put(MC_END);

	return offset;
}

int CMcPass::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) return -1;
	
	offset += mConn->get(mSubCmd);
	
	if(mSubCmd == SUB_CMD_RESP_MSG) {   
		offset += mConn->get(mResult);
	}

	offset += mConn->get(mSystemID);
	offset += mConn->get(mFromServerType);
	offset += mConn->get(mFromServerNo);
	offset += mConn->get(mToServerType);
	offset += mConn->get(mToServerNo);
	offset += mConn->get(mSessionID);
	
	uint16_t contentLen ;
	offset += mConn->get(contentLen);
	offset += mConn->get(mBusinessCmd);
	offset += mConn->get(mBusinessType);
	offset += mConn->get(mContent);

	uint8_t     end;
	offset += mConn->get(end);
	if(end != MC_END) { LOG(2, "end flag err[%d]", end); return -5; }

	McMsgHead *head = getHead();

	if(head == NULL)	return -6;

	if(offset != head->len)	{ LOG(2, "package len err[%d,%d]", offset, head->len); return -7; }
	return offset;
}

int CMcPass::processReq(CoConnection *c)
{
	mConn = c;
	
	int offset = 0;

	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	mConn->logBuf(6, 1);

	bool isRecv;
	if(mSubCmd == SUB_CMD_REQ_MSG) isRecv = true;
	else    isRecv = false;

	if(mConn->sendData() < 0)  { LOG(2, "send data fail"); return -2;}
/*
	if(mSubCmd == SUB_CMD_REQ_MSG) {
		mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, mArgs);
	}*/
	return 0;
}

int CMcPass::processResp(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	mConn = c;

	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 

		return -1; 
	}

	LOG(5, "subcmd:%d, seq:%d, bcmd:%d, btype:%x", mSubCmd, getHead()->seq, mBusinessCmd, mBusinessType);
	LOG(6, "content:");
	LOG_BUF(6, (char *)mContent.data(), mContent.length());
	if(mSubCmd != SUB_CMD_REQ_MSG) {
/*		void * arg = mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);
		if(arg == NULL) {
			LOG(2, "pkg arg error");
			return offset;
		}

		coroutine_t     co = (coroutine_t) arg;

		uintptr_t *data = (uintptr_t *) cycle->mPool->alloc(sizeof(uintptr_t) * 2);

		assert(data != NULL);

		data[0] = (uintptr_t) &mSubCmd;
		data[1] = (uintptr_t) &mResult;

		co_set_data(co, data);
		co_call(co);

		cycle->mPool->free(data);
		data = NULL;
*/
	} else {

		create_worker_t     * createWorker ;
		createWorker = (create_worker_t *) g_context->mCreateWorker;

		CWorker * worker = createWorker();

		assert(worker != NULL);

		worker->setContext(g_context);
		worker->setCycle(cycle);

		worker->mSystemID = mSystemID;
		worker->mSessionID = mSessionID;
		worker->mSessionTicket = "";
		worker->mIMType = 0;
		worker->mFromUserName = "";
		worker->mToUserName = "";
		worker->mContent = mContent;
		worker->mOldServerType = mFromServerType;
		worker->mOldServerNo = mFromServerNo;
		worker->mNewServerType = mToServerType;
		worker->mNewServerNo = mToServerNo;
		worker->mSeq = getHead()->seq;
		worker->mPassBusinessCmd = mBusinessCmd;
		worker->mPassBusinessType = mBusinessType;

		g_workernum++;

		coroutine_t     co;
		if((co = co_create(CWorker::pass, (void *)worker, 0, g_context->mCoroStackSize)) == NULL) {
			LOG(2, "coro create fail");
			fprintf(stderr, "coro fatal:");
		} else {
			g_coronum++;

			co_call(co);
		}
	}
	return offset;
}
