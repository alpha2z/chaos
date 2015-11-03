#ifndef __CHAOS_MM_POOL_H
#define __CHAOS_MM_POOL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

namespace chaos {
	/*
	typedef struct Pool Pool;

	typedef struct PoolLarge{
		PoolLarge	*next;
		void		*alloc;
	}PoolLarge;

	typedef struct PoolData{
		u_char  *last;
		u_char  *end;
		Pool	*next;
		uintptr_t   failed;
	}PoolData;

	typedef struct Pool{
		PoolData	data;
		size_t		max;
		Pool		*current;
		PoolLarge	*large;
	}Pool;
*/
	class CoMmpool {
		public:
			CoMmpool();
			~CoMmpool();
/*
			int create(size_t size);
			void destroy();
			void reset();
*/
			void *alloc(size_t size);
			void free(void *p);

			template<typename T>
			T *newObj() {    
				T *t = (T *) this->alloc(sizeof(T));
			   	return  new (t) T ;
			}

			template<typename T>
			void delObj(T *t) {
				t->~T() ;
				this->free((void *)t);
			}
/*
		private:
			void *allocBlock(size_t size);
			void *allocLarge(size_t size);

		private:
			Pool	*mPool;
*/		public:
			uint32_t	mAllocNum;
			uint32_t	mFreeNum;

	};
}

#endif

