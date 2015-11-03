#ifndef __ROBOT_ROBOT_H
#define __ROBOT_ROBOT_H

#include <string>
#include "CoChannel.h"

#include "handler.h"
#include "mc_handler.h"
#include "qt_client.h"
#include "worker.h"
#include "struct.h"
#include "data_channel_mng.h"

#include "memlog.h"

class CRobot
{
    public:
        CRobot();
        ~CRobot();

    public:
        int init(int argc, char *argv[]);

        robot::RobotContext * getContext();
        void setWorkerContext(uintptr_t context);

        void run();
    private:
        int loadLibrary(std::string path);
        void initSigHandler();
        static void sigHandler(int sig);

        static void dataChannelHandler(int fd, short events, void *arg);
    private:
        chaos::CoCycle			mCycle;
        chaos::CoChannel		mChannel;
        CHandler		        mHandler;
        CMcHandler		        mMcHandler;
        CQtClient               mQtClient;
        CDataChannelMng		    mDataChannelMng;
        chaos::CoLoger			mLoger;
        void 					* mWorkerLib;
        create_worker_t 	    * mCreate_worker ;
        destroy_worker_t 	    * mDestroy_worker;
        robot::RobotContext		mContext;

};

#endif
