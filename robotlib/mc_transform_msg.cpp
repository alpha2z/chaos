#include <assert.h>
#include "CoData.h"
#include "CoChannel.h"
#include <sys/timeb.h>
#include "mc_transform_msg.h"

#include "mc_handler.h"
#include "rich_msg.h"
#include "user_mng.h"
#include "worker.h"
#include "pcl.h"
#include "solo_hexshow.h"
#include "memlog.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

int g_workernum = 0;
int g_coronum = 0;

int CMcTransformMsg::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = getHeadLen() + sizeof(mSubCmd);

	if(mSubCmd == SUB_CMD_REQ_MSG) {
		head->len += sizeof(uint16_t) + mSystemID.length()
			+ sizeof(mSessionID) + sizeof(mSeq) 
			+ sizeof(uint16_t) + mSessionTicket.length()
			+ sizeof(mIMType) + sizeof(mSenderType)
			+ sizeof(uint16_t) + mFromUserName.length()
			+ sizeof(uint16_t) + mToUserName.length()
			+ sizeof(uint16_t) + mContent.length()
			+ sizeof(mTXCityID) + sizeof(mTXUin)
			+ sizeof(mTXBitmapFlag)
			+ sizeof(MC_END);
	} else {
		head->len += sizeof(mResult) 
			+ sizeof(uint16_t) + mSystemID.length()
			+ sizeof(mSeq)
			+ sizeof(mIMType) + sizeof(mSenderType)
			+ sizeof(MC_END);
	}

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_TRANSFORM_MSG;
	
	if(mSubCmd != SUB_CMD_RESP_MSG)  head->seq = getSeq();

	if((offset = encodeHead()) < 0) return -1;
	offset += mConn->put(mSubCmd);

	if(mSubCmd == SUB_CMD_REQ_MSG) {
		offset += mConn->put(mSystemID);
		offset += mConn->put(mSessionID);
		offset += mConn->put(mSeq);
		offset += mConn->put(mSessionTicket);
		offset += mConn->put(mIMType);
		offset += mConn->put(mSenderType);
		offset += mConn->put(mFromUserName);
		offset += mConn->put(mToUserName);


		offset += mConn->put(mContent);
		offset += mConn->put(mTXCityID);
		offset += mConn->put(mTXUin);
		offset += mConn->put(mTXBitmapFlag);
	} else {
		offset += mConn->put(mSystemID);
		offset += mConn->put(mResult);
		offset += mConn->put(mSeq);
		offset += mConn->put(mIMType);
		offset += mConn->put(mSenderType);
	}

	offset += mConn->put(MC_END);
	
	return offset;
}

int CMcTransformMsg::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) return -1;
	
	offset += mConn->get(mSubCmd);

	if(mSubCmd == SUB_CMD_REQ_MSG) {
		offset += mConn->get(mSystemID);
		offset += mConn->get(mSessionID);
		offset += mConn->get(mSeq);
		offset += mConn->get(mSessionTicket);

		offset += mConn->get(mIMType);
		offset += mConn->get(mSenderType);
		offset += mConn->get(mFromUserName);
		offset += mConn->get(mToUserName);
		offset += mConn->get(mContent);
		offset += mConn->get(mTXCityID);
		offset += mConn->get(mTXUin);
		offset += mConn->get(mTXBitmapFlag);
	} else {
		offset += mConn->get(mSystemID);
		offset += mConn->get(mResult);
		offset += mConn->get(mSeq);
		offset += mConn->get(mIMType);
		offset += mConn->get(mSenderType);
	}

	uint8_t     end;
	offset += mConn->get(end);
	if(end != MC_END) { LOG(2, "end flag err[%d]", end); return -5; }

	return offset;
}

int CMcTransformMsg::processReq(CoConnection *c)
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
	else	isRecv = false;
	
	if(mConn->sendData() < 0)  { LOG(2, "send data fail");  
		return -2;}

	if(mSubCmd == SUB_CMD_REQ_MSG) {
		LOG(5, "seq[%d]", getHead()->seq);
		mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, mArgs);
	}

	return 0;
}

int CMcTransformMsg::processResp(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	mConn = c;

	int ret = 0;
	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 
		
		return -1; 
	}

	LOG(5, "subcmd[%d, %d]", mSubCmd, getHead()->seq);
	if(mSubCmd != SUB_CMD_REQ_MSG) {
		void * arg = mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);
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

	} else {
		mSubCmd = SUB_CMD_RESP_MSG;
		mResult = 0;

		if((ret = processReq(c)) < 0) { LOG(2, "process req fail[%d]", ret); return offset; }

		//	CWorker * worker = (CWorker *) g_context->mWorker;
			create_worker_t     * createWorker ;
			createWorker = (create_worker_t *) g_context->mCreateWorker;
			
			CWorker * worker = createWorker();

			g_workernum++;

			assert(worker != NULL);
			
			worker->setContext(g_context);
			worker->setCycle(cycle);

			worker->mSystemID = mSystemID;
			worker->mSessionID = mSessionID;
			worker->mSessionTicket = mSessionTicket;
			worker->mIMType = mIMType;
			worker->mFromUserName = mFromUserName;
			worker->mToUserName = mToUserName;
			worker->mContent = mContent;
			worker->mTXUin = mTXUin;
			worker->mTXCityID = mTXCityID;
			worker->mTXBitmapFlag = mTXBitmapFlag;

			timeb bt;
			ftime(&bt);

			coroutine_t     co;
			if((co = co_create(CWorker::work, (void *)worker, 0, g_context->mCoroStackSize)) == NULL) {
				LOG(2, "coro create fail");
				fprintf(stderr, "coro fatal:");
			} else {
				timeb et;
				ftime(&et);
				LOG(2, "cocreate time:%d", 1000 * et.time + et.millitm - 1000 * bt.time - bt.millitm );
				g_coronum++;
				co_call(co);
			}

			return offset;
		//}
	
	}

	return offset;
}
