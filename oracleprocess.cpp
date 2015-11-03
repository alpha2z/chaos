#include "CoCycle.h"
#include <iostream>
#include "oracleprocess.h"

extern CoCycle *g_cycle;

using namespace std;

OracleProcess::~OracleProcess()
{
	db.logoff();
}

int OracleProcess::Init(string  strUserName,string  strPass,string  strDBName ,int iMaxLongString)
{
    m_strUserName = strUserName;
    m_strPass = strPass;
    m_strDBName = strDBName;
    m_iMaxLongString = iMaxLongString;

    m_iDBFlag = 0;
    if( iMaxLongString >0)
        db.set_max_long_size(iMaxLongString);// set maximum long string size for connect object

    otl_connect::otl_initialize(); // initialize OCI environment
    try
    {
        //im/im123@61.144.207.90:1521/bst
        string sDBLogin = strUserName + "/"+strPass + "@"+strDBName;
        LOG(2,"OracleProcess init[%s]",sDBLogin.c_str());
        db.rlogon(sDBLogin.c_str()); // connect to Oracle
        if( db.connected == 1)
            m_iDBFlag = 1;

        //db.direct_exec("drop table test_tab", otl_exception::disabled );
    }
    catch(otl_exception& p)
    { // intercept OTL exceptions
        LOG(6,"OracleProcess init[%s][%s]",p.msg,p.stm_text);
        string sErrMsg =(const  char*)p.msg;

        if(sErrMsg.find( "otl_connect is already connected")!=string::npos )
        {
            //和数据库连接中断
            LOG(6,"DB Connect already connected " );
            m_iDBFlag = 1;
        }

        cerr<<p.msg<<endl; // print out error message
        cerr<<p.stm_text<<endl; // print out SQL that caused the error
        cerr<<p.var_info<<endl; // print out the variable that caused the error

        return -1;
    }
    m_iDBFlag = 1;

    return db.connected;
}

int OracleProcess::Select(string & sql,otl_stream & osi ,int iSize )
{
    if( db.connected!=1)
        return -2;
    LOG(6,"Select [%s]",sql.c_str());
    try{

        osi.open(iSize,sql.c_str() ,db);
    }
    catch(otl_exception& p)
    { // intercept OTL exceptions
        LOG(6,"OracleProcess Select[%s][%s][%d]",p.msg,p.stm_text, p.code);
        cerr<<p.msg<<endl; // print out error message
        cerr<<p.stm_text<<endl; // print out SQL that caused the error
        cerr<<p.var_info<<endl; // print out the variable that caused the error

        string sErrMsg =(const  char*)p.msg;

        if(sErrMsg.find( "ORA-03114")!=string::npos )
        {
            //和数据库连接中断
            LOG(6,"DB Connect close " );
            db.logoff();
            m_iDBFlag = 0;
        }

        return -1;
    }

    if( db.connected!=1)
        return -2;
    return 0;
}

int OracleProcess::ProcessSql2(string & sql)
{
    LOG(6,"Processsql [%s]",sql.c_str());
    if( db.connected!=1)
        return -1;
    int iRet = 0;
    try{
        LOG(5,"data a [%s]",sql.c_str());

        db.direct_exec(sql.c_str(), otl_exception::disabled );
        db.commit();
        LOG(5,"data b [%s]",sql.c_str());
        //	otl_stream o(50, // buffer size
        //				 sql.c_str(),
        //					// SQL statement
        //				 db // connect object
        //				);
    }
    catch(otl_exception& p)
    { // intercept OTL exceptions
        LOG(6,"OracleProcess ProcessSql[%s][%s]",p.msg,p.stm_text);
        cerr<<p.msg<<endl; // print out error message
        cerr<<p.stm_text<<endl; // print out SQL that caused the error
        cerr<<p.var_info<<endl; // print out the variable that caused the error
        string sErrMsg = (const  char*)p.msg;
        if(sErrMsg.find( "ORA-03114")!=string::npos )
        {
            //和数据库连接中断
            LOG(6,"DB Connect close " );
            db.logoff();
            m_iDBFlag = 0;
        }

        iRet = -1;
    }
    if( db.connected!=1)
        return -2;

    return iRet;
}


int OracleProcess::ProcessSql(string & sql)
{
    LOG(6,"Processsql [%s]",sql.c_str());
    if( db.connected!=1)
        return -1;
    int iRet = 0;
    try{
        LOG(5,"data 2 [%s]",sql.c_str());

        db<<sql.c_str();
        db.commit();
    }
    catch(otl_exception& p)
    { // intercept OTL exceptions
        LOG(6,"OracleProcess ProcessSql[%s][%s]",p.msg,p.stm_text);
        cerr<<p.msg<<endl; // print out error message
        cerr<<p.stm_text<<endl; // print out SQL that caused the error
        cerr<<p.var_info<<endl; // print out the variable that caused the error
        string sErrMsg = (const  char*)p.msg;
        if(sErrMsg.find( "ORA-03114")!=string::npos )
        {
            //和数据库连接中断
            LOG(6,"DB Connect close " );
            db.logoff();
            m_iDBFlag = 0;
        }

        iRet = -1;
    }
    if( db.connected!=1)
        return -2;

    return iRet;
}

int OracleProcess::ProcessSql3(string & sql,otl_stream & osi ,int iSize )
{
    if( db.connected!=1)
        return -2;
    try{

        osi.open(iSize,sql.c_str() ,db);
    }
    catch(otl_exception& p)
    { // intercept OTL exceptions
        LOG(6,"OracleProcess ProcessSql3[%s][%s]",p.msg,p.stm_text);
        cerr<<p.msg<<endl; // print out error message
        cerr<<p.stm_text<<endl; // print out SQL that caused the error
        cerr<<p.var_info<<endl; // print out the variable that caused the error

        string sErrMsg =(const  char*)p.msg;

        if(sErrMsg.find( "ORA-03114")!=string::npos )
        {
            //和数据库连接中断
            LOG(6,"DB Connect close " );
            db.logoff();
            m_iDBFlag = 0;
        }

        return -1;
    }
    LOG(5,"ProcessSql3 2 [%s]",sql.c_str());

    if( db.connected!=1)
        return -2;
    return 0;
}

int OracleProcess::Close()
{
    db.logoff();
    return 0;
}
