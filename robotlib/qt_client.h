
#include <iostream>

#include "CoHandler.h"

#include "struct.h"
#include "mc_msg.h"
#include "mc_login.h"
#include "mc_transform_msg.h"
#include "mc_shift_route.h"
#include "mc_pass.h"
#include "mc_getTxsession.h"
#include "qt_check.h"
#include "qt_sync.h"
#include "qt_msg.h"

#define QT_HELLO_STATUS_DOING 1
#define QT_HELLO_STATUS_DONE 2

class CQtClient : public chaos::CoHandler {
	public:
		CQtClient();
		virtual ~CQtClient();
	public:
		virtual int handle(chaos::CoConnection *c); 
		virtual void close(chaos::CoConnection *c);
		virtual void ready(chaos::CoConnection *c);
		virtual void timeout(chaos::CoConnection *c);
		virtual void finish(chaos::CoConnection *c) { return ; }

		virtual void handlePkgWithTimeout(uint32_t pkgID, void *arg);

		static void doHello(int fd, short events, void *arg);

	private:
		int parseHead(chaos::CoConnection *c);


    private:
		CQtCheck				mCheck;
		CQtSync					mQtSync;
		CQtMsg					mQtMsg;

};

