#include <assert.h>
#include "CoData.h"
#include "CoChannel.h"
#include "base64.h"

#include "mc_shift_route.h"

#include "rich_msg.h"

//#include "view_mng.h"
#include "mc_handler.h"

#include "mc_transform_msg.h"
#include "pcl.h"
#include "worker.h"

using namespace chaos;
using namespace robot;

extern int g_workernum;
extern int g_coronum;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

int CMcShiftRoute::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = getHeadLen() + sizeof(mSubCmd)
		+ sizeof(uint16_t) + mSystemID.length();

	if(mSubCmd == SUB_CMD_REQ) { 
		head->len += sizeof(mType)
			+ sizeof(mOldServerType) + sizeof(mOldServerNo)
			+ sizeof(mNewServerType) + sizeof(mNewServerNo)
			+ sizeof(mSessionID) 
			+ sizeof(uint16_t) + mOpenID.length()
			+ sizeof(uint16_t) + mUserNick.length()
			+ sizeof(mGender)
			+ sizeof(mBirthday)
			+ sizeof(mBusinessSetID)
			+ sizeof(mBusinessSetResult)
			+ sizeof(uint16_t) + mBusinessSetResultContent.length()
			+ sizeof(mIMType)
			+ sizeof(uint16_t) + mUserName.length()
			+ sizeof(mCity)
			+ sizeof(uint16_t) + mKFName.length()
			+ sizeof(mAttrID)
			+ sizeof(mOgzID)
			+ sizeof(mLeaveWordType)
			+ sizeof(mTXCityID)
			+ sizeof(mTXUin)
			+ sizeof(mTXBitmapFlag)
			+ sizeof(mKfID)                           //ywy add 2012-10-29
			+ sizeof(uint16_t) + mContent.length()    //ywy add 2012-10-29
			+ sizeof(MC_END);
	} else {
		head->len += sizeof(mResult)
			+ sizeof(mType)
			+ sizeof(mOldServerType) + sizeof(mOldServerNo)
			+ sizeof(mNewServerType) + sizeof(mNewServerNo)
			+ sizeof(mSessionID)     
			+ sizeof(uint16_t) + mOpenID.length()
			+ sizeof(mNewSessionID)
			+ sizeof(MC_END);
	}

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_SHIFT_ROUTE;
	
	if((offset = encodeHead()) < 0) return -1;
	offset += mConn->put(mSubCmd);
	offset += mConn->put(mSystemID);
	
	if(mSubCmd == SUB_CMD_REQ) {
		offset += mConn->put(mType);

		offset += mConn->put(mOldServerType);
		offset += mConn->put(mOldServerNo);
		offset += mConn->put(mNewServerType);
		offset += mConn->put(mNewServerNo);
		offset += mConn->put(mSessionID);
		offset += mConn->put(mOpenID);
		offset += mConn->put(mUserNick);
		offset += mConn->put(mGender);
		offset += mConn->put(mBirthday);
		offset += mConn->put(mBusinessSetID);
		offset += mConn->put(mBusinessSetResult);
		offset += mConn->put(mBusinessSetResultContent);
		offset += mConn->put(mIMType);
		offset += mConn->put(mUserName);
		offset += mConn->put(mCity);
		offset += mConn->put(mKFName);
		offset += mConn->put(mAttrID);
		offset += mConn->put(mOgzID);
		offset += mConn->put(mLeaveWordType);
		offset += mConn->put(mTXCityID);
		offset += mConn->put(mTXUin);
		offset += mConn->put(mTXBitmapFlag);
		offset += mConn->put(mKfID);
		offset += mConn->put(mContent);
	} else {
		offset += mConn->put(mResult);
		offset += mConn->put(mType);

		offset += mConn->put(mOldServerType);
		offset += mConn->put(mOldServerNo);
		offset += mConn->put(mNewServerType);
		offset += mConn->put(mNewServerNo);
		offset += mConn->put(mSessionID);
		offset += mConn->put(mOpenID);
		offset += mConn->put(mNewSessionID);
	}
	offset += mConn->put(MC_END);

	return offset;
}

int CMcShiftRoute::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) return -1;
	
	offset += mConn->get(mSubCmd);
	offset += mConn->get(mSystemID);

	if(mSubCmd == SUB_CMD_RESP) {
		offset += mConn->get(mResult);

		offset += mConn->get(mType);
		offset += mConn->get(mOldServerType);
		offset += mConn->get(mOldServerNo);
		offset += mConn->get(mNewServerType);
		offset += mConn->get(mNewServerNo);
		offset += mConn->get(mSessionID);
		offset += mConn->get(mOpenID);

		if(offset == getHead()->len - sizeof(mNewSessionID) - 1)
			offset += mConn->get(mNewSessionID);
	} else {
		offset += mConn->get(mType);
		offset += mConn->get(mOldServerType);
		offset += mConn->get(mOldServerNo);
		offset += mConn->get(mNewServerType);
		offset += mConn->get(mNewServerNo);
		offset += mConn->get(mSessionID);
		offset += mConn->get(mOpenID);
		offset += mConn->get(mUserNick);
		offset += mConn->get(mGender);
		offset += mConn->get(mBirthday);
		offset += mConn->get(mBusinessSetID);
		offset += mConn->get(mBusinessSetResult);
		offset += mConn->get(mBusinessSetResultContent);
		offset += mConn->get(mIMType);
		offset += mConn->get(mUserName);
		offset += mConn->get(mCity);
		offset += mConn->get(mKFName);
		offset += mConn->get(mAttrID);
		offset += mConn->get(mOgzID);
		offset += mConn->get(mLeaveWordType);
		offset += mConn->get(mTXCityID);
		offset += mConn->get(mTXUin);
		offset += mConn->get(mTXBitmapFlag);
		offset += mConn->get(mKfID);   // ywy 2012-11-1 add kfid
		offset += mConn->get(mContent);   // ywy 2012-11-1 add strContent
	}

	uint8_t	end;
	offset += mConn->get(end);
	if(end != MC_END) { LOG(2, "end flag err[%d]", end); return -5; }

	return offset;
}

int CMcShiftRoute::processReq(CoConnection *c)
{
	mConn = c;
	
	int offset = 0;

	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	LOG(5, "shift route");
	mConn->logBuf(6, 1);

	if(mConn->sendData() < 0)  { LOG(2, "send data fail"); return -2;}

	if(mSubCmd == SUB_CMD_REQ) {

		static int i = 0;
		i++;

		static time_t t = 0;
		time_t ct = time(NULL);
		if(ct - t > 10) {
			LOG(2, "shift num:%d, %d, %d", i, g_workernum, g_coronum);
			t = ct;
		}

		mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, mArgs);
	}
	return 0;
}

int CMcShiftRoute::processResp(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	mConn = c;

	int offset = 0, ret = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 
		
		return -1; 
	}

	LOG(5, "subcmd[%d], type[%d], seq[%d]", mSubCmd, mType, getHead()->seq);

	if(mSubCmd == SUB_CMD_REQ) {
		mResult = RESULT_SUC;
		mSubCmd = SUB_CMD_RESP;
		if((ret = processReq(c)) < 0) {
			LOG(2, "process req fail[%d]", ret);
			return offset;
		}

		//CWorker *worker = (CWorker *) g_context->mWorker;

		create_worker_t     * createWorker ;
		createWorker = (create_worker_t *) g_context->mCreateWorker;

		CWorker * worker = createWorker();

		assert(worker != NULL);

		worker->setContext(g_context);
		worker->setCycle(cycle);

		worker->mSystemID = mSystemID;
		worker->mSessionID = mSessionID;
		worker->mOldServerType = mOldServerType;
		worker->mOldServerNo = mOldServerNo;
		worker->mNewServerType = mNewServerType;
		worker->mNewServerNo = mNewServerNo;
		worker->mFromUserName = mOpenID;
		worker->mIMType = mIMType;
		worker->mCityID = mCity;
		worker->mOgzID = mOgzID;
		worker->mAttrID = mAttrID;
		worker->mKFName = mKFName;
		worker->mBusinessSetID = mBusinessSetID;
		worker->mBusinessArgs = mBusinessSetResultContent;
		worker->mLeaveWordType = mLeaveWordType;
		worker->mTXCityID = mTXCityID;
		worker->mTXUin = mTXUin;
		worker->mTXBitmapFlag = mTXBitmapFlag;
		worker->mKfID= mKfID ;
		worker->mRouteContext = mContent ;
		

		g_workernum++;

		coroutine_t     co;
		if((co = co_create(CWorker::welcome, (void *) worker, 0, g_context->mCoroStackSize)) == NULL) {
			LOG(2, "coro create fail");
			fprintf(stderr, "coro fatal:");
		} else {
			g_coronum++;
			co_call(co);
		}

		return offset;

	} else {
		void * arg = mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);
		if(arg == NULL) {
			LOG(2, "pkg arg error");
			return offset;
		}

		coroutine_t     co = (coroutine_t) arg;

		uintptr_t *data = (uintptr_t *) cycle->mPool->alloc(sizeof(uintptr_t));

		assert(data != NULL);

		data[0] = (uintptr_t) &mResult;

		co_set_data(co, (void *)data);
		co_call(co);

		cycle->mPool->free(data);
		data = NULL;

	}
	return offset;
}
