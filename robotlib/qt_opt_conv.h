#ifndef __QT_CONVERSION_H__
#define __QT_CONVERSION_H__


#include "qt_check.h"

class CQtConversion : public CQtBase
{
    public:
		enum {
			SUB_CMD_REQ_MSG = 0,
			SUB_CMD_RESP_MSG = 1,
		};

		CQtConversion() {}
		~CQtConversion() {}

		virtual int encode();
		virtual int decode();

		virtual int processReq(chaos::CoConnection *c);
		virtual int processResp(chaos::CoConnection *c);

	    void setCmd(uint8_t cmd);
	    void setOpenID(string* arrOpenID);
        void setSendTime(string*);
	    void setOpt(uint8_t opt);
        void setConversionID(uint32_t uConversionID);
        void setCityID(uint32_t uCityID);
        void setChannelID(uint32_t id);

	public:
		uint8_t			m_uCmd;
		string          m_strOpenID;
        string          m_strSendTime;
        uint8_t         m_uOpt;
        uint32_t        m_uConversionID;
        uint32_t        m_uChannelID;
        uint32_t        m_uCityID;

		uint8_t			m_uResult;
};

#endif// __QT_CONVERSION_H__
