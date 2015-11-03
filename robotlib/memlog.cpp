#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "memlog.h"

using namespace std ; 
using namespace solo ;

void ShmLog::flush_impl(void) {
	if (NULL == logBuffer) {
		return ;
	}

	const string str = buffer().str() ;
	size_t & si = logBuffer->startIndex ;
	size_t & ei = logBuffer->endIndex ;

	size_t len = str.length() ;
	if (len >= lineLen) {
		len = lineLen - 1 ;
	}
	memcpy(logBuffer->lines[ei], str.data(), len) ;
	logBuffer->lines[ei][len] = '\0' ;

	/*
	snprintf(logBuffer->lines[ei],lineLen,"%s",str) ;
	logBuffer->lines[ei][lineLen - 1] = '\0' ;

	cout << si << ":" << ei << " " << logBuffer->lines[ei] ; */

	++ei ; ei %= lineCount ;
	if (ei == si) {
		++si ; si %= lineCount ;
	}
}

ShmLog::ShmLog(void) {
	shmId = -1 ;
	logBuffer = NULL ;
}

void ShmLog::close(void) {
	if (logBuffer == NULL) {
		return ; 
	}
	logBuffer->startIndex = logBuffer->endIndex ;
	shmdt(logBuffer) ;
	logBuffer = NULL ;
	shmId = -1 ;
}

ShmLog::~ShmLog(void) {
	close() ;
}

bool ShmLog::open(key_t key, const std::string & filename) {
	bool created = false ;
	shmId = shmget(key, sizeof(LogBuffer), 0666) ;
	if (-1 == shmId) {
		shmId = shmget(key, sizeof(LogBuffer), 0666|IPC_CREAT) ;
		if (-1 == shmId) {
			cerr << "Failed to get shm memory id" << endl ;
			return true ;
		}
		created = false ;
	}
	logBuffer = static_cast<LogBuffer*>(shmat(shmId, NULL, 0666) ) ;
	if  ( reinterpret_cast<void*>(static_cast<intptr_t>(-1)) == static_cast<void*>(logBuffer) ) {
		cerr << "Failed to get shm memory" << endl ;
		return false ;
	}

	if (!created) {
		/* dump */
		cout << "Old memory log exist" << endl ;
		if (filename.length() == 0) {
			cerr << "warning: no memlog will be dumped" << endl ;
			return true ;
		}

		ofstream of ;
		of.open(filename.c_str(), ios::trunc) ;
		if (!of) {
			cerr << "Failed to open ShmLogDumpFile" << endl ;
		}

		size_t & si = logBuffer->startIndex ;
		size_t & ei = logBuffer->endIndex ;
		
		while(si != ei) {
			logBuffer->lines[si][lineLen - 1] = '\0' ;
			of << logBuffer->lines[si] ;

			++si ;
			si %= lineCount ;
		}
		cout << "Success: dump memory log to file:" << filename << endl ;
	} else {
		cout << "New memroy log is created" << endl ;
	}

	/* init */
	{
		logBuffer->startIndex = 0 ; 
		logBuffer->endIndex = 0 ; 
		for (size_t i = 0 ; i < lineCount ; ++i) {
			logBuffer->lines[i][0] = '\0' ;
		}
	}


	return true ;
}

