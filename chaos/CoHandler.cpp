#include <assert.h>
#include "CoHandler.h"
#include "CoCycle.h"

using namespace chaos;

extern CoCycle  *g_cycle;

CoHandler::CoHandler()
{
}

CoHandler::~CoHandler()
{
}

int CoHandler::addPkgWithTimeoutEvent(uint32_t pkgID, void *arg)
{
	CoCycle *cycle = g_cycle;

	map<uint32_t, void *>::iterator iter;
	iter = mPkgArgMap.find(pkgID);

	if(iter != mPkgArgMap.end())    return -1;
	
	struct event *ev = (struct event *) cycle->mPool->alloc(sizeof(struct event));

	if(ev == NULL)	return -2;

	struct timeval tv;
	tv.tv_usec = 0;
	tv.tv_sec = mWriteTimeoutSec;

	uintptr_t   * argList = (uintptr_t *)cycle->mPool->alloc(sizeof(uintptr_t)*4);
	argList[0] = (uintptr_t) arg;
	argList[1] = (uintptr_t) ev;
	argList[2] = (uintptr_t) this;
	argList[3] = (uintptr_t) pkgID;

	evtimer_set(ev, handlePkgWithTimeoutCB, argList);
	event_base_set(cycle->mBase, ev);

	evtimer_add(ev, &tv);

	mPkgArgMap.insert(pair<uint32_t, uintptr_t *> (pkgID, argList));
	return 0;
}

void * CoHandler::clearPkgWithTimeoutEvent(uint32_t pkgID)
{
	CoCycle *cycle = g_cycle;

	map<uint32_t, void *>::iterator iter;
	iter = mPkgArgMap.find(pkgID);

	if(iter == mPkgArgMap.end())	return NULL;

	uintptr_t * argList = (uintptr_t *) iter->second;

	assert(argList != NULL);

	struct event *ev = (struct event *) argList[1];

	if(ev != NULL) {
		evtimer_del(ev);

		cycle->mPool->free(ev);
		ev = NULL;
	}

	mPkgArgMap.erase(iter);

	void * arg = (void *) argList[0];

	cycle->mPool->free(argList);
	argList = NULL;

	return arg;
}

void CoHandler::handlePkgWithTimeoutCB(int fd, short events, void *arg)
{
	uintptr_t * argList = (uintptr_t *) arg;
	void * a = (void *)argList[0];
	CoHandler *handler = (CoHandler *) argList[2];

	if(handler == NULL) return;

	uint32_t pkgID = (uint32_t) argList[3];

	handler->handlePkgWithTimeout(pkgID, a);
}

