#ifndef __USER_MNG_H
#define __USER_MNG_H

#include "CoCycle.h" 
#include <stdint.h>
#include <string>
#include <map>
#include <list>
#include <queue>
#include <vector>

typedef struct _stUser {
    std::string name;
    std::string systemID;
    uint32_t lastLink;
	struct event	ev;
	struct timeval	tv;
} User;

typedef struct {
	std::string name;
	std::string systemID;
} UserMapKey;

class CUserMng
{
	public:
		enum {
			USER_VIEW_STATUS_READY = 0,
			USER_VIEW_STATUS_ENTER = 1,	
			USER_VIEW_STATUS_WXENTER = 2,
		};
		CUserMng();
		virtual ~CUserMng();
		int insert(User * user) ;
		int remove(UserMapKey key);

		User * getUser(UserMapKey key);
        User * createUser();
		static void userExpire(int fd, short events, void *arg);
	public:	
		std::map<UserMapKey, User *>  mUserMap;
		
};

#endif
