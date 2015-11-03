//#include <regex.h>
#include <errno.h>
#include <pcre.h> 
#include <assert.h>
#include <sstream>
#include "CoCycle.h"
#include "myworker.h"
#include "ogz_mng.h"
#include <ctype.h>
#include <curl/curl.h>
#include "code_converter.h"
#include "robotlib/openid_converter.h"
#include "robotlib/api.h"
#include "robotlib/md5.h"
#include <time.h>


using namespace std;
using namespace chaos;

#define DEFAULT_MESSAGE "default:SigT=&Param="
#define SAFE_FREE(x) if (x) {free(x); x = NULL;}

#define OPEN_CONVERSION 1
#define CLOSE_CONVERSION 2
#define NOTIFICATION_CONVERSION 3

extern CoCycle * g_cycle;

CMyWorker::CMyWorker() {}
CMyWorker::~CMyWorker() {}


int CMyWorker::encodeUrl(string & content)
{
    CoCycle * cycle = g_cycle;

    int dataOffset = 0;

    int dataLen = 2*sizeof(uint16_t) + sizeof(mPassUrl.mSeq)
        + 2*sizeof(uint16_t) + sizeof(mPassUrl.mUin)
        + 2*sizeof(uint16_t) + mPassUrl.mSessionTicket.length()
        + 2*sizeof(uint16_t) + sizeof(mPassUrl.mSessionSeq) 
        + 2*sizeof(uint16_t) + sizeof(mPassUrl.mWindow)
        + 2*sizeof(uint16_t) + mPassUrl.mUrl.length() ;


    char * bufUrl = (char *)cycle->mPool->alloc(dataLen);

    if(bufUrl == NULL) {
        LOG(2, "alloc fail");
        return -1;
    }

    memset(bufUrl, 0, dataLen);

    uint16_t	len = sizeof(mPassUrl.mSeq);
    CoData::put(bufUrl, dataOffset, TAG1);
    CoData::put(bufUrl, dataOffset, len);
    CoData::put(bufUrl, dataOffset, mPassUrl.mSeq);

    len = sizeof(mPassUrl.mUin);
    CoData::put(bufUrl, dataOffset, TAG2);
    CoData::put(bufUrl, dataOffset, len);
    CoData::put(bufUrl, dataOffset, mPassUrl.mUin);

    CoData::put(bufUrl, dataOffset, TAG3);
    CoData::put(bufUrl, dataOffset, mPassUrl.mSessionTicket, 0);

    len = sizeof(mPassUrl.mSessionSeq);
    CoData::put(bufUrl, dataOffset, TAG4);
    CoData::put(bufUrl, dataOffset, len);
    CoData::put(bufUrl, dataOffset, mPassUrl.mSessionSeq);

    len = sizeof(mPassUrl.mWindow);
    CoData::put(bufUrl, dataOffset, TAG31);
    CoData::put(bufUrl, dataOffset, len);
    CoData::put(bufUrl, dataOffset, mPassUrl.mWindow);

    CoData::put(bufUrl, dataOffset, TAG32);
    CoData::put(bufUrl, dataOffset, mPassUrl.mUrl, 0);

    content.clear();
    content.append(bufUrl, dataOffset);

    cycle->mPool->free(bufUrl);
    bufUrl = NULL;
    return 0;
}

unsigned int CMyWorker::string2uint( const std::string& ss )   
{
    int i,len = (int)ss.size();
    unsigned int num = 0 ;

    i = 0 ;

    while (i < len)
    {
        num = num * 10 + ( int )(ss[i] - '0' );
        i ++ ;                                                                                                                                               
    } 

    return num;
}

int CMyWorker::sendUrl(string& content,int windows) {
    int ret = 0 ;
    string retContent = "";
    string fname = mFromUserName ;

    if(fname.length() < 1 ) return -1 ;
    string firtName = fname.substr(0,1);

    int nQQ = string2uint(mFromUserName);
    string  strW = "3" ;
    string  strItem = content ;
    uint8_t iW =windows ;

    //uint8_t uPlatform = (mTXBitmapFlag & 0x6) >> 1;
    uint8_t imtype =1 ;
    uint8_t forceRquest =0 ;
    uint32_t txsessionid = 0 ;
    string yy = "" ;

    mSessionTicket = "";
    ret = getTxSession(mSystemID, mFromUserName,imtype, forceRquest,txsessionid,mSessionTicket) ;
    if(ret != 0){
        LOG(2,"getTxSession failed user:%s, [%d] session id : %d", mFromUserName.c_str(), ret, txsessionid);
        return -1;
    }
    LOG(2,"getTxSession succeed user:%s, session id:%d", mFromUserName.c_str(), txsessionid);

    if (mSessionTicket.length() == 0) {
        LOG(2, "session ticket too short user:%s", mFromUserName.c_str());
        return -1;
    }

    int ti = (unsigned) time(NULL);
    short ts = ti;

    mPassUrl.mUin = nQQ;
    mPassUrl.mSessionTicket = mSessionTicket ;
    mPassUrl.mUrl = strItem ;
    mPassUrl.mSeq = ts ;
    mPassUrl.mSessionSeq = txsessionid ;	
    mPassUrl.mWindow = iW ;

    ret = encodeUrl(retContent);
    if(ret < 0) {
        LOG(2, "encode packet fail[%d] user:%s", ret, mFromUserName.c_str());
        return -1;
    }

    uint8_t result = 0;
    uint8_t	subCmd=0;
    uint32_t businessCmd=2;
    uint8_t businesType=0;

    mNewServerNo = 0 ;
    mNewServerType =0 ;

    ret = passMsg(subCmd, businessCmd, businesType, retContent, result);
    if(ret < 0) {
        LOG(2, "pass msg fail, user:%s, ret:%d.", mFromUserName.c_str(), ret);
        return -1;
    } else {
        LOG(2, "pass msg success. user:%s, content:%s.", mFromUserName.c_str(), content.c_str());
    }

    return 0 ; 
}

void CMyWorker::doWork(string content)
{

}

void CMyWorker::doWelcome()
{

}

int CMyWorker::next(const char**now, const char** start, char* buff) {
    int len = *now - *start;
    if (len <= 0)
        return -1;

    memcpy(buff, *start, len);
    buff[len] = '\0';
    return len;
}

void CMyWorker::doPass()
{
    string content = mContent;
    int tl = content.length();
    uint32_t uConversionID = 0;
    uint32_t uChannnelID = 0;
    uint8_t uUserType = 0;
    uint8_t uOptType = 0xFF;
    uint32_t uCityID = 0;

    string strOpenID;
    string strTime;
    string strContent;

    // 1 打开会话|会话ID|OPENID|渠道ID|用户类型|对话时间|CITYID|
    // 2 关闭会话|会话ID|OPENID|渠道ID|用户类型|对话时间|0|
    // 3 内容推送|OPENID|用户类型|对话时间|对话内容|
    const char* start = content.c_str();
    const char* end = content.c_str() + tl;
    const char* now = start;
    int len = 0;
    int idx = 0;
    bool complete = false;
    char buff[tl];
    for (; now < end; now++) {
        if (*now == '|') {
            len = next(&now, &start, buff);
            if (len < 0) {
                LOG(2, "parse content error, %s", content.c_str());
                break;
            }

            if (idx == 0) {
                // get opt type
                uOptType = strtol(buff, NULL, 10);
                start = now + 1;
            } else {
                if (uOptType == OPEN_CONVERSION || uOptType == CLOSE_CONVERSION) {
                    if (idx == 1) {
                        // get conversionID
                        uConversionID = strtol(buff, NULL, 10);
                        start = now + 1;

                    } else if (idx == 2) {
                        // get openID
                        strOpenID = "";
                        strOpenID.append(buff, len);
                        start = now + 1;

                    } else if (idx == 3) {
                        // get channelID
                        uChannnelID = strtol(buff, NULL, 10);
                        start = now + 1;

                    } else if(idx == 4) {
                        // get user type
                        uUserType = strtol(buff, NULL, 10);
                        start = now + 1;

                    } else if (idx == 5) {
                        // get time
                        strTime = "";
                        strTime.append(buff, len);
                        start = now + 1;

                    } else if(idx == 6) {
                        // get cityid
                        uCityID = strtol(buff, NULL, 10);
                        start = now + 1;
                        complete = true;
                        break;
                    }

                } else if (uOptType == NOTIFICATION_CONVERSION) {
                    if (idx == 1) {
                        // get openID
                        strOpenID = "";
                        strOpenID.append(buff, len);
                        start = now + 1;

                    } else if(idx == 2) {
                        // get user type
                        uUserType = strtol(buff, NULL, 10);
                        start = now + 1;

                    } else if (idx == 3) {
                        // get  time
                        strTime = "";
                        strTime.append(buff, len);
                        start = now + 1;
                    } else if (idx == 4) {
                        strContent.append(buff, len);
                        complete = true;
                        break;
                    }
                }
            }
            idx++;
        }
    }

    if (complete == false) {
        LOG(2, "content is incomplete, idx:%d openID:%s, %s ", idx, strOpenID.c_str(), content.c_str());
        return;
    }

    if (uOptType == OPEN_CONVERSION) {
        struct _stConversion conversion;
        conversion.uCmd = 0;
        conversion.strOpenID = strOpenID;
        conversion.strTime = strTime;
        conversion.uOpt = 0;
        conversion.uConversionID = uConversionID;
        conversion.uChannnelID = uChannnelID;
        conversion.uCityID = uCityID;
        sendOpt2QT(&conversion);

        LOG(2, "conversion open userid:%s, conversion:%d, channel:%d, time:%s, city:%d", 
                strOpenID.c_str(), uConversionID, uChannnelID, strTime.c_str(), uCityID);

    } else if (uOptType == CLOSE_CONVERSION) {
        struct _stConversion conversion;
        conversion.uCmd = 0;
        conversion.uOpt = 1;
        conversion.uConversionID = uConversionID;
        conversion.strTime = strTime;
        conversion.strOpenID = strOpenID;
        conversion.uChannnelID = uChannnelID;
        conversion.uCityID = 0;
        sendOpt2QT(&conversion);

        LOG(2, "conversion close userid:%s, conversion:%d, channel:%d, time:%s",
                strOpenID.c_str(), uConversionID, uChannnelID, strTime.c_str());
    } else if (uOptType == NOTIFICATION_CONVERSION) {
        struct _stMessage message;
        message.uCmd = 0;
        message.strOpenID = strOpenID;
        message.uUserType = uUserType;
        message.strSendTime = strTime;
        message.strContent = strContent;

        sendMsg2QT(&message);
        LOG(2, "notify msg to quantong userid:%s, time:%s, content:%s", 
                strOpenID.c_str(), strTime.c_str(), strContent.c_str());
    }



    time_t now_secs;
    //struct tm *p;
    time(&now_secs);
    LOG(2, "time() : %d(secs)", now_secs);
    //p = localtime(&timep);
    //timep = mktime(p);
    //LOG(2, "time()->localtime()->mktime():%d", timep);

}








