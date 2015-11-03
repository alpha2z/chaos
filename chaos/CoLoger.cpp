#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdlib.h>

#include "CoLoger.h"
#include <assert.h>

#define BUFF_LENGTH 1024

using namespace chaos;

#include <iostream>
using namespace std;

CoLoger::CoLoger() {
    pthread_mutex_init(&m_Mutex, NULL);
    m_listProducer.clear();
    m_listCustomer.clear();
    m_bStop = false;

    m_uCurrLen = BUFF_LENGTH; 
    m_pBuff = (char *)malloc(m_uCurrLen);
    memset(m_pBuff, 0, m_uCurrLen);
    assert(m_pBuff[m_uCurrLen - 1] == 0);
    m_uThreadCount = 0;

}

CoLoger::~CoLoger() {
    pthread_mutex_destroy(&m_Mutex);
    m_bStop = true;
    ::free(m_pBuff);
}

int CoLoger::shiftFiles()
{
	struct stat fileStat;
	
	int i;
	struct tm logTm, shiftTm;

	char filename[MAX_LOG_FILE_NAME_LEN];
	char newFilename[MAX_LOG_FILE_NAME_LEN];

	memset(filename, 0, sizeof(filename));
	memset(newFilename, 0, sizeof(newFilename));

	strncat(filename, mLog.filename, sizeof(filename) - 10);
    strcat(filename, ".log");
	
	if(stat(filename, &fileStat) < 0) {
        cout<<"stat file failed: "<<filename<<endl;
        return -1;
    }

	switch (mLog.shiftType) {
		case 0:
			if (fileStat.st_size < mLog.maxSize)
                return 0;
			break;
		case 2:
			if (fileStat.st_mtime - mLog.lastShiftTime < mLog.maxCount) return 0;
			break;
		case 3:
			if (mLog.lastShiftTime - fileStat.st_mtime > 86400)
                break;

			memcpy(&logTm, localtime(&fileStat.st_mtime), sizeof(logTm));
			memcpy(&shiftTm, localtime(&mLog.lastShiftTime), sizeof(shiftTm));
			if (logTm.tm_mday == shiftTm.tm_mday)
                return 0;
			break;
		case 4:
			if (mLog.lastShiftTime - fileStat.st_mtime > 3600) break;
			memcpy(&logTm, localtime(&fileStat.st_mtime), sizeof(logTm));
			memcpy(&shiftTm, localtime(&mLog.lastShiftTime), sizeof(shiftTm));
			if (logTm.tm_hour == shiftTm.tm_hour) return 0;
			break;
		case 5:
			if (mLog.lastShiftTime - fileStat.st_mtime > 60) break;

			memcpy(&logTm, localtime(&fileStat.st_mtime), sizeof(logTm));

			memcpy(&shiftTm, localtime(&mLog.lastShiftTime), sizeof(shiftTm));

			if (logTm.tm_min == shiftTm.tm_min) return 0;
			break;
		default:
			if (mLog.logCount < mLog.maxCount) return 0;
			mLog.logCount = 0;
	}

	// fclose(mLog.plog_s);

	for(i = mLog.maxLogNum - 2; i >= 0; i--)
	{
		if (i == 0)
			sprintf(filename,"%s.log", mLog.filename);
		else
			sprintf(filename,"%s%d.log", mLog.filename, i);
			
		if (access(filename, F_OK) == 0)
		{
			sprintf(newFilename, "%s%d.log", mLog.filename, i+1);
			if (rename(filename, newFilename) < 0 )
			{
				return -1;
			}
		}
	}

	// if ((mLog.plog_s = fopen(filename, "a+")) == NULL) return -1;
	time(&mLog.lastShiftTime);
	return 0;
}

void * thread_logic(void * arg) {
    CoLoger *pLogger = (CoLoger *)arg;
    pLogger->doThreadLog();
    pthread_exit(&pLogger->m_thread);
    pLogger->m_uThreadCount--;
}

int CoLoger::init(char* filename, long shiftType, long maxLogNum, long max)
{
	memset(&mLog, 0, sizeof(Log));
	// if ((mLog.plog_s = fopen(filename, "a+")) == NULL) return -1;
	strncpy(mLog.filename, filename, sizeof(mLog.filename)-1);

	mLog.shiftType = shiftType;
	mLog.maxLogNum = maxLogNum;
	mLog.maxSize = max;
	mLog.maxCount = max;
	mLog.logCount = 0;
	time(&mLog.lastShiftTime);

    //return shiftFiles();
    return 0;
}

void CoLoger::start() {
    assert(m_uThreadCount == 0);
    if (pthread_create(&m_thread, NULL, thread_logic, this)) {
        fprintf(stderr, "create thread failed");
        exit(1);
    }
    cout<<"log thread start"<<endl;
    m_uThreadCount++;
}

char *CoLoger::dateTimeStr(time_t *mytime)
{	
	static char s[50];	
	struct tm curr;		
	curr = *localtime(mytime);	
	
	if (curr.tm_year > 50)	{		
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",	
				curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
				curr.tm_hour, curr.tm_min, curr.tm_sec);	
	}	
	else	
	{		
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
				curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
				curr.tm_hour, curr.tm_min, curr.tm_sec);	
	}					
	
	return s;
}

int CoLoger::doLog(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    uint32_t len = vsnprintf(m_pBuff, m_uCurrLen, fmt, ap);
    va_end(ap);

    if (m_uCurrLen < len) {
        ::free(m_pBuff);
        m_uCurrLen = len * 2;
        m_pBuff = (char *)malloc(m_uCurrLen);
        assert(m_pBuff);

        va_start(ap, fmt);
        len = vsnprintf(m_pBuff, len, fmt, ap);
        va_end(ap);

    }

    assert(len < m_uCurrLen);
    std::string str(m_pBuff, len);

    pthread_mutex_lock(&m_Mutex);
    m_listProducer.push_back(str);
    pthread_mutex_unlock(&m_Mutex);

    return 0;
}

int CoLoger::doThreadLog() {

    assert(shiftFiles() == 0);

    bool bDeal;
    std::string strLog;
    char fn[MAX_LOG_FILE_NAME_LEN];
    while(!m_bStop) {
        bDeal = false;
        pthread_mutex_lock(&m_Mutex);
        if (m_listProducer.size() > 0) {
            bDeal = true;
            for (LogIter iter = m_listProducer.begin(); iter != m_listProducer.end(); iter++) {
                strLog = *iter;
                m_listCustomer.push_back(strLog);
            }
            m_listProducer.clear();
        }
        pthread_mutex_unlock(&m_Mutex);

        struct timeval log_tv;
        for (LogIter iter = m_listCustomer.begin(); iter != m_listCustomer.end(); iter++) {
            strLog = *iter;

            memset(fn, 0, sizeof(fn));
            strncat(fn, mLog.filename, sizeof(fn) - 10);
            strcat(fn, ".log");

            if ((mLog.file = fopen(fn, "a+")) == NULL) {
                fprintf(stderr, "open file %s failed\n", fn);
                std::cout<<"open file failed"<<std::endl;
                exit(1);
            }

            gettimeofday(&log_tv, NULL);
            fprintf(mLog.file, "[%s.%.6ld] ", dateTimeStr(&(log_tv.tv_sec)), log_tv.tv_usec);
            fprintf(mLog.file, "%s\n", strLog.c_str());

            mLog.logCount++;
            fclose(mLog.file);
            assert(shiftFiles() == 0);
        }
        
        m_listCustomer.clear();
        if (!bDeal && !m_bStop) {
            usleep(2500);
        }
    }
    std::cout<<"do thread exit"<<std::endl;
    return 0;
}

int CoLoger::doLogBuf(char * buf, size_t len)
{
	register size_t pos = 0;
	int linelen = 16;

	int linecount = len / linelen + 1;

	int tmpbuflen = linecount * (6 + 4*linelen);
	char *tmpbuf = (char *)malloc(tmpbuflen);

	char line[linelen + 1] ;

	memset(line, 0, sizeof(line));
	memset(tmpbuf, 0, tmpbuflen);
	
	size_t i = 0;
	sprintf(tmpbuf + pos, "\t");
	pos++;

	u_char tmpchar = 0;

	for (i = 0; i < len; i++) {

		if ((i % linelen == 0 && (int)i != 0) ) {
			sprintf(tmpbuf + pos, "\t|\t%s\n\t", line);
			pos += 5 + strlen(line);
			memset(line, 0, sizeof(line));

		}

		if(buf[i] > 0x1f)
			line[i % linelen] = buf[i];
		else
			line[i % linelen] = '.';

		tmpchar = buf[i];
		
		sprintf(tmpbuf + pos, "%.2x ", tmpchar);
		pos += 3;
	

	}
	
	if(len > 0) {
		size_t fill = len % linelen ;
		if(fill != 0) {
			for(i = 0; i < 3*(linelen - fill); i++)
			{
				tmpbuf[pos++] = ' ';
			}
		}
	}

	sprintf(tmpbuf + pos, "\t|\t%s", line);
	pos += 4 + strlen(line);

	doLog("\n%s\n", tmpbuf);
	
	free(tmpbuf);
	tmpbuf = NULL;

	return 0;
}
