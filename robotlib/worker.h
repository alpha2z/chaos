#ifndef __ROBOT_WORKER_H
#define __ROBOT_WORKER_H

#include <string>
#include "CoCycle.h"
#include "CoConnection.h"

#include "api.h"
#include "struct.h"

class CWorker
{
	public:
		CWorker();
		virtual ~CWorker();

	public:
		virtual void doWork(std::string content) = 0;
		virtual	void doWelcome() = 0;
		virtual	void doPass() = 0;
	 
	public:
		robot::RobotContext	*getContext();
		void setContext(robot::RobotContext * context);

		uintptr_t  getWorkerContext();

		chaos::CoCycle		*getCycle();
		void setCycle(chaos::CoCycle *cycle);

		static void work(void * args);
		static void welcome(void *args);
		static void pass(void *args);

    protected:
        int sendMsg(std::string content);
        int shiftRoute(uint8_t serverType, uint16_t serverNo, std::string businessArgs,
                uint32_t cityID, uint32_t attrID, uint32_t ogzID, std::string strContent,uint32_t txKfID = 0 ,uint8_t leaveWordType = 0,
                uint32_t txCityID = 0, uint32_t txUin = 0);   //ywy 2012-10-29 eidt

        int dataChannelReq(std::string key, void * reqItem, void * respItem, bool isWaitResponse);
        int passMsg(uint8_t subCmd, uint32_t businessCmd, uint8_t businesType, 
                std::string content, uint8_t result);
        int replaceUrlVar(string &text, string systemID, string imqd, 
				        string province, string city, time_t time, string key, 
						string openID, string imno);

		string genKey(uint32_t uin);

		
		int  getTxSession(std::string systemID, std::string openID,uint8_t imtype,uint8_t forceRquest,uint_least32_t & txsessionid,std::string & txSessionTicket ) ; //ywy 2012-08-09
        
        void sendMsg2QT(struct _stMessage*);
        void sendOpt2QT(struct _stConversion*);
		
	public:
		robot::RobotContext	*mContext;
		uintptr_t	mWorkerContext;
		chaos::CoCycle			*mCycle;

	public:
		std::string		mSystemID;
		uint32_t     mSessionID;
		std::string       mSessionTicket;
		uint8_t      mIMType;
		std::string       mFromUserName;
		std::string       mToUserName;
		std::string       mContent;
		uint8_t 			 mOldServerType;
		uint16_t			mOldServerNo;
		uint8_t 			 mNewServerType;
		uint16_t			mNewServerNo;
		uint32_t			mAttrID;
		uint32_t			mOgzID;
		uint32_t			mCityID;
		std::string			mKFName;
		uint32_t			mBusinessSetID;
		std::string			mBusinessArgs;
		uint32_t			mTXUin;
		uint32_t			mTXCityID;
		uint32_t			mTXBitmapFlag;
		uint8_t				mLeaveWordType;
		uint32_t			mSeq;	
		uint32_t			mPassBusinessCmd;
		uint8_t				mPassBusinessType;
		uint32_t            mKfID ;
		std::string         mRouteContext ;
	private:
		CApi			mApi;
};

typedef CWorker* create_worker_t();
typedef void destroy_worker_t(CWorker*);

#endif
