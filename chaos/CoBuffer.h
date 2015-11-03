#ifndef __CHAOS_BUFFER_H
#define __CHAOS_BUFFER_H

#include <stdlib.h>
#include <stdint.h> 
#include <string>

using namespace std;

struct evbuffer;

namespace chaos {

class CoBuffer {
	public:
		CoBuffer();
		virtual ~CoBuffer();

	public:
		int put(char data);
		int get(char &data);

		int put(int8_t data);
		int get(int8_t &data);
		
		int put(uint8_t data);
		int get(uint8_t &data);
		
		int put(int16_t data);
		int get(int16_t &data);
		
		int put(uint16_t data);
		int get(uint16_t &data);
		
		int put(int32_t data);
		int get(int32_t &data);
		
		int put(uint32_t data);
		int get(uint32_t &data);
		
		int put(int64_t data);
		int get(int64_t &data);
		
		int put(uint64_t data);
		int get(uint64_t &data);
	
		int put(const void *data, size_t dataLen);
		int get(char *data, size_t dataLen);

		int put(string &data, int prefixType = 0);
		int get(string &data, int prefixType = 0);
	public:
		void clone(const CoBuffer *buffer);
		int append(const void * buffer, int len = 0);
		int append(const CoBuffer * buffer);
		void drain(int len);
		void reset();
		size_t getSize() const;
		char * getLine();

		void logBuf(int level);

		struct sockaddr_in *getClientAddr();
		void setClientAddr(struct sockaddr_in & addr);
		/*
		 *   Find a string within an buffer.
		 *   @param what the string to be searched for
		 *   @param len the length of the search string
		 *   @return a pointer to the beginning of the search string, or NULL if the search failed.
		 *  */
		u_char * find(const u_char *, size_t);

		struct evbuffer * getBuffer() const;
		const u_char * getData() const;

		void hexShow(int flag);

		int read( int fd );
		int write( int fd );

	private:
		struct evbuffer * mBuffer;

};

};

#endif
