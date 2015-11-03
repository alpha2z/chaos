#include <assert.h>
#include "CoChannel.h"
#include "struct.h"
#include "user_mng.h"
#include "worker_struct.h"

using namespace std;
using namespace robot;
using namespace chaos;

extern CoCycle	* g_cycle;
extern RobotContext * g_context;
extern WorkerContext * g_workerContext;

CUserMng::CUserMng() {}
CUserMng::~CUserMng() {}

bool operator<(UserMapKey a, UserMapKey b)
{
	if(a.name.compare(b.name) < 0)	
        return true;

	if(a.name.compare(b.name) == 0)	{
		if(a.systemID.compare(b.systemID) < 0)	
            return true;
		return false;
	}

	return false;
}

int CUserMng::insert(User *user) 
{
	UserMapKey	key;
	key.name = user->name;
	key.systemID = user->systemID;
	((CoChannel *)(g_context->mChannel))->addTimer(&(user->ev), user->tv, userExpire, user);
	mUserMap.insert(pair<UserMapKey, User *>(key, user));

    LOG(2, "SysID: %s; QQ: %s", key.systemID.c_str(), key.name.c_str());
	return  0;
}

User* CUserMng::createUser() {
	CoCycle * cycle = g_cycle;
    User *pUser = cycle->mPool->newObj<User>();
    assert(pUser);
    return pUser;
}

int CUserMng::remove(UserMapKey	key)
{
	CoCycle * cycle = g_cycle;
	map<UserMapKey, User *>::iterator iter;
	iter = mUserMap.find(key);
	if(iter == mUserMap.end()) 
        return -1;

    ((CoChannel *)(g_context->mChannel))->clearTimer(&(iter->second->ev));
  	cycle->mPool->delObj((User *)iter->second);
    iter->second = NULL;

	mUserMap.erase(iter);
    LOG(2, "SysID:%s, QQ: %s", key.systemID.c_str(), key.name.c_str());
	return 0;
}

User* CUserMng::getUser(UserMapKey key)
{
	map<UserMapKey, User *>::iterator iter;
	iter = mUserMap.find(key);
	if(iter == mUserMap.end())
        return NULL;

	User * user = iter->second;
	((CoChannel *)(g_context->mChannel))->clearTimer(&(user->ev));
	((CoChannel *)(g_context->mChannel))->addTimer(&(user->ev), user->tv, userExpire, user);
	return user;
}

void CUserMng::userExpire(int fd, short events, void *arg)
{
	User * user = (User *) arg;
	assert(user != NULL);

	CUserMng * pMng = (CUserMng *)(g_workerContext->mUserMngPtr);
	assert(pMng != NULL);
	
	UserMapKey key;
	key.name = user->name;
	key.systemID = user->systemID;

	pMng->remove(key);
}

