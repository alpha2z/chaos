#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <iostream>

#include "CoCycle.h"
#include "CoBuffer.h"

#include "event.h"

using namespace std;
using namespace chaos;

extern CoCycle * g_cycle;

CoBuffer::CoBuffer()
{
	mBuffer = evbuffer_new();
}

CoBuffer::~CoBuffer()
{
	evbuffer_free( mBuffer );
	mBuffer = NULL;
}

struct sockaddr_in *CoBuffer::getClientAddr()
{
	return &(mBuffer->client_addr);
}

void CoBuffer::setClientAddr(struct sockaddr_in & addr)
{
	mBuffer->client_addr = addr;
}

void CoBuffer::clone(const CoBuffer * buffer)
{
	reset();
	append(buffer->getData(), buffer->getSize());
}

int CoBuffer::append(const void * buffer, int len)
{
//	len = (len < 0 ? strlen( (char*)buffer ) : len);

	return evbuffer_add(mBuffer, (void*)buffer, len);
}

int CoBuffer::append(const CoBuffer * buffer)
{
	return evbuffer_add_buffer(mBuffer, buffer->getBuffer());
}

void CoBuffer::drain(int len)
{
	evbuffer_drain(mBuffer, len);
}

void CoBuffer::reset()
{
	drain(getSize());
}

int CoBuffer::write(int fd)
{
	return evbuffer_write(mBuffer, fd);
}

int CoBuffer::read(int fd)
{
	return evbuffer_read(mBuffer, fd, -1);
}


size_t CoBuffer::getSize() const
{
	return EVBUFFER_LENGTH(mBuffer);
}

char * CoBuffer::getLine()
{
	return evbuffer_readline(mBuffer);
}

u_char * CoBuffer::find(const u_char *what, size_t len)
{
	return evbuffer_find(mBuffer, what, len);
}

struct evbuffer * CoBuffer::getBuffer() const
{
	return mBuffer;
}

const u_char * CoBuffer::getData() const 
{
	if( NULL != EVBUFFER_DATA( mBuffer ) ) {
		return EVBUFFER_DATA( mBuffer );
	} else {
		return (u_char *)"";
	}
}

void CoBuffer::hexShow(int flag)
{
	register size_t count;

	const u_char * buf = getData();

	for (count = 0; count < getSize(); count++) {
		if (count % 25 == 0) printf("\n");
		if (flag && buf[count] > 0x1f) printf("%2c ", buf[count]);
		else printf("%.2x ", buf[count]);
	}
	if ((count - 1) % 25) printf("\n");
}


int CoBuffer::put(char data)
{
	if(append((void *)&data, sizeof(data)) != 0)	return -1;	
	return sizeof(data);
}

int CoBuffer::get(char &data)
{
	if(getSize() < sizeof(data)) return -1;
	evbuffer_remove(mBuffer, (void *)&data, sizeof(data));
	return sizeof(data);
}

int CoBuffer::put(int8_t data)
{
	if(append((void *)&data, sizeof(data)) != 0) return -1;	
	return sizeof(data);
}

int CoBuffer::get(int8_t &data)
{
	evbuffer_remove(mBuffer, (void *)&data, sizeof(data));
	return sizeof(data);
}

int CoBuffer::put(uint8_t data)
{
	if(append((void *)&data, sizeof(data)) != 0) return -1;	
	return sizeof(data);
}

int CoBuffer::get(uint8_t &data)
{
	if(getSize() < sizeof(data)) return -1;
	evbuffer_remove(mBuffer, (void *)&data, sizeof(data));
	return sizeof(data);
}

int CoBuffer::put(int16_t data)
{
	char buf[sizeof(data)] = {0};

	data = htons(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0)	return -1;	

	return sizeof(data);
}

int CoBuffer::get(int16_t &data)
{
	char buf[sizeof(data)] = {0};

	if(getSize() < sizeof(buf)) return -1;
	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohs(*(int16_t *)buf);
	return sizeof(data);
}

int CoBuffer::put(uint16_t data)
{
	char buf[sizeof(data)] = {0};

	data = htons(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0) return -1;	

	return sizeof(data);
}

int CoBuffer::get(uint16_t &data)
{
	char buf[sizeof(data)] = {0};
	if(getSize() < sizeof(buf)) return -1;
	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohs(*(uint16_t *)buf);
	return sizeof(data);
}

int CoBuffer::put(int32_t data)
{
	char buf[sizeof(data)] = {0};

	data = htonl(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0) return -1;	

	return sizeof(data);
}

int CoBuffer::get(int32_t &data)
{
	char buf[sizeof(data)] = {0};
	if(getSize() < sizeof(buf)) return -1;
	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohl(*(int32_t *)buf);
	return sizeof(data);
}


int CoBuffer::put(uint32_t data)
{
	char buf[sizeof(data)] = {0};

	data = htonl(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0) return -1;	

	return sizeof(data);
}

int CoBuffer::get(uint32_t &data)
{
	char buf[sizeof(data)] = {0};
//	cout<<sizeof(data)<<endl;
	if(getSize() < sizeof(buf)) return -1;

	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohl(*(uint32_t *)buf);
	return sizeof(data);
}

int CoBuffer::put(int64_t data)
{
	char buf[sizeof(data)] = {0};

	data = htons(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0) return -1;	

	return sizeof(data);
}

int CoBuffer::get(int64_t &data)
{
	char buf[sizeof(data)] = {0};

	if(getSize() < sizeof(buf)) return -1;

	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohl(*(int64_t *)buf);
	return sizeof(data);
}

int CoBuffer::put(uint64_t data)
{
	char buf[sizeof(data)] = {0};

	data = htons(data);
	memcpy(buf, &data, sizeof(data));
	if(append((void *)buf, sizeof(data)) != 0) return -1;	

	return sizeof(data);
}

int CoBuffer::get(uint64_t &data)
{
	char buf[sizeof(data)] = {0};

	if(getSize() < sizeof(buf))	return -1;

	evbuffer_remove(mBuffer, (void *)buf, sizeof(buf));
	
	data = ntohl(*(uint64_t *)buf);
	return sizeof(data);
}

int CoBuffer::put(const void * data, size_t dataLen)
{
	if(append((void *)data, dataLen) != 0) return -1;

	return dataLen;
}

int CoBuffer::get(char *data, size_t dataLen)
{
	if(data == NULL)	return -1;
	
	return evbuffer_remove(mBuffer, data, dataLen);
}

int CoBuffer::put(string &data, int prefixType)
{
	if(prefixType == 0) {
		int16_t len = 0;
		len = data.length();

		put(len);
		if(append((void *)data.data(), len) != 0) return -1;

		return sizeof(len) + len;
	} else {
		int32_t len = 0;
		len = data.length();

		put(len);
		if(append((void *)data.data(), len) != 0) return -1;

		return sizeof(len) + len;
	}
}

int CoBuffer::get(string &data, int prefixType)
{
	CoCycle *cycle = g_cycle;

	char *d = NULL;

	int l = 0;
	if(prefixType == 0) {
		int16_t len = 0;

		get(len);
		l = len;
	} else {
		int32_t len = 0;

		get(len);
		l = len;
	}

	if((uint16_t)l > getSize())	l = getSize();

	d = (char *)cycle->mPool->alloc(l + 1);
	assert(d != NULL);
	memset(d, 0, l + 1);

	evbuffer_remove(mBuffer, d, l);

	data = "";
	data.append(d, l);

	cycle->mPool->free(d);

    if(prefixType == 0) return sizeof(int16_t) + l;
    else    return sizeof(int32_t) + l;
}

void CoBuffer::logBuf(int level)
{
	CoCycle * cycle = g_cycle;
	if(level < cycle->mLogLevel) {
		cycle->mLoger->doLogBuf((char *)getData(), getSize());

	}
}
