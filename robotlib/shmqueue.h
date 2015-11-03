#ifndef SHMQUEUE_H
#define SHMQUEUE_H

#include "stdio.h"
#include "shmem.h"

using namespace std;


class ShmQueue 
{
public:
	ShmQueue(key_t shkey ,long int count ,int iItemSize );
	int GetItem(void* pItem ,int iSize);
	int PutItem(void* pItem ,int iSize);
	int IsEmpty();
	int GetItemCount();
	int ClearAll();
	int ShowItem(char*sOut, int iItem );
	int ShowHead();
	int ShowTail();
	bool IsCreated();
private:
	int d_QueueItemSize; 
	int d_QueueSize;   
	int d_MaxItemCount;    
	int* d_piQueueHead; 
	int* d_piQueueTail; 
	void *d_pQueue;  
	bool	d_bCreated;
};

#endif
