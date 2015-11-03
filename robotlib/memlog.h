#pragma once 

#include "solo_log.h"
#include <string>
#include <sys/shm.h>

class ShmLog: public solo::verbose_log {

	private:
		solo::verbose_log::open ;
		solo::verbose_log::close ;

	private:
		key_t shmkey ;
		int   shmId ;
		size_t index ;
		
		static const size_t lineLen = 1000;
		static const size_t lineCount = 1000 ;

		struct LogBuffer {
			size_t endIndex ;
			size_t startIndex ;
			char lines[lineCount][lineLen] ;
		} * logBuffer ;
		void flush_impl(void) ;

	public:
		bool open(key_t shmkey, const std::string & filename) ;
		void close(void) ;
		ShmLog(void) ;
		~ShmLog(void) ;
} ;


