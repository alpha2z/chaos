#ifndef __CHAOS_ARRAY_H
#define __CHAOS_ARRAY_H

#include "CoMmpool.h"

namespace chaos {

	class CoArray {
		public:
			CoArray(int maxCount);
			~CoArray();

			
			int append(void * value);
			
			const void * get(int index);
			void * del(int index);

		public:
			int 	mCount;
			int 	mMaxCount;

			void 	**mFirst;
			CoMmpool  *mPool;
	};

};
#endif
