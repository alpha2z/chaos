#include <assert.h>
#include "CoData.h"
#include "CoChannel.h"
#include "business_notify_result.h"
#include "mc_shift_route.h"
#include "base64.h"
#include "code_converter.h"
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

int CBusinessNotifyResult::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = getHeadLen() 
		+ sizeof(mResult)
		+ sizeof(uint16_t) + mSystemID.length()
		+ sizeof(mFromServerType) + sizeof(mToServerType)
		+ sizeof(MC_END);

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_BUSINESS_NOTIFY_RESULT;
	head->seq = getSeq();

	if((offset = encodeHead()) < 0) return -1;
	offset += mConn->put(mResult);
	offset += mConn->put(mSystemID);
	offset += mConn->put(mFromServerType);
	offset += mConn->put(mToServerType);
	offset += mConn->put(MC_END);

	return offset;
}

int CBusinessNotifyResult::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) { 
		LOG(2, "decode fail[%d]", offset) ; 
		return -1;
	}
	
	offset += mConn->get(mSystemID);
	offset += mConn->get(mFromServerType);
	offset += mConn->get(mToServerType);
	offset += mConn->get(mSessionID);
	offset += mConn->get(mBusinessSetID);
	offset += mConn->get(mBusinessSetResult);
	offset += mConn->get(mBusinessSetResultContent);
	offset += mConn->get(mContent);

	uint8_t	end;
	offset += mConn->get(end);
	if(end != MC_END) { LOG(2, "end flag err[%d]", end); return -5; }

	return offset;
}

int CBusinessNotifyResult::processResp(CoConnection *c)
{
	mConn = c;

	int offset = 0;

	if((offset = encode()) < 0) { 
		LOG(2, "encode fail[%d]", offset); 
		return -1; 
	}

	mConn->logBuf(6, 1);

	if(mConn->sendData() < 0)  { LOG(2, "send data fail"); return -2;}

		//	mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, NULL);

	return 0;
}

int CBusinessNotifyResult::processReq(CoConnection *c)
{
	CoCycle *cycle = g_cycle;

	mConn = c;

	int offset = 0, ret = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 

		return -1; 
	}
	
//	mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);
	mResult = RESULT_SUC;
	if((ret = processResp(c)) < 0) {
		LOG(2, "process req[%d]", ret);
		return offset;
	}

	size_t beginPos = 0, endPos = 0;

	LOG(5, "shift route");
	CMcShiftRoute       shiftRoute;
	shiftRoute.mSystemID = mSystemID;
	shiftRoute.mSubCmd = CMcShiftRoute::SUB_CMD_REQ;
	shiftRoute.mType = CMcShiftRoute::SHIFT_TYPE_TO_MANUAL;
	shiftRoute.mOldServerType = SERVER_TYPE_ROBOT;
	shiftRoute.mNewServerType = SERVER_TYPE_KFIF;

	shiftRoute.mKFName = getField(mContent, DELIMITER, beginPos, endPos);

	beginPos = endPos;
	shiftRoute.mOpenID = getField(mContent, DELIMITER, beginPos, endPos);

	beginPos = endPos;
	string attrID = getField(mContent, DELIMITER, beginPos, endPos);

	shiftRoute.mAttrID = atoi(attrID.c_str());

	int outLen = shiftRoute.mOpenID.length() * 2 + 4;
	char * out = (char *)cycle->mPool->alloc(outLen + 1);

	assert(out != NULL);

	memset(out, 0, outLen + 1);

	Base64 base64;
	int len = base64.from64tobits(out, shiftRoute.mOpenID.data(), outLen);

	LOG(5, "outlen[%d, %d]", outLen, len);

	if(len < 0)	{ 
		LOG(2, "base64 convert fail[%d]", len); 
		cycle->mPool->free(out);          
		out = NULL;
		return offset; 
	}

	shiftRoute.mOpenID = "";
	shiftRoute.mOpenID.append(out, len);

	cycle->mPool->free(out);
	out = NULL;


	shiftRoute.mSessionID = mSessionID;
	shiftRoute.mBusinessSetID = mBusinessSetID;
	shiftRoute.mBusinessSetResult = mBusinessSetResult;

	CodeConverter cc = CodeConverter("utf-8","gb2312");
	static char buf[2048];

	memset(buf, 0, sizeof(buf));
	cc.convert((char *)mBusinessSetResultContent.c_str(), mBusinessSetResultContent.length(), buf, sizeof(buf));

	shiftRoute.mBusinessSetResultContent = buf;

	shiftRoute.mIMType = CMcShiftRoute::IM_TYPE_QQ;
	shiftRoute.mUserName = shiftRoute.mOpenID;
	shiftRoute.mCity = 0;

	if((ret = shiftRoute.processReq((CoConnection *)g_context->mMcConn)) < 0) {
		LOG(2, "shift route fail[%d]", ret);
		return offset;
	}


	return offset;
}

string  CBusinessNotifyResult::getField(string text, string delimiter, size_t beginPos, size_t &endPos)
{
	string ret = "";

	endPos = text.find(delimiter, beginPos);

	if(endPos == string::npos)    endPos = text.length();
	ret.assign(text, beginPos, endPos-beginPos);

	if(endPos != text.length()) endPos += delimiter.length();
	return ret;
}
