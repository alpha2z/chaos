#ifndef __CQT_CHECK_H__
#define __CQT_CHECK_H__

#include "struct.h"

#define REAL_READ_BUFF 0
#define CLONE_READ_BUFF 1

class CQtBase
{
	public:
		CQtBase();
		virtual ~CQtBase();

		virtual int encode() = 0;
		virtual int decode() = 0;

		CQtHead *getHead();
		void setHead(CQtHead *head);

		virtual int processReq(chaos::CoConnection *c) = 0;
		virtual int processResp(chaos::CoConnection *c) = 0;

		void clear();

		int decodeHead(int type = 0);
        int encodeHead(uint16_t);

		int getHeadLen();

		unsigned int getSeq();
	public:
		chaos::CoConnection *mConn;

		CQtHead  mHead;
		int mHeadLen;

		uintptr_t       *mArgs;

};

class CQtCheck : public CQtBase {
	public:
		CQtCheck() {}
		virtual ~CQtCheck() {}

		virtual int encode() { return 0; }
		virtual int decode() { return 0; }

		virtual int processReq(chaos::CoConnection *c) { mConn = c; return 0;}
		virtual int processResp(chaos::CoConnection *c) { mConn = c; return 0;}
};

#endif //__CQT_CHECK_H__

