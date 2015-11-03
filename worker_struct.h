#ifndef __WORKER_STRUCT_H
#define __WORKER_STRUCT_H

#include <string>

const std::string    PREFIX = "<$";
const std::string    SUFFIX = "$>";
const std::string   MID = "|";

//const std::string   VIEW_SET_GLOBAL = "P_GD2_S";

const std::string   VIEW_SET_GLOBAL = "GLOBAL"; 
const std::string   VIEW_WELCOME = "WEL_V"; 
const std::string   VIEW_BUSI_URL = "VIEW_BUSI_URL";
const std::string   VIEW_QUIT = "QUIT_V";
const std::string   VIEW_SET_TO_MAN = "TO_MAN_S";
const std::string   VIEW_TO_MAN = "TO_MAN_V";

// const std::string   VIEW_WELCOME = "WEL_V";
const std::string	VIEW_SECOND_WEL = "SECOND_WEL_V";
const std::string	VIEW_SECOND_QQ_WEL = "SECOND_QQ_WEL_V";
// const std::string	VIEW_QUIT = "QUIT_V";
const std::string	VIEW_NOT_SERVICE_TIME = "NOT_SERVICE_TIME_V";
const std::string	VIEW_SET_JUMPER = "JUMPER_S";
// const std::string   VIEW_TO_MAN = "TO_MAN_V";
const std::string   VIEW_TO_MAN_FAIL = "TO_MAN_FAIL_V";
const std::string	ACTION_PROVINCE_LIST = "PROVINCE_LIST_A";
const std::string	VIEW_SET_PROVINCE = "PROVINCE_S";
const std::string	VIEW_SET_CITY = "CITY_S";
const std::string	VIEW_CITY = "CITY_V";
const std::string	ACTION_CITY_LIST = "CITY_LIST_A";
const std::string	VIEW_SET_ATTR = "ATTR_S";
const std::string	ACTION_ATTR_LIST = "ATTR_LIST_A";
const std::string	VIEW_CHOICE = "CHOICE_V";
const std::string	VIEW_CHOICE2 = "CHOICE2_V";
const std::string	ACTION_LAST_PRO = "LAST_PRO_A";
const std::string	ACTION_PROVINCE_VAR = "PROVINCE_VAR";
const std::string	ACTION_CITY_VAR = "CITY_VAR";
const std::string	VIEW_RET_CITY_LIST = "RET_CITY_LIST_V";
const std::string   ACTION_TIME_RANGE_VAR = "TIME_RANGE_VAR";
//2012-06-04 add ywy
const std::string    MOBILE_VAR_CITY_ID  = "<$CITY_ID$>";
const std::string    MOBILE_VAR_QD_ID  = "<$QD_ID$>";

//2014-1-24 add by alice 
const std::string 	MOBILE_VAR_U_PARAM = "<$U_PARAM$>";
const std::string 	MOBILE_RELATED_TEL_NUM = "<$RELATED_TEL_NUM$>";

const std::string	VIEW_SET_RESEARCH = "RESEARCH_S";
const std::string	VIEW_SET_EXPIRE = "EXPIRE_S";
const std::string	VIEW_SET_LY = "LY_S";

const std::string   TENCENT_DEFALUT_STR = "default:SigT=";
const std::string   GLOBAL_IKNOW=PREFIX +"IKNOW" + SUFFIX; 
const std::string   GLOBAL_INDATA=PREFIX +"INDATA" + SUFFIX+"000010" ;

const std::string   DATA_CHANNEL_POST_REQ = "D_C_POST_REQ";
const std::string   DATA_CHANNEL_POST_RESP = "D_C_POST_RESP";
const std::string   DATA_CHANNEL_POST_VISIT_REQ = "D_C_POST_VISIT_REQ";



const std::string	MOBILE_VAR_PROVINCE = PREFIX + "PROVINCE_VAR" + SUFFIX;
const std::string   MOBILE_VAR_CITY = PREFIX + "CITY_VAR" + SUFFIX;
const std::string	MOBILE_VAR_ATTR_LIST = PREFIX + "ATTR_LIST_VAR" + SUFFIX;
const std::string	MOBILE_VAR_TIME_RANGE = PREFIX + "TIME_RANGE_VAR" + SUFFIX;

const   uint16_t    TAG1 = 1;
const   uint16_t    TAG2 = 2;
const   uint16_t    TAG3 = 3;
const   uint16_t    TAG4 = 4;
const   uint16_t    TAG5 = 5;
const   uint16_t    TAG6 = 6;
const   uint16_t    TAG7 = 7;
const   uint16_t    TAG8 = 8;
const   uint16_t    TAG9 = 9;
const   uint16_t    TAG10 = 10;
const   uint16_t    TAG99 = 99;
const   uint16_t    TAG31 = 31;    //ywy 2012-06-04 add
const   uint16_t    TAG32 = 32;    //ywy 2012-06-04 add

typedef struct {
	uintptr_t		mViewMngPtr;
	uintptr_t		mMobileViewMngPtr;
	uintptr_t		mUserMngPtr;
	uintptr_t		mOgzMngPtr;
    uintptr_t       mClient;
	int				mUserExpireSec;
	int				mUserExpireUsec;
} WorkerContext;

#pragma pack(push, 1)

struct PassUrl {
    uint16_t        mSeq;
    uint32_t        mUin;
    string          mSessionTicket;
    uint16_t        mSessionSeq;
    uint8_t         mWindow;
    string          mUrl;   
};

struct _stNodeData {
    uint32_t uLinkID;
    uint32_t uCityID;
    uint32_t uID;
    uint32_t uChannelID;
    char chKeyWord[128];
    struct _stNodeData& operator = (const struct _stNodeData& data) {
        uLinkID = data.uLinkID;
        uCityID = data.uCityID;
        uID = data.uID;
        uChannelID = data.uChannelID;
        memset(chKeyWord, '\0', 128);
        uint16_t len = strlen(data.chKeyWord);
        memcpy(chKeyWord, data.chKeyWord, len);
        return *this;
    }
    _stNodeData():uLinkID(0),uCityID(0),uID(0){
        memset(chKeyWord, '\0', 128);
    }

};

struct _stNode {
    struct _stNodeData stData;
    struct _stNode* children;
    struct _stNode* next;
    uint8_t c;
    _stNode() {
        children = NULL;
        next = NULL;
        c = 0;
    }
};



#pragma pack(pop)

#endif
