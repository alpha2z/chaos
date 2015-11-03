#include "CoData.h"
#include "CoChannel.h"
#include "mc_login.h"

#include "mc_handler.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext * g_context;

int CMcLogin::encode()
{
	int offset = 0;

	McMsgHead *head = getHead();
	head->len = getHeadLen() 
		+ sizeof(mServerType) + sizeof(mLoadLevel)
		+ sizeof(mServerNo) + sizeof(MC_END);

	head->ver = MC_VER;
	head->innerCmd = CMD_MC_LOGIN;
	head->seq = getSeq();

	if((offset = encodeHead()) < 0) return -1;
	offset += mConn->put(mServerType);
	offset += mConn->put(mLoadLevel);
	offset += mConn->put(mServerNo);
	offset += mConn->put(MC_END);

	return offset;
}

int CMcLogin::decode()
{
	int offset = 0;

	if((offset = decodeHead(0)) < 0) return -1;
	
	offset += mConn->get(mResult);

	if(mResult != MC_LOGIN_CODE_SUC
			&& mResult == MC_LOGIN_CODE_FAIL
			&& mResult == MC_LOGIN_CODE_REDIRECT) {
		LOG(2, "mc login result err[%d]", mResult);
		return -2;
	}

	if(mResult == MC_LOGIN_CODE_REDIRECT) {
		offset += mConn->get(mRedirectIP);
		offset += mConn->get(mRedirectPort);
	}

	uint8_t     end;
	offset += mConn->get(end);
	if(end != MC_END) { LOG(2, "end flag err[%d]", end); return -5; }

	return offset;
}

int CMcLogin::processReq(CoConnection *c)
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

//	mConn->logBuf(6, 1);

	if(mConn->sendData() < 0)  { LOG(2, "send data fail"); return -2;}

	mConn->mHandler->addPkgWithTimeoutEvent(getHead()->seq, NULL);
	return 0;
}

int CMcLogin::processResp(CoConnection *c)
{
	mConn = c;

	int offset = 0;

	if((offset = decode()) < 0) { 
		LOG(2, "decode fail[%d]", offset); 
		return -1; 
	}

	mConn->mHandler->clearPkgWithTimeoutEvent(getHead()->seq);

	RobotContext	*context = g_context;

//	LOG(5, "result[%d, %d]", mResult, c->mRead->ev_flags);
	if(mResult == MC_LOGIN_CODE_REDIRECT) {
		((CoConnection *)context->mMcConn)->close();
		CoConnection *mcConn = NULL;
		char * mcip = NULL;

		struct in_addr in;
		in.s_addr = mRedirectIP;

		mcip = inet_ntoa(in);
		
//		cout<<"mc:"<<mcip<<":"<<mRedirectPort<<endl;

		mcConn = ((CoChannel *) context->mChannel)->createClientChannel(mcip,
				mRedirectPort, (CoHandler *)(context->mMcHandler), 
				CoConnection::CONN_MODE_TCP, true);

		if(mcConn == NULL) {
			LOG(2, "redirct to msg center fail");
		}
	}
	return offset;
}
