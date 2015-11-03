#ifndef __ROBOT_MYWORKER_H
#define __ROBOT_MYWORKER_H

#include "robotlib/worker.h"
#include "worker_struct.h"
#include "user_mng.h"

struct PassFriend {
    string          mSessionTicket;
    uint8_t		    mCmdType;
    uint16_t        mSeq;
    uint32_t        mUin;
    string			mData;
};

class CMyWorker : public CWorker
{
    public:
        CMyWorker();
        virtual ~CMyWorker();

        enum {
            SERVER_TYPE_KFIF = 3,
        };

        enum {
            TO_MAN_SUC = 0,
        };

    public:
        virtual void doWork(std::string content);
        virtual void doWelcome();
        virtual	void doPass();

    private:
        int sendUrl(string& content,int windows);
        int encodeUrl(string & content);

        unsigned int string2uint( const std::string& ss );
        int next(const char**now, const char** start, char* buff);

    private:
        PassUrl        mPassUrl;
};

extern "C" CWorker* create() {
    return new CMyWorker;
}

extern "C" void destroy(CWorker* p) {
    delete p;
}

#endif


