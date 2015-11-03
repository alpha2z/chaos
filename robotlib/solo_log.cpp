#include "solo_log.h"

namespace solo {
	static log<void>::flush_type __flushlog__ ;
	const log<void>::flush_type & flushlog  = __flushlog__ ;

}

