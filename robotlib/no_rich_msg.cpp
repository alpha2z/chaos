#include "CoCycle.h"

#include "rich_msg.h"
#include "view_mng.h"
#include "user_mng.h"

using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext	*g_context;

CRichMsg::CRichMsg() {}
CRichMsg::~CRichMsg() {}


ViewNamePair CRichMsg::find(string content, size_t &start)
{
	ViewNamePair	viewNamePair;
	viewNamePair.viewSetName = "";
	viewNamePair.viewName = "";
//	viewNamePair.actionType = "";
	viewNamePair.pair = "";

	size_t pos = 0, endPos = 0;

	pos = content.find(PREFIX, start);

	if(pos != string::npos)	{
		start = pos;
		endPos = content.find(SUFFIX, pos);
		if(endPos != string::npos)	{
//			pos += PREFIX.length();
			viewNamePair.pair = content.substr(pos, endPos + SUFFIX.length() - pos);
			LOG(5, "pair[%s]", viewNamePair.pair.c_str());

			pos = PREFIX.length();
			endPos = viewNamePair.pair.find(MID, pos);
			
			if(pos != string::npos)	{
				viewNamePair.viewSetName = viewNamePair.pair.substr(pos, endPos - pos);

				LOG(5, "viewsetname[%s,%d, %d]", viewNamePair.viewSetName.c_str(), pos, endPos);

				pos = endPos + MID.length();

				endPos = viewNamePair.pair.length() - SUFFIX.length();
				viewNamePair.viewName = viewNamePair.pair.substr(pos, endPos - pos);
				LOG(5, "viewname[%s]", viewNamePair.viewName.c_str());
			}
		}
	}

	return viewNamePair;
}

ssize_t CRichMsg::subZHStr(string src, string &desc, size_t index, size_t len)
{
	unsigned int i = index;

	if(i < 0 || i >= src.length())	return -1;

	if(len < 0) return -2;

	while(i < len + index)
	{
		if((unsigned char) (src.at(i)) > 127) 	i += 2;
		else	++i;
	}

	if(i == len + i) {
		desc = src.substr(index, len);
		return len;
	} else {
		desc = src.substr(index, len - 1);
		return len - 1;
	}
}

int CRichMsg::replace(string &text)
{
	CoCycle * cycle = g_cycle;
	
	mQuitFlag = 0;
	mToManFlag = 0;
	mSearchFlag = 0;

	CViewMng	*mng = (CViewMng *) (g_context->mViewMng);
	if(mng == NULL)	return -2;

	CUserMng	*userMng = (CUserMng *) (g_context->mUserMng);
	if(userMng == NULL) return -3;

	User	*user = userMng->get(mUserName);
	LOG(5, "username[%s,%d]", mUserName.c_str(), userMng->mUserMap.size());
	LOG_BUF(6, (char *)mUserName.data(), mUserName.length());
	if(user == NULL)	{
		user = cycle->mPool->newObj<User>();
		user->status = CUserMng::USER_VIEW_STATUS_READY;
		user->name = mUserName;

		userMng->insert(user);
	}

	LOG(5, "user status[%d, %d]", user->status, CUserMng::USER_VIEW_STATUS_READY);
	ViewSet *viewSet = NULL;
	View	*view = NULL;
	if(user->status == CUserMng::USER_VIEW_STATUS_READY) {
		viewSet = mng->getViewSet(VIEW_SET_GLOBAL);
		if(viewSet == NULL) {
			LOG(2, "no global view set");
			return 0;
		}
	
		view = mng->getView(VIEW_SET_GLOBAL, VIEW_WELCOME);
		if(view == NULL) {
			LOG(2, "no welcome view");	return 0;
		}

		text = view->content;
		user->viewSetName = viewSet->name;
		user->status = CUserMng::USER_VIEW_STATUS_ENTER;
		LOG(5, "read view[%s]", text.c_str());
	}

	viewSet = mng->getViewSet(user->viewSetName);
	if(viewSet == NULL) {
		LOG(2, "no view set[%s]", user->viewSetName.c_str());
		return -10;
	}

	LOG(5, "viewset type[%d]", viewSet->type);

	int viewCount = 0, caCount = 0;
	viewCount = replaceView(text);

	caCount = replaceClickAction(text);

	if(viewCount == 0 && caCount == 0) {
		if(mQuitFlag != 1 && mSearchFlag != 1) {
			LOG(5, "user quit");
			text = user->lastContent;
		}
	} else {
		LOG(5, "repeat");
		if(mQuitFlag != 1 && mSearchFlag != 1) {
			user->lastContent = text;
		}
	}

	return 0;
}

int CRichMsg::replaceView(string &text)
{
	CoCycle * cycle = g_cycle;
	int viewCount = 0;

	CViewMng    *mng = (CViewMng *) (g_context->mViewMng);

	if(mng == NULL) return -2;

	CUserMng    *userMng = (CUserMng *) (g_context->mUserMng);
	if(userMng == NULL) return -3;

	User    *user = userMng->get(mUserName);
	LOG(5, "username[%d,%d]", mUserName.c_str(), userMng->mUserMap.size());

	if(user == NULL) { LOG(2, "user null[%s]", mUserName.c_str()); return viewCount; }
	
	size_t start = 0;
	ViewNamePair	viewNamePair;
	ViewSet *viewSet = NULL;
	View *view = NULL;

	short groupFlag = 0;

	viewSet = mng->getViewSet(user->viewSetName);
	if(viewSet == NULL) {
		LOG(2, "no view set[%s]", user->viewSetName.c_str());
		return -10;
	}
	do {
		viewNamePair = find(text, start);
		if(viewNamePair.viewSetName.compare("") == 0) {
			LOG(5, "no view set");

			if(viewSet->type == CViewMng::VIEW_SET_TYPE_SEARCH)  mSearchFlag = 1;

			break;
		} else {
			LOG(5, "view set [%s, %s]", viewNamePair.viewSetName.c_str(),
					viewNamePair.viewName.c_str());

			if(viewNamePair.viewName.compare(VIEW_WELCOME) == 0) {
				user->viewSetName = viewNamePair.viewSetName;
			}

			viewSet = mng->getViewSet(user->viewSetName);
			if(viewSet == NULL) {
				LOG(2, "no view set[%s]", user->viewSetName.c_str());
				return -10;
			}

			if(viewNamePair.viewName.compare(VIEW_BUSI_URL) == 0 ) return 0;
			if(viewSet->type == CViewMng::VIEW_SET_TYPE_TO_MAN && groupFlag == 0) {
				view = mng->getView(viewSet->name, viewNamePair.viewName);
				if(view == NULL)	{
					LOG(5, "groupid[%s]", viewNamePair.viewName.c_str());
					groupFlag = 1;
					user->groupID = atoi(viewNamePair.viewName.c_str());
				
					view = mng->getView(viewSet->name, VIEW_TO_MAN);
					if(view == NULL) {
						LOG(2, "no to man view");
						return -11;
					}
					mToManFlag = 1;
					text = view->content;
				}
			}

			if(viewNamePair.viewName.compare(VIEW_QUIT) == 0) {
				userMng->remove(mUserName);
				cycle->mPool->delObj(user);
				user = NULL;
				mQuitFlag = 1;
				return viewCount;
			}
		}

		view = mng->getView(viewNamePair.viewSetName, viewNamePair.viewName);
		if(view != NULL)	{
			LOG(5, "replace[%s, %s]", viewNamePair.pair.c_str(), view->content.c_str());
			text.replace(start, viewNamePair.pair.length(), view->content);
			start += view->content.length();
			++viewCount;
			continue;

		}
	
		start += viewNamePair.pair.length();
	} while(1);


	return viewCount;
	
}

int CRichMsg::replaceClickAction(std::string &text)
{
	CViewMng    *mng = (CViewMng *) (g_context->mViewMng);

	if(mng == NULL) return -2;

	int count = 0;
	size_t start = 0;
	int cmd = 0;
	ViewNamePair    viewNamePair;
	ClickAction *ca = NULL;

	string	richMsg = "";
	do {
		viewNamePair = find(text, start);
		if(viewNamePair.viewSetName.compare("") == 0) {
			LOG(5, "no view set");
			break;
		} else {
			LOG(5, "view set [%s, %s]", viewNamePair.viewSetName.c_str(),
					viewNamePair.viewName.c_str());
		}

		ca = mng->getClickAction(viewNamePair.viewSetName, viewNamePair.viewName);
		if(ca != NULL)    {
			string action = "";
			LOG(5, "vn[%s], menu[%s]", viewNamePair.viewName.c_str(), VIEW_BUSI_URL.c_str());
			if(viewNamePair.viewName.compare(VIEW_BUSI_URL) == 0) {
				action = mBusinessPara;
			} else {
				action = ca->action;
			}

			LOG(5, "replace[%s, %s, %s]", viewNamePair.pair.c_str(), ca->title.c_str(), action.c_str());
			if(ca->type == 0) {
				cmd = 2;
			} else {
				cmd = 3;
			}

			richMsg = getRichMsg(cmd, ca->title, action);

			text.replace(start, viewNamePair.pair.length(), richMsg);
			start += richMsg.length();
			++count;
			continue;
		}

		start += viewNamePair.pair.length();
	} while(1);
	return count;
}

string CRichMsg::getRichMsg(int cmd, string name, string action)
{
	static char buf[1024];

	memset(buf, 0, sizeof(buf));

	int len = 14 + name.length() + action.length();

	snprintf(buf, sizeof(buf)-1, "\25%d%3d%2d%3d%s%3d%sA", 9, len, cmd,
			name.length(), name.c_str(),
			action.length(), action.c_str());

	return buf;
}

