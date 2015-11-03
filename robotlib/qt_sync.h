#ifndef __QT_SYNC_H__
#define __QT_SYNC_H__

#include "CoData.h"
#include "CoChannel.h"
#include "CoCycle.h"
#include "struct.h"
#include "qt_check.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext * g_context;

class CQtSync : public CQtBase {
    public:
        virtual int encode();
        virtual int decode();
        virtual int processReq(CoConnection *c);
        virtual int processResp(CoConnection *c);
    public:
        uint8_t			mServerType;
        uint8_t			mLoadLevel;
        uint16_t		mServerNo;

        uint8_t         mResult;
};


#endif //__QT_SYNC_H__
