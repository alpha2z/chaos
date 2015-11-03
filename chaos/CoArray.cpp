#include <string.h>
#include <iostream>

#include "CoCycle.h"
#include "CoArray.h"
#include "CoLoger.h"

using namespace chaos;
using namespace std;

extern CoCycle *g_cycle;

CoArray::CoArray(int maxCount)
{
	CoCycle *cycle = g_cycle;

	mMaxCount = maxCount;
	mCount = 0;
	
	mFirst = (void**)cycle->mPool->alloc(sizeof( void * ) * maxCount );
	if(mFirst == NULL)	{

		cycle->mPool->free(mFirst);
	}

}

CoArray::~CoArray()
{
	CoCycle *cycle = g_cycle;

	if(mFirst != NULL)
		cycle->mPool->free(mFirst);
}

int CoArray::append(void * value)
{
	if(value == NULL)	return -1;

	if(mCount >= mMaxCount)	return -2;

	mFirst[mCount++] = value;

	return 0;
}

const void * CoArray::get(int index)
{
	const void *ret = NULL;
	
	if(index < 0 || index >= mCount)	return ret;

	ret = mFirst[index];
	return ret;
}

void * CoArray::del(int index)
{
	void *ret = NULL;

	if(index < 0 || index >= mCount)  return ret;

	ret = mFirst[index];

	mCount--;

	if( (index + 1) < mMaxCount ) {
		memmove(mFirst + index, mFirst + index + 1,
				(mMaxCount - index - 1 ) * sizeof( void * ) );
	} else {
		mFirst[index] = NULL;
	}
	return ret;
}
