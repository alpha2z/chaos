#include <signal.h>
#include <assert.h>
#include <syslog.h>
#include <iostream>
#include <dlfcn.h>

#include "CoData.h"
#include "CoChannel.h"
#include "CoCycle.h"

#include "configfile.h"
#include "configitem.h"
#include "configmap.h"
#include "handler.h"
#include "struct.h"
#include "user_mng.h"
#include "worker.h"
#include "robot.h"
#include "data_channel_mng.h"
#include "pcl.h"
#include "memlog.h"

using namespace std;
using namespace chaos;
using namespace configlib;
using namespace robot;


extern CoCycle *g_cycle;
RobotContext		*g_context;

CRobot::CRobot() { }

CRobot::~CRobot() {}

int CRobot::loadLibrary(string path)
{
	mWorkerLib = dlopen(path.c_str(), RTLD_LAZY);
	if (!mWorkerLib) {
		cerr << "Cannot load library: " << dlerror() << '\n';
		return -1;
	}

	dlerror();

	mCreate_worker = (create_worker_t *) dlsym(mWorkerLib, "create");
	const char* dlsym_error = dlerror();
	if (dlsym_error) {
		cerr << "Cannot load symbol create: " << dlsym_error << '\n';
		return -1;
	}

	dlerror();

	mDestroy_worker = (destroy_worker_t *) dlsym(mWorkerLib, "destroy");
	dlsym_error = dlerror();
	if (dlsym_error) {
		cerr << "Cannot load symbol destroy: " << dlsym_error << '\n';
		return 1;
	}

	return 0;
}

void CRobot::dataChannelHandler(int fd, short events, void *arg)
{
	CoCycle * cycle = g_cycle;
	CoChannel *channel = (CoChannel *)arg;

	RobotContext * context = g_context;

	CDataChannelMng * mng = (CDataChannelMng *)context->mDataChannelMngPtr;

	if(mng == NULL)	return;

	map<string, DataQueue *>::iterator	iter;
	
	map<string, DataQueue *> * respmap = mng->getRespMap();
	
	for(iter = respmap->begin(); iter != respmap->end(); iter++)
	{
		DataQueue * queue = iter->second;

		if(queue == NULL)	return;

		size_t len = queue->itemSize - sizeof(uint32_t);
		void * item = cycle->mPool->alloc(len);
		assert(item != NULL);

		uint32_t seq = 0;

		while(mng->get(iter->first, item, CDataChannelMng::DATA_CHANNEL_TYPE_RESP, seq) == 0) {
	
			LOG(5, "seq[%d]", seq);
			map<uint32_t, void *>::iterator iterCoro;
			iterCoro = mng->getArgMap()->find(seq);

			if(iterCoro == mng->getArgMap()->end())  continue;

			coroutine_t co = (coroutine_t ) iterCoro->second;
			
			uintptr_t * args = (uintptr_t *) cycle->mPool->alloc(sizeof(uintptr_t) * 2);
			assert(args != NULL);

			args[0] = (uintptr_t) &len;
			args[1] = (uintptr_t) item;
			co_set_data(co, (void *) args);
			co_call(co);

			cycle->mPool->free(args);
			args = NULL;
		}
		cycle->mPool->free(item);
		item = NULL;
	}

	static time_t t = time(NULL);

	time_t curt = time(NULL);
	if(curt - t > 10) {
		LOG(7, "allocnum:%d,  freenum:%d, cha:%d", cycle->mPool->mAllocNum, cycle->mPool->mFreeNum, cycle->mPool->mAllocNum - cycle->mPool->mFreeNum);
		t = curt;
	}

	struct timeval tv;
	tv.tv_usec = context->mDataChannelGetUsec;
	tv.tv_sec = context->mDataChannelGetSec;

	channel->addTimer((struct event *)context->mDataChannelEv, tv, dataChannelHandler, channel);
}

void CRobot::sigHandler(int sig)
{
	configfile * cf = (configfile *) g_context->mConfigfilePtr;

	configitem<int> loglevel(*cf, "log", "loglevel", "", 0);

	cf->read();

	LOG(5, "reset log level:%d", (int) loglevel);
	g_cycle->mLogLevel = (int) loglevel;
}

void CRobot::initSigHandler()
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGUSR1, &act, NULL);
}

int CRobot::init(int argc, char *argv[])
{
	if(argc != 2) {
		cout<<"Usage: "<<argv[0]<<" confile"<<endl;
		return -1;
	}

	initSigHandler();

	configfile * config = new configfile(argv[1]);

	configitem<std::string> localip(*config, "robot", "localip", "", "");
	configitem<int> localport(*config, "robot", "localport", "", 0);
	configitem<int> serverType(*config, "robot", "servertype", "", 0);
	configitem<int> serverNo(*config, "robot", "serverno", "", 0);
	configitem<int> loadLevel(*config, "robot", "loadlevel", "", 0);
	configitem<std::string> searchDomain(*config, "robot", "searchdomain", "", "");
	configitem<std::string> searchUrl(*config, "robot", "searchurl", "", "");
	configitem<int> searchPort(*config, "robot", "searchport", "", 0);
	configitem<std::string> libpath(*config, "robot", "libpath", "", "");
	configitem<int> daemon(*config, "robot", "daemon", "", 0);
	configitem<int> datachannelgetusec(*config, "robot", "datachannelgetusec", "", 0);
	configitem<int> datachannelgetsec(*config, "robot", "datachannelgetsec", "", 0);
	configitem<int> corostacksize(*config, "robot", "corostacksize", "", 0);
	configitem<int> datachanneltimeoutusec(*config, "robot", "datachanneltimeoutusec", "", 0);
	configitem<int> datachanneltimeoutsec(*config, "robot", "datachanneltimeoutsec", "", 0);
	
    configitem<std::string> logpath(*config, "log", "logpath", "", "");
    configitem<int> loglevel(*config, "log", "loglevel", "", 0);
    configitem<int> filenum(*config, "log", "filenum", "", 5);
    configitem<int> filesize(*config, "log", "filesize", "", 500000000);

	configitem<std::string> mcdomain(*config, "msgcenter", "domain", "", "");
	configitem<int> mcport(*config, "msgcenter", "port", "", 0);
	configitem<int> mchellotime(*config, "msgcenter", "hellotime", "", 0);

	configitem<std::string> qtdomain(*config, "client", "client_address", "", "");
	configitem<int> qtport(*config, "client", "client_port", "", 0);

	config->read();

	string lp = (string)logpath;
	cout<<"loger:"<<(string)logpath<<endl;
	mLoger.init((char *)(lp.c_str()), 0, (int)filenum, (int)filesize);

	mCycle.mLogLevel = (int) loglevel;

	g_cycle = &mCycle;
	

	if((int)daemon == 1)		g_cycle->daemon();

	if(mChannel.init(&mLoger) < 0) {
		cout<<"init fail"<<endl;
		return -2;
	}

	LOG(5, "alloc:%d ,%d", g_cycle->mPool->mAllocNum, g_cycle->mPool->mFreeNum);

	mMcHandler.mWriteTimeoutSec = 5;
	mMcHandler.mReadTimeoutSec = 5;

	mQtClient.mWriteTimeoutSec = 5;
	mQtClient.mReadTimeoutSec = 5;

	CoConnection *mcConn = NULL;
	char * mcip = NULL;
	CoConnection *qtConn = NULL;
	char * qtip = NULL;
	
	mcip = CoData::getIPByDomain((char *)((string)mcdomain).c_str());
	if(mcip == NULL) {
		cout<<"configure file error,mcdomain:"<<(string)mcdomain<<endl;
		return -30;
	}
    cout<<"configure file error,mcdomain:"<<(string)mcdomain<<endl;
    cout<<"mc:"<<mcip<<":"<<mcport<<endl;

    mcConn = mChannel.createClientChannel(mcip,
			mcport, &mMcHandler, CoConnection::CONN_MODE_TCP, true);
	if(mcConn == NULL) { 
		cout<<"conn to msg center server fail"<<endl;
		return -4;
	}

	qtip = CoData::getIPByDomain((char *)((string)qtdomain).c_str());
	if(qtip == NULL) {
		cout<<"configure file error,qtdomain:"<<(string)qtdomain<<endl;
		return -31;
	}
    cout<<"configure file error,qtdomain:"<<(string)qtdomain<<endl;
	cout<<"qt:"<<qtip<<":"<<qtport<<endl;

	qtConn = mChannel.createClientChannel(qtip,
		qtport, &mQtClient, CoConnection::CONN_MODE_TCP, true);

	if(qtConn == NULL) { 
		cout<<"conn to quantong server fail"<<endl;
		return -4;
	}
 
	int ret = 0;

	struct timeval datachanneltv;
	struct event *mcev = new struct event;
	struct timeval *mctv = new struct timeval;
	struct event *datachannelev = new struct event;

	mctv->tv_usec = 0;
	mctv->tv_sec = mchellotime;
	datachanneltv.tv_usec = (int)datachannelgetusec;
    datachanneltv.tv_sec = (int)datachannelgetsec;
 
	mWorkerLib = NULL;
	mCreate_worker = NULL;
	mDestroy_worker = NULL;

	ret = loadLibrary((string)libpath);
	if(ret < 0) {
		cout<<"load library fail:"<<ret<<endl;
		return -6;
	}

	struct event *qtev = new struct event;
	struct timeval *qttv = new struct timeval;

	mContext.mChannel = (uintptr_t) &mChannel;
	mContext.mMcHelloEvent = (uintptr_t) mcev;
	mContext.mMcHelloTime = mchellotime;
	mContext.mMcDomain = (string) mcdomain;
	mContext.mMcPort	= (int) mcport;
	mContext.mMcConn = (uintptr_t) mcConn;
	mContext.mServerType = (int) serverType;
	mContext.mLoadLevel = (int) loadLevel;
	mContext.mServerNo = (int) serverNo;
	mContext.mMcHelloTimeval = (uintptr_t) mctv;

	mContext.mQtHelloEvent = (uintptr_t) qtev;
	mContext.mQtHelloTime = mchellotime;
	mContext.mQtDomain = (string) qtdomain;
	mContext.mQtPort	= (int) qtport;
	mContext.mQtConn = (uintptr_t) qtConn;
	mContext.mQtHelloTimeval = (uintptr_t) qttv;
 
	mContext.mSeq = 0;
	mContext.mQtSeq = 0;

	mContext.mDataChannelMngPtr = (uintptr_t) &mDataChannelMng;
	mContext.mDataChannelEv = (uintptr_t) datachannelev;
	mContext.mDataChannelGetUsec = datachannelgetusec;
	mContext.mDataChannelGetSec = datachannelgetsec;
	mContext.mCreateWorker = (uintptr_t) mCreate_worker;
	mContext.mDestroyWorker = (uintptr_t) mDestroy_worker;
	mContext.mCoroStackSize = (int) corostacksize;
	mContext.mDataChannelTv.tv_usec = datachanneltimeoutusec;
	mContext.mDataChannelTv.tv_sec = datachanneltimeoutsec;

	mContext.mConfigfilePtr = (uintptr_t) config;

	g_context = &mContext;

	mHandler.mWriteTimeoutSec = 5;
	mHandler.mReadTimeoutSec = 0;

	//mChannel.addTimer(datachannelev, datachanneltv, dataChannelHandler, &mChannel);

    cout<<"ip:"<<(string)localip<<",port:"<<localport<<endl;
    if (localport > 0) {
        ret = mChannel.createServerChannel((char *)((string)localip).c_str(),
                localport, &mHandler, CoConnection::CONN_MODE_TCP);

        if (ret < 0) {
            cout<<"create channle fail:"<<ret<<":"<<strerror(errno)<<endl;
            return -7;
        }
    }

	LOG(5, "alloc:%d ,%d", g_cycle->mPool->mAllocNum, g_cycle->mPool->mFreeNum);
	return 0;
}

void CRobot::run()
{
   g_cycle->mLoger->start(); 
	g_cycle->mLoger->doLog("server start");
	while(1) {
		mChannel.runOnce();
	}
	dlclose(mWorkerLib);
}
RobotContext * CRobot::getContext()
{
	return g_context;
}

void CRobot::setWorkerContext(uintptr_t context)
{
	g_context->mWorkerContext = context;
}
