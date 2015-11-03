#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "CoCycle.h"
#include "CoConstants.h"

#include <iostream>
using namespace chaos;

CoCycle   *g_cycle = NULL;

void CoCycle::daemon()
{
	int fd;

	signal(SIGALRM, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	if (fork()) exit(0);

	if (setsid() == -1)
		exit(-1);

	for(fd=3;fd<NOFILE;fd++)
		close(fd);

//	chdir("/");
//	umask(0);
/*
	{//ulimit -c iMaxCoreFileSize
		struct rlimit rlim;
		getrlimit(RLIMIT_CORE, &rlim);
		rlim.rlim_max = rlim.rlim_cur = MAX_CORE_FILE_SIZE;
		if(setrlimit(RLIMIT_CORE, &rlim))
		{
			printf("setrlimit core file size failed!\n");
		}
	}

	{//ulimit -s iMaxStackSize
		struct rlimit rlim;
		getrlimit(RLIMIT_STACK, &rlim);
		rlim.rlim_max = rlim.rlim_cur = MAX_STACK_SIZE;
		if(setrlimit(RLIMIT_STACK, &rlim))
		{
			printf("setrlimit stack size failed!\n");
		}
	}*/
    return;
}
