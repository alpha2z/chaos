#include <sstream>
#include <assert.h>
#include <CoCycle.h>
#include <CoData.h>
#include "pcl.h"
#include "struct.h"
#include "data_channel_mng.h"

using namespace std;
using namespace chaos;

extern	CoCycle	* g_cycle;

CDataChannelMng::CDataChannelMng() {	mSeq = 0; }
CDataChannelMng::~CDataChannelMng() {}

int CDataChannelMng::add(string key, key_t shmkey, long int count, int itemSize, uint8_t type) 
{
	int is = itemSize + sizeof(uint32_t);   //add size of seq 
	DataQueue * queue = (DataQueue *) new DataQueue(shmkey, count, is);

	assert(queue != NULL);
/*
	if(!queue->shmQueue.IsCreated()) {
		delete queue;
		queue = NULL;
		return -1;
	}
*/
	queue->itemSize = is;

	if(type == DATA_CHANNEL_TYPE_REQ) {
		mReqChannelMap.insert(pair<string, DataQueue *>(key, queue));
	} else {
		mRespChannelMap.insert(pair<string, DataQueue *>(key, queue));
	}

	return  0;
}

int CDataChannelMng::del(string key, uint8_t type)
{
	map<string, DataQueue *>::iterator iter;

	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -1;
	} else {
		iter = mRespChannelMap.find(key);
		if(iter == mRespChannelMap.end())	return -1;
	}

	DataQueue *queue = iter->second;

	if(queue != NULL) {
		delete queue;
		queue = NULL;
	}

	if(type == DATA_CHANNEL_TYPE_REQ) {
		mReqChannelMap.erase(iter);
	} else {
		mRespChannelMap.erase(iter);
	}

	return 0;
}

int CDataChannelMng::clear(string key, uint8_t type)
{
	map<string, DataQueue *>::iterator iter;
	
	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -1;
	} else {
		iter = mRespChannelMap.find(key);
		if(iter == mRespChannelMap.end())   return -1;
	}

	DataQueue *queue = iter->second;

	if(queue != NULL) {
		DataQueue *queue = iter->second;
		queue->shmQueue.ClearAll();
	}

	return 0;
}

int CDataChannelMng::get(string key, void * item, uint8_t type, uint32_t &seq)
{
	CoCycle *cycle = g_cycle;
	map<string, DataQueue *>::iterator  iter;
	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -1;
	}else {
		iter = mRespChannelMap.find(key);

		if(iter == mRespChannelMap.end()) return -1;
	}

	DataQueue * queue = iter->second;

	void * itemWithSeq = cycle->mPool->alloc(queue->itemSize);
	assert(itemWithSeq != NULL);

	memset((char *)itemWithSeq, 0, queue->itemSize);
	size_t len = queue->itemSize - sizeof(uint32_t);

	int offset = 0;
	CoData data;

	int ret = getItem(key, itemWithSeq, type);

	if(ret == 0) {
		data.get((char *)itemWithSeq, offset, seq);
		data.get((char *)itemWithSeq, offset, (char *)item, len);
	}

	cycle->mPool->free(itemWithSeq);
	itemWithSeq = NULL;
	return ret;
}

int CDataChannelMng::getItem(string key, void * item, uint8_t type)
{
	map<string, DataQueue *>::iterator iter;

	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -1;
	}else {
		iter = mRespChannelMap.find(key);
		
		if(iter == mRespChannelMap.end()) return -1;
	}

	DataQueue * queue = iter->second;

	if(queue == NULL)	return -2;

	int is = queue->itemSize;

	if((queue->shmQueue.GetItem(item, is)) < 0)	return -3;

	LOG(7, "get item");
	LOG_BUF(7, (char *)item, is);
	return 0;
}

int CDataChannelMng::put(string key, void * item, uint8_t type, uint32_t seq)
{
	CoCycle *cycle = g_cycle;

	map<string, DataQueue *>::iterator  iter;

	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -1;
	}else {
		iter = mRespChannelMap.find(key);

		if(iter == mRespChannelMap.end()) return -2;
	}

	DataQueue * queue = iter->second;

	void * itemWithSeq  = cycle->mPool->alloc(queue->itemSize);
	assert(itemWithSeq != NULL);

	memset((char *)itemWithSeq, 0, queue->itemSize);
	
	size_t len = queue->itemSize - sizeof(uint32_t);

	int ret = 0;
	int offset = 0;
	CoData data;
	data.put((char *)itemWithSeq, offset, seq);
	data.put((char *)itemWithSeq, offset, (char *)item, len);

	ret = putItem(key, itemWithSeq, type); 

	LOG(7, "put ret:%d, seq:%d", ret, seq);
	cycle->mPool->free(itemWithSeq);
	item = NULL;

	return ret;
}

int CDataChannelMng::putItem(string key, void * item, uint8_t type)
{
	map<string, DataQueue *>::iterator iter;

	if(type == DATA_CHANNEL_TYPE_REQ) {
		iter = mReqChannelMap.find(key);
		if(iter == mReqChannelMap.end()) return -10;
	} else {
		iter = mRespChannelMap.find(key);
		if(iter == mRespChannelMap.end()) return -11;
	}

	DataQueue * queue = iter->second;

	if(queue == NULL)   return -12;

	int is = queue->itemSize;

	if((queue->shmQueue.PutItem(item, is)) < 0)    return -13;

	LOG_BUF(7, (char *)item, is);
	return 0;
}


unsigned int CDataChannelMng::getSeq() {
	if(mSeq >= robot::MAX_UINT32) {
		mSeq = 0;
	} else {
		++mSeq;
	}

	return mSeq;
}

map<string, DataQueue *> * CDataChannelMng::getReqMap()
{
	return &mReqChannelMap;
}

map<string, DataQueue *> * CDataChannelMng::getRespMap()
{
	return &mRespChannelMap;
}

map<uint32_t, void *> * CDataChannelMng::getArgMap()
{
	return &mArgMap;
}

string CDataChannelMng::getItemInfo()
{
	map<string, DataQueue *>::iterator reqIter;
	map<string, DataQueue *>::iterator respIter;

	stringstream	ss;

	ss << " req channel item info:" << endl;
	for(reqIter = mReqChannelMap.begin(); reqIter != mReqChannelMap.end(); reqIter++) {
		DataQueue * dq = reqIter->second;
		ss << " item key: " << reqIter->first << ", size: " << dq->shmQueue.GetItemCount() << endl;
	}

	ss << " resp channel item info:" << endl;
	for(respIter = mRespChannelMap.begin(); respIter != mRespChannelMap.end(); respIter++) {
		DataQueue * dq = respIter->second;
		ss << " item key: " << respIter->first << ", size: " << dq->shmQueue.GetItemCount() << endl;
	}

	return ss.str();
}
