#ifndef __MSG_CENTER_MSG_H
#define __MSG_CENTER_MSG_H

#include <string.h>
#include "CoCycle.h"
#include "CoLoger.h"
#include "CoConnection.h"

#include "struct.h"

class CMcMsg
{
	public:
		CMcMsg();
		virtual ~CMcMsg();

		virtual int encode() = 0;
		virtual int decode() = 0;

		McMsgHead *getHead();
		void setHead(McMsgHead *head);

		virtual int processReq(chaos::CoConnection *c) = 0;
		virtual int processResp(chaos::CoConnection *c) = 0;

		void clear();

		int decodeHead(int type = 0);
		int encodeHead();

		int getHeadLen();

		unsigned int getSeq();
	public:
		chaos::CoConnection *mConn;

		McMsgHead  mHead;
		int mHeadLen;

		uintptr_t       *mArgs;

};

class CMcSimpleMsg : public CMcMsg {
	public:
		CMcSimpleMsg() {}
		virtual ~CMcSimpleMsg() {}

		virtual int encode() { return 0; }
		virtual int decode() { return 0; }

		virtual int processReq(chaos::CoConnection *c) { mConn = c; return 0;}
		virtual int processResp(chaos::CoConnection *c) { mConn = c; return 0;}
};

#endif
