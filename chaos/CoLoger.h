#ifndef __CHAOS_LOGER_H
#define __CHAOS_LOGER_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string>
#include <list>

typedef std::list<std::string> LogList;
typedef std::list<std::string>::iterator LogIter;

namespace chaos {
	#define MAX_LOG_FILE_NAME_LEN  255

	typedef struct Log{
		FILE	*file;
		char	filename[MAX_LOG_FILE_NAME_LEN];

		/* 0 -> shift by size,  1 -> shift by LogCount, 
		 * 2 -> shift by interval, 3 ->shift by day, 
		 * 4 -> shift by hour, 5 -> shift by min
		 */
		int     shiftType;     
		int	    maxLogNum;
		long	maxSize;
		long	maxCount;
		long	logCount;
		time_t	lastShiftTime;
	}Log;

	enum {
		CO_LOG_ERROR = 2,
		CO_LOG_DEBUG = 10,
	};

	class CoLoger {
		public:
			CoLoger();
			~CoLoger();

			int doLog(const char* format, ...);
			int doLogBuf(char * buf, size_t len);

			int init(char* filename, long shiftType, long maxLogNum, long max);
            void start();

		private:
			int shiftFiles();
			char *dateTimeStr(time_t *mytime);
		public:
			Log		mLog;
            int doThreadLog();
            pthread_t m_thread;
            uint32_t m_uThreadCount;

        private:

            pthread_mutex_t m_Mutex;
            LogList m_listProducer;
            LogList m_listCustomer;
            bool m_bStop;
            char* m_pBuff;
            uint32_t m_uCurrLen;

            
	};

};

#endif
