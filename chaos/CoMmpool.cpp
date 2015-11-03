#include <stdint.h>
#include <assert.h>
#include <iostream>

#include "CoMmpool.h"
#include "CoConstants.h"
#include "CoCycle.h"

using namespace std;
using namespace chaos;

#define CHAOS_ALIGN_PTR(p, a)   \
	(u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

CoMmpool::CoMmpool() { mAllocNum = 0; mFreeNum = 0; }
CoMmpool::~CoMmpool() {}
extern CoCycle *g_cycle;

void * CoMmpool::alloc(size_t  size)
{
	mAllocNum++;
    //LOG(2, "mAllocNum :%d, mFreeNum :%d, gap:%d, size:%d", mAllocNum, mFreeNum, mAllocNum - mFreeNum, size);
	return malloc(size);
}

void CoMmpool::free(void *p)
{
	mFreeNum++;
	::free(p);
}
#if 0
int CoMmpool::create(size_t size)
{
	mPool = (Pool *)malloc(size);
	if (mPool == NULL)  return -1;

	mPool->data.last = (u_char *) mPool + sizeof(Pool);
	mPool->data.end = (u_char *) mPool + size;
	mPool->data.next = NULL;

	size = size - sizeof(Pool);
	mPool->max = (size < (size_t)MAX_ALLOC_FROM_POOL) ? size : MAX_ALLOC_FROM_POOL;

	mPool->current = mPool;
	mPool->large = NULL;

	return 0;
}

void CoMmpool::destroy()
{
	Pool          *p, *n;
	PoolLarge     *l;

	for (l = mPool->large; l; l = l->next) {

		if (l->alloc) {
			free(l->alloc);
		}
	}

	for (p = mPool, n = mPool->data.next; /* void */; p = n, n = n->data.next) {
		free(p);

		if (n == NULL) {
			break;
		}
	}

}

void CoMmpool::reset()
{
	Pool        *p;
	PoolLarge  *l;

	for (l = mPool->large; l; l = l->next) {
		if (l->alloc) {
			free(l->alloc);
		}
	}

	mPool->large = NULL;

	for (p = mPool; p; p = p->data.next) {
		p->data.last = (u_char *) p + sizeof(Pool);
	}
}

void * CoMmpool::alloc(size_t  size)
{
	u_char      *m;
	Pool      	*p;

	mAllocNum++;
	if (size <= mPool->max) {

		p = mPool->current;

		do {
			m = CHAOS_ALIGN_PTR(p->data.last, sizeof(unsigned long));

			if ((size_t) (p->data.end - m) >= size) {
				p->data.last = m + size;
				return m;
			}

			p = p->data.next;

		} while (p);

		return allocBlock(size);
	}

	return (void *)allocLarge(size);
}

void CoMmpool::free(void *p)
{
	PoolLarge  *l;

	mFreeNum++;

	for (l = mPool->large; l; l = l->next) {
		if (p == l->alloc) {
			::free(l->alloc);
			l->alloc = NULL;

			return;
		}
	}
	assert("pool free fail");
}

void * CoMmpool::allocBlock(size_t size)
{
	u_char      *m;
	size_t       psize;
	Pool      *p, *newnode, *current;

	psize = (size_t) (mPool->data.end - (u_char *) mPool);

	m = (u_char *)malloc(psize);
	if (m == NULL) {
		return NULL;
	}

	newnode = (Pool *) m;

	newnode->data.end = m + psize;
	newnode->data.next = NULL;
	newnode->data.failed = 0;
	m += sizeof(PoolData);
	m = CHAOS_ALIGN_PTR(m, sizeof(unsigned long));
	newnode->data.last = m + size;

	current = mPool->current;

	for (p = current; p->data.next; p = p->data.next) {
		if (p->data.failed++ > 4) {
			current = p->data.next;
		}
	}

	p->data.next = newnode;

	mPool->current = current ? current : newnode;

	return m;
}

void * CoMmpool::allocLarge(size_t size)
{
	void              *p;
	uintptr_t         n;
	PoolLarge        *large;

	p = malloc(size);
	if (p == NULL) {
		return NULL;
	}

	n = 0;

	for (large = mPool->large; large; large = large->next) {
		if (large->alloc == NULL) {
			large->alloc = p;
			return p;
		}

		if (n++ > 3) {
			break;
		}
	}

	large = (PoolLarge *)alloc(sizeof(PoolLarge));
	if (large == NULL) {
		::free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = mPool->large;
	mPool->large = large;

	return p;
}
#endif
