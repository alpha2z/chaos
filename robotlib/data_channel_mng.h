#ifndef __DATA_CHANNEL_MNG_H
#define __DATA_CHANNEL_MNG_H

#include <string>
#include <map>

#include "shmqueue.h"

struct DataQueue{
	int itemSize;
	ShmQueue shmQueue;

	DataQueue(key_t shmkey, long int count, int itemSize):
		shmQueue(shmkey, count, itemSize)
	{} ;
};

#pragma pack(push, 1)
struct DataItem {
	uint32_t	seq;
	void *item;
};
#pragma pack(push, 0)

class CDataChannelMng
{
	public:
		enum {
			DATA_CHANNEL_TYPE_REQ = 0,
			DATA_CHANNEL_TYPE_RESP = 1,
		};

	public:
		CDataChannelMng();
		virtual ~CDataChannelMng();

		int add(std::string key, key_t shmkey, long int count, int itemSize, uint8_t type);
		int del(std::string key, uint8_t type);
		int clear(std::string key, uint8_t type);

		int putItem(std::string key, void * item, uint8_t type);
		int getItem(std::string key, void * item, uint8_t type);

		int put(std::string key, void * item, uint8_t type, uint32_t seq);
		int get(std::string key, void * item, uint8_t type, uint32_t &seq);

		unsigned int getSeq();

		map<string, DataQueue *> * getReqMap();
		map<string, DataQueue *> * getRespMap();

		map<uint32_t, void *> * getArgMap();

		std::string getItemInfo();

	private:	
		unsigned int mSeq;
		std::map<uint32_t, void *>  mArgMap;

		map<string, DataQueue *>  mReqChannelMap;
		map<string, DataQueue *>  mRespChannelMap;
		
};

#endif
