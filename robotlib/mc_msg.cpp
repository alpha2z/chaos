#include <iostream>
#include "CoCycle.h"
#include "CoData.h"
#include "mc_msg.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext		*g_context;

CMcMsg::CMcMsg()
{
}

CMcMsg::~CMcMsg()
{
}

int CMcMsg::decodeHead(int type)
{
	int offset = 0;

	if(type == 0) {
		uint8_t		begin;
		offset += mConn->get(begin);

		if(begin != MC_BEGIN) return -1;
		offset += mConn->get(mHead.len);
		offset += mConn->get(mHead.ver);
		offset += mConn->get(mHead.cmd);
		offset += mConn->get(mHead.checkSum);
		offset += mConn->get(mHead.seq);
		offset += mConn->get(mHead.innerCmd);
	} else {
		CoBuffer buf;
		buf.clone(mConn->mRecvBuf);

		uint8_t     begin;
		offset += buf.get(begin);
		
		if(begin != MC_BEGIN) return -1;

		offset += buf.get(mHead.len);
		offset += buf.get(mHead.ver);
		offset += buf.get(mHead.cmd);
		offset += buf.get(mHead.checkSum);
		offset += buf.get(mHead.seq);
		offset += buf.get(mHead.innerCmd);

	}
	if(mHead.cmd != CMD_INNER_MAIN)	{ 
		LOG(2, "cmd[%d,%d]", mHead.cmd, CMD_INNER_MAIN) ; return -2; 
	}

	return offset;
}

int CMcMsg::encodeHead()
{
	int offset = 0;
	uint8_t		begin;
	begin = MC_BEGIN;
	
	mHead.cmd = CMD_INNER_MAIN;

	offset += mConn->put(begin);
	offset += mConn->put(mHead.len);
	offset += mConn->put(mHead.ver);
	offset += mConn->put(mHead.cmd);
	offset += mConn->put(mHead.checkSum);
	offset += mConn->put(mHead.seq);
	offset += mConn->put(mHead.innerCmd);
	
	return offset;
}

McMsgHead *CMcMsg::getHead()
{
	return &mHead;
}

void CMcMsg::setHead(McMsgHead *head)
{
	mHead = *head;
}

int CMcMsg::getHeadLen()
{
	mHeadLen = sizeof(uint8_t) + sizeof(McMsgHead);
	return mHeadLen;
}

void CMcMsg::clear()
{
	mConn = NULL;
	mHeadLen = 0;

	memset(&mHead, 0, sizeof(McMsgHead));


}

unsigned int CMcMsg::getSeq() {
	RobotContext *context = g_context;

	if(context->mSeq >= robot::MAX_UINT32) {
		context->mSeq = 0;
	} else {
		++context->mSeq;
	}

	return context->mSeq;
}
