#include "CoData.h"
#include "CoChannel.h"
#include "qt_sync.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext * g_context;

int CQtSync::encode()
{
	int offset = 0;

	CQtHead *head = getHead();
    head->len = getHeadLen() 
        + sizeof(mServerType) + sizeof(mLoadLevel)
        + sizeof(mServerNo);

	head->seq = getSeq();

	if((offset = encodeHead(CMD_QT_SYNC)) < 0) 
		return -1;
	offset += mConn->put(mServerType);
	offset += mConn->put(mLoadLevel);
	offset += mConn->put(mServerNo);

	return offset;
}

int CQtSync::decode()
{
	int offset = 0;

	if((offset = decodeHead(REAL_READ_BUFF)) < 0) 
		return -1;
	
	offset += mConn->get(mResult);

	if(mResult != 0) {
		LOG(2, "mc sync result err[%d]", mResult);
		return -2;
	}

	return offset;
}

int CQtSync::processReq(CoConnection *c)
{
	mConn = c;
	
	int offset = 0;

	mServerType = g_context->mServerType;
	mServerNo = g_context->mServerNo;
	mLoadLevel = g_context->mLoadLevel;
	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	if(mConn->sendData() < 0) {
		LOG(2, "send data fail");
		return -2;
	}

	return 0;
}

int CQtSync::processResp(CoConnection *c)
{
	mConn = c;

	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 
		return -1; 
	}

	return offset;
}


