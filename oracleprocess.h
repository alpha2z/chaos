#ifndef _ORACLEPROCESS__H
#define _ORACLEPROCESS__H
#include "CoData.h"
#include "CoBuffer.h"

using namespace chaos;


#include <string>

#define OTL_ORA11G
#define OTL_STL
#define OTL_ANSI_CPP

#include "otlv4.h"

using namespace std;

class OracleProcess
{
public:
	~OracleProcess();
	int Init(string	strUserName,string	strPass,string	strDBName ,int iMaxLongString);
	
	int Select(string & sql,otl_stream & osi ,int iSize );
	int ProcessSql(string &sql);
	int ProcessSql2(string & sql);
	int ProcessSql3(string & sql,otl_stream & osi ,int iSize );
	int Close();
	
	otl_connect db; // connect object
	int m_iDBFlag; //连接标志
	string  m_strUserName;
	string  m_strPass;
	string  m_strDBName;
	int m_iMaxLongString;
};


#endif

