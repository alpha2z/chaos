#ifndef __ROBOT_API_H
#define __ROBOT_API_H

#include "CoConnection.h"
#include "mc_transform_msg.h"
#include "mc_shift_route.h"
#include "mc_pass.h"
#include "mc_getTxsession.h"

struct DataChannelArg {
	uint32_t	seq;
	struct event ev;
	void *	coptr;
};

class CApi
{
	public:
		CApi();
		~CApi();

		enum {
			WEB_API_SUC = 0,
			WEB_API_FAIL = 1,
		};

	public:
		/*发送消息
		 * 输入：
		 * 输出：
		 */
		int sendMsg(string systemID, uint32_t sessionID,
				string sessionTicket, uint8_t imtype,
				string  fromUserName, string toUserName, string content, uint32_t txBitmapFlag);

		/*切换到其他系统，例如机器人、人工等
		 */
		int shiftRoute(string systemID, uint8_t oldServerType,
				uint16_t oldServerNo, uint8_t newServerType, uint16_t newServerNo,
				uint32_t sessionID, string openID, uint8_t imtype, string businessArgs, 
				uint32_t cityID, uint32_t attrID, uint32_t ogzID, uint8_t leaveWordType,
				uint32_t txCityID, uint32_t txUin, uint32_t txBitmapFlag,uint32_t txKfID , string strContent);   // ywy 2012-10-29 eidt

		int dataChannelReq(string key, void * reqItem, void * respItem, bool isWaitResponse);
		int passMsg(std::string systemID, uint8_t fromServerType, uint16_t fromServerNo,
				uint8_t toServerType, uint16_t toServerNo, uint32_t sessionID, 
				std::string content, uint8_t result, uint32_t seq,
				uint8_t subCmd, uint32_t businessCmd, uint8_t businessType);

		string	genKey(uint32_t uin);
		string	genKey(uint32_t uin, time_t time);
		string	genKey(uint32_t uin, time_t time,uint32_t cityID);

		int replace(string &text, string oldstr, string newstr);
		int replaceUrlVar(string &text, string systemID, string imqd,
				string province, string city, time_t time, string key, string openID, string imno);
		int replaceUrlVar(string &text, string systemID, string imqd,
				string province, string city, time_t time, string openID, uint32_t uin, uint32_t cityID);


		static void dataChannelTimeout(int fd, short events, void *arg);

		int  getTxSession(string systemID, string openID,uint8_t imtype,uint8_t forceRquest,uint_least32_t & txsessionid ,string & txSessionTicket) ;   //ywy 2012-07-10
			
        void sendMsg2QT(struct _stMessage*);
        void sendOpt2QT(struct _stConversion*);


//		chaos::CoConnection * webConnect(string domain, short port);
	private:
		CMcTransformMsg		mMcTransformMsg;
		CMcShiftRoute		mMcShiftRoute;
		CMcPass				mMcPass;
		CMcGetTxSession         mMcGetTxsession ;   //ywy 2012-07-10
};

#endif
