#include "CoData.h"
#include "CoChannel.h"
#include "mc_getTxsession.h"
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

int CMcGetTxSession::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = 19 ;

	head->len +=  sizeof(uint16_t)  +mSystemID.length()
		+sizeof(uint16_t)  + mOpenid.length()
		+ sizeof(mImType) 
		+ sizeof(mForceRequest)
		+ sizeof(MC_END);

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_QUERYTXSESSION;
	
	head->seq = getSeq();

	if((offset = encodeHead()) < 0) return -1;

	offset += mConn->put(mSystemID);
	offset += mConn->put(mOpenid);
	offset += mConn->put(mImType);
	offset += mConn->put(mForceRequest);

	offset += mConn->put(MC_END);

	return offset;
}

int CMcGetTxSession::decode()
{
	int offset = 0;

// decode  == systemId + openId + imType + cResult + qqSessionId + qqSessionTicket

	if((offset = decodeHead(0)) < 0) return -1;
		
	offset += mConn->get(mSystemID);
	offset += mConn->get(mOpenid);	
	offset += mConn->get(mImType);	
	offset += mConn->get(mResult);
	offset += mConn->get(mTxSessionId);   
    offset += mConn->get(mSessionTicket);

    
	
	uint8_t     end;
	
	offset += mConn->get(end);
	
	if(end != MC_END) { LOG(2, "end flag err[%d] --mResult:[%d]--offset:[%d]", end,mResult,offset); return -5; }

	McMsgHead *head = getHead();

	if(head == NULL)	return -6;
	

	if(offset != head->len)	{ LOG(2, "package len err[%d,%d]", offset, head->len); return -7; }
	return offset;
}

int CMcGetTxSession::processReq(CoConnection *c)
{
	mConn = c;
	
	int offset = 0;

	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	mConn->logBuf(6, 1);

//	bool isRecv;
	mSubCmd = 0;
	
//	if(mSubCmd == SUB_CMD_REQ_MSG) isRecv = true;
//	else    isRecv = false;

	if(mConn->sendData() < 0)  { LOG(2, "send data fail"); return -2;}
	

    if(mSubCmd == SUB_CMD_REQ_MSG) {

        static int i = 0;
		i++;

		static time_t t = 0;
		time_t ct = time(NULL);
		if(ct - t > 10) {
			LOG(2, "-------------------->getTxsession :%d, %d, %d", i, g_workernum, g_coronum);
			t = ct;
		}
		
		mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, mArgs);
	}
	return 0;
}

int CMcGetTxSession::processResp(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	mConn = c;

	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 

		return -1; 
	}

	mSubCmd = 1;
	
	if(mSubCmd != SUB_CMD_REQ_MSG) {
		void * arg = mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);
		if(arg == NULL) {
			LOG(2, "pkg arg error");
			return offset;
		}

		coroutine_t     co = (coroutine_t) arg;

		uintptr_t *data = (uintptr_t *) cycle->mPool->alloc(sizeof(uintptr_t) * 3);

		assert(data != NULL);
        data[0] = (uintptr_t) &mResult;
        if( 0 == mResult ){
		    data[1] = (uintptr_t) &mTxSessionId;   	
			data[2] = (uintptr_t) &mSessionTicket ;
    	}else{
            data[1]=(uintptr_t) &mResult; 
			data[2]=(uintptr_t) &mResult;
    	}

		co_set_data(co, data);
		co_call(co);

		cycle->mPool->free(data);
		data = NULL;
          
	} else {
/*
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
	*/	}
//	}
	return offset;
}
