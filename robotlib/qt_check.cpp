#include <iostream>
#include "CoCycle.h"
#include "CoData.h"
#include "qt_check.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext		*g_context;

CQtBase::CQtBase()
{
}

CQtBase::~CQtBase()
{
}

/**
*	type = REAL_READ_BUFF, real read buff, 
*	type = CLONE_READ_BUFF, clone buff, then read,
*/
int CQtBase::decodeHead(int type)
{
	int offset = 0;

	if(type == REAL_READ_BUFF) {
		offset += mConn->get(mHead.len);
		offset += mConn->get(mHead.cmd);
		offset += mConn->get(mHead.code);
		offset += mConn->get(mHead.seq);
	} else {
		CoBuffer buf;
		buf.clone(mConn->mRecvBuf);

		offset += buf.get(mHead.len);
		offset += buf.get(mHead.cmd);
		offset += buf.get(mHead.code);
		offset += buf.get(mHead.seq);
	}

	return offset;
}

int CQtBase::encodeHead(uint16_t cmd)
{
	int offset = 0;
	mHead.cmd = cmd;
	offset += mConn->put(mHead.len);
	offset += mConn->put(mHead.cmd);
	offset += mConn->put(mHead.code);
	offset += mConn->put(mHead.seq);
	
	return offset;
}

CQtHead *CQtBase::getHead()
{
	return &mHead;
}

void CQtBase::setHead(CQtHead *head)
{
	mHead = *head;
}

int CQtBase::getHeadLen()
{
	mHeadLen = sizeof(CQtHead);
	return mHeadLen;
}

void CQtBase::clear()
{
	mConn = NULL;
	mHeadLen = 0;

	memset(&mHead, 0, sizeof(CQtHead));
}

unsigned int CQtBase::getSeq() {
	RobotContext *context = g_context;

	if(context->mQtSeq >= robot::MAX_UINT32) {
		context->mQtSeq = 0;
	} else {
		++context->mQtSeq;
	}

	return context->mQtSeq;
}


