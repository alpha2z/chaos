#ifndef __QT_MSG_H__
#define __QT_MSG_H__


#include "qt_check.h"

class CQtMsg : public CQtBase
{
	public:
		enum {
			SUB_CMD_REQ_MSG = 0,
			SUB_CMD_RESP_MSG = 1,
		};

		CQtMsg() {}
		~CQtMsg() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	    void setCmd(uint8_t cmd);
	    void setOpenID(string* arrOpenID);
	    void setUserType(uint8_t uUserType);
	    void setTime(string *arrTime);
	    void setContent(string *pTemp);

	public:
		uint8_t			m_uCmd;
		string          m_strOpenID;
		uint8_t			m_uUserType;
		string			m_strSendTime;
		string			m_strContent;

		uint8_t			m_uResult;
};

#endif //__QT_MSG_H__
