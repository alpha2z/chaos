#include <iostream>
#include "CoCycle.h"
#include "shmqueue.h"

using namespace std;
using namespace chaos;

extern CoCycle * g_cycle;
ShmQueue::ShmQueue(key_t shkey ,long int count ,int iItemSize )
{
	if(iItemSize <= 12)	{
		cout<<"item size must > 12"<<endl;
		LOG(2, "item size must > 12");
		return ;
	}
	Shmem myQueueShm(shkey ,count*iItemSize);
	char* ptrmem = myQueueShm.GetShareMemory();
	if( ptrmem== NULL )
	{
		d_bCreated = false;
		return ;
	}

	d_bCreated = true;

	d_QueueSize = count*iItemSize;
	if( myQueueShm.iFlag == 1)
	{
		bzero( ptrmem ,count*iItemSize );

		d_MaxItemCount = count - 1;	
		d_QueueItemSize = iItemSize;

		*(int*)ptrmem = d_QueueItemSize;
		*((int*)ptrmem+1) = d_MaxItemCount;		
		*((int*)ptrmem+2) = 0;
		*((int*)ptrmem+3) = 0;
	}
	else
	{
		d_QueueItemSize = *(int*)ptrmem;
		d_MaxItemCount = *((int*)ptrmem+1);	
	}
	
	d_piQueueHead = ((int*)ptrmem+2);
	d_piQueueTail = ((int*)ptrmem+3);

	d_pQueue = ptrmem + d_QueueItemSize;
	
}

int ShmQueue::ShowTail()
{
	return (*d_piQueueTail);
}

int ShmQueue::ShowHead()
{
	return (*d_piQueueHead);
}

int ShmQueue::ShowItem(char * sOut, int iItem)
{
	if(	( iItem>=(*d_piQueueHead) && iItem<(*d_piQueueTail) ) ||
		(((*d_piQueueHead)>(*d_piQueueTail)) &&( iItem>=(*d_piQueueHead)||iItem<(*d_piQueueTail))) )
	{
		memcpy( sOut, (char*)d_pQueue +iItem*d_QueueItemSize, d_QueueItemSize );
		return 0;
	}
	return -1;
}

int ShmQueue::GetItem(void * pItem, int iSize)
{
	if( (*d_piQueueHead) == (*d_piQueueTail) )
		return -1;

	memcpy( pItem , (char*)d_pQueue + (*d_piQueueHead)*d_QueueItemSize , (iSize>d_QueueItemSize)?d_QueueItemSize:iSize );
	(*d_piQueueHead)++;

	if( (*d_piQueueHead) >= d_MaxItemCount )
		(*d_piQueueHead) = 0;

	return 0;
}

int ShmQueue::PutItem(void * pItem, int iSize)
{
	if( ((*d_piQueueTail) < (*d_piQueueHead) && (*d_piQueueHead)==(*d_piQueueTail)+1) ||
		((*d_piQueueHead)==0 && (*d_piQueueTail) >= d_MaxItemCount-1 ) )
		return -1;

	memcpy( (char*)d_pQueue+ (*d_piQueueTail)*d_QueueItemSize , pItem , (iSize>d_QueueItemSize)?d_QueueItemSize:iSize );

	(*d_piQueueTail)++;
	if( (*d_piQueueTail) > d_MaxItemCount -1 )
	{
		(*d_piQueueTail) = 0;
	}
	
	return 0;
}

int ShmQueue::ClearAll()
{
	(*d_piQueueTail) = (*d_piQueueHead) = 0;
	
	return 0;
}

int ShmQueue::GetItemCount()
{
	if( (*d_piQueueHead) <= (*d_piQueueTail) )
		return (*d_piQueueTail) - (*d_piQueueHead);
	else
	{
		return d_MaxItemCount - ((*d_piQueueHead) - (*d_piQueueTail) );
	}
}

int ShmQueue::IsEmpty()
{
	return (*d_piQueueHead) ==(*d_piQueueTail);
}

bool ShmQueue::IsCreated()
{
	return d_bCreated;
}
