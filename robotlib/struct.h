#ifndef __STRUCT_H
#define __STRUCT_H

#include <string>

#pragma pack(push, 1)
typedef struct {
	uint16_t        len;
	uint16_t		ver;
	uint16_t		cmd;
	uint32_t		checkSum;
	uint32_t		seq;
	uint32_t		innerCmd;
}McMsgHead;

typedef struct {
	uint16_t        len;
	uint16_t		cmd;
	uint32_t		code;
	uint32_t		seq;
}CQtHead;
#pragma pack(pop)

struct _stConversion {
    uint8_t		uCmd;
    string      strOpenID;
    string      strTime;
    uint8_t     uOpt;
    uint32_t    uConversionID;
    uint32_t    uChannnelID;
    uint32_t    uCityID;
};

struct _stMessage {
    uint8_t     uCmd;
    string      strOpenID;
    uint8_t     uUserType;
    string      strSendTime;
    string      strContent;
};      


namespace robot {

	const   int     OPEN_ID_LEN = 16;
	const   unsigned int    MAX_UINT32 = 4294967295u;

	const std::string   URLKEY = "vlo-olv@qq.com";

	const	std::string	WEB_API_XML_ITEM_STR = "web_api_resp";

	const uint8_t MC_BEGIN = 0x1;
	const uint8_t MC_END = 0xff;

	const uint16_t	MC_VER = 0x01;

	enum {
		CMD_INNER_MAIN = 0x100,
	};

	enum {
		CMD_MC_LOGIN = 0x1,
		CMD_MC_TRANSFORM_MSG = 0x2,
		CMD_MC_SESSION_SEQ = 0x6,
		CMD_MC_GET_SESSION_TICKET = 0x7,
		CMD_MC_SHIFT_ROUTE = 0x8,
		CMD_MC_PASS_MSG = 0x0a,
		CMD_MC_BUSINESS_NOTIFY_RESULT = 0x0b,
		CMD_MC_QUERYTXSESSION = 0x18,
	};

	enum {
		CMD_QT_SYNC = 0x1,
		CMD_QT_CONV = 0x2,
		CMD_QT_MSG = 0x3,
	};

	enum {
		IM_TYPE_QQ = 1,
		IM_TYPE_MSN = 2,
		IM_TYPE_TY = 3,
		IM_TYPE_WEB = 4,
	};

	enum {
		SENDER_TYPE_QQ = 0,
		SENDER_TYPE_KF = 10,
		SENDER_TYPE_ROBOT = 20,
	};

	enum {
		SERVER_TYPE_IMIF = 0,
		SERVER_TYPE_ROBOT = 2,
		SERVER_TYPE_KFIF = 3,
	};

	enum {
		MC_HELLO_STATUS_DOING = 0,
		MC_HELLO_STATUS_DONE = 1,
	};

	//for rich msg variable
 //    const std::string    PREFIX = "<$";
	// const std::string    SUFFIX = "$>";
	// const std::string	MID = "|";

	// const std::string	VIEW_SET_GLOBAL = "GLOBAL"; 
	// const std::string	VIEW_WELCOME = "WEL_V"; 
	// const std::string	VIEW_BUSI_URL = "VIEW_BUSI_URL";
	// const std::string	VIEW_QUIT = "QUIT_V";
	// const std::string	VIEW_SET_TO_MAN = "TO_MAN_S";
	// const std::string	VIEW_TO_MAN = "TO_MAN_V";
//	const std::string	VIEW_SHIFT_ROUTE = "SHIFT_ROUTE_V";
	const std::string	DELIMITER = "|";

	const std::string	URL_VAR_HOSTTEL = "<$HOSTTEL$>";
	const std::string	URL_VAR_IMQD = "<$IMQD$>";
	const std::string	URL_VAR_OPENID = "<$QDID$>";
	const std::string	URL_VAR_QQ = "<$QQ$>";
	const std::string	URL_VAR_PROVINCE = "<$PROV$>";
	const std::string	URL_VAR_CITY = "<$CITY$>";
	const std::string	URL_VAR_CITYID = "<$CITYID$>";
	const std::string	URL_VAR_TIME = "<$TIME$>";
	const std::string	URL_VAR_KEY = "<$KEY$>";
	const std::string	URL_VAR_KEY2 = "<$KEY2$>";    //2013-06-17 增加城市id的加密码规则

	
	typedef struct {
		uintptr_t		mChannel;
		uintptr_t		mMcConn;		//msgcenter 服务器连接
		uintptr_t		mMcHandler;		//msgcenter 处理实例
		uintptr_t		mQtConn;        //qt 客服连接

		std::string		mMcDomain;
		short			mMcPort;
		unsigned int	mSeq;
		uintptr_t		mMcHelloEvent;
		uintptr_t		mMcHelloTimeval;
		int				mMcHelloTime;
		int				mMcHelloStatus;
		int				mServerNo;
		int				mServerType;
		int				mLoadLevel;

		std::string		mQtDomain;
		short			mQtPort;
		unsigned int	mQtSeq;
		uintptr_t		mQtHelloEvent;
		uintptr_t		mQtHelloTimeval;
		int				mQtHelloTime;
		int				mQtHelloStatus;

		uintptr_t		mCreateWorker;
		uintptr_t		mDestroyWorker;
		uintptr_t		mWorkerContext;
		uintptr_t		mDataChannelMngPtr;
		int				mDataChannelGetUsec;
		int				mDataChannelGetSec;
		uintptr_t		mDataChannelEv;
		uintptr_t		mMemLog;
		int				mCoroStackSize;
		struct	timeval	mDataChannelTv;
		uintptr_t		mConfigfilePtr;
	}RobotContext;

};

#endif
