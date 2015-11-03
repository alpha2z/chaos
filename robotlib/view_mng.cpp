#include "CoCycle.h"
#include "view_mng.h"
#include "code_converter.h"
#include "struct.h"

using namespace chaos;
using namespace mysqlpp;

extern CoCycle *g_cycle;

CViewMng::CViewMng() {}

CViewMng::~CViewMng() {}

int CViewMng::init(Connection *conn)
{
	CoCycle *cycle = g_cycle;

	if(conn == NULL)	return -1;

	mDBConn = conn;

	StoreQueryResult::const_iterator it;

	Row row ;

	Query query = mDBConn->query();
	
	query << "select id, ltrim(rtrim(name)), ltrim(rtrim(remark)), "
		<< "interruptable, type from t_view_set";
	StoreQueryResult res = query.store();

	if(res.empty()) return -2;

	static char buf[2048];
	CodeConverter cc = CodeConverter("utf-8","gb2312");

	for (it = res.begin(); it != res.end(); ++it) {
		row = *it;

		ViewSet *viewSet = cycle->mPool->newObj<ViewSet>();
		assert(viewSet != NULL);

		viewSet->id = row[0];

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[1].c_str(), row[1].length(), buf, sizeof(buf));

		viewSet->name = buf;

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[2].c_str(), row[2].length(), buf, sizeof(buf));

		viewSet->remark = buf;

		int interruptable = row[3];
		if(interruptable == 0)		viewSet->interruptable = true;
		else viewSet->interruptable = false;

		viewSet->type = row[4];
		insertViewSet(viewSet);
	}

	query << "select a.id, ltrim(rtrim(a.name)), ltrim(rtrim(a.content)), "
		<< " ltrim(rtrim(b.name)) from t_view as a " 
		<< "left join t_view_set as b on a.view_set_id = b.id";
	res = query.store();


	if(res.empty())	return -3;

	for (it = res.begin(); it != res.end(); ++it) {
		row = *it;

		View *view = cycle->mPool->newObj<View>();
		assert(view != NULL);

		view->id = row[0];

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[1].c_str(), row[1].length(), buf, sizeof(buf));

		view->name = buf;

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[2].c_str(), row[2].length(), buf, sizeof(buf));

		view->content = buf;

		insertView(row[3].c_str(), view);
	}
	
	query << "select a.id, ltrim(rtrim(a.name)), ltrim(rtrim(a.title)), "
		<< " ltrim(rtrim(a.action)), a.type, ltrim(rtrim(b.name)) "
		<<"from t_click_action as a left join t_view_set as b "
		<< "on a.view_set_id = b.id";
	res = query.store();
	
	if(res.empty())	return -4;

	for (it = res.begin(); it != res.end(); ++it) {
		row = *it;

		ClickAction *clickAction = cycle->mPool->newObj<ClickAction>();
		assert(clickAction != NULL);

		clickAction->id = row[0];

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[1].c_str(), row[1].length(), buf, sizeof(buf));

		clickAction->name = buf;

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[2].c_str(), row[2].length(), buf, sizeof(buf));

		clickAction->title = buf;

		memset(buf, 0, sizeof(buf));
		cc.convert((char *)row[3].c_str(), row[3].length(), buf, sizeof(buf));

		clickAction->action = buf;

		clickAction->type = row[4];

		insertClickAction(row[5].c_str(), clickAction);
	}

	return 0;
}

int CViewMng::insertViewSet(ViewSet *viewSet)
{
	mViewSetMap.insert(pair<string, ViewSet *>(viewSet->name, viewSet));
	mViewSetIDMap.insert(pair<uint32_t, ViewSet *>(viewSet->id, viewSet));
	return 0;
}

int CViewMng::insertView(string viewSetName, View *view)
{
	ViewSet *viewSet = getViewSet(viewSetName);
	
	if(viewSet == NULL)		return -1;

	viewSet->viewMap.insert(
			pair<string, View *>(view->name, view));
	return 0;
}

int CViewMng::insertClickAction(string viewSetName, ClickAction *clickAction)
{
	ViewSet *viewSet = getViewSet(viewSetName);

	if(viewSet == NULL) return -1;

	viewSet->clickActionMap.insert(pair<string, ClickAction *>(clickAction->name, clickAction));
	return 0;
}

int CViewMng::removeViewSet(string name)
{
	map<string, ViewSet *>::iterator iter;
	iter = mViewSetMap.find(name);

	if(iter == mViewSetMap.end()) return -1;

	map<uint32_t, ViewSet *>::iterator iterID;
	iterID = mViewSetIDMap.find(iter->second->id);

	if(iterID == mViewSetIDMap.end())	return -2;

	mViewSetMap.erase(iter);
	mViewSetIDMap.erase(iterID);
	return 0;
}
int CViewMng::removeView(string viewSetName, string name)
{
	ViewSet *viewSet = getViewSet(viewSetName);

	if(viewSet == NULL) return -2;

	map<string, View *>::iterator iter;
	iter = viewSet->viewMap.find(name);

	if(iter == viewSet->viewMap.end()) return -1;

	viewSet->viewMap.erase(iter);

	return 0;
}

int CViewMng::removeClickAction(string viewSetName, string name)
{
	ViewSet *viewSet = getViewSet(viewSetName);

	if(viewSet == NULL) return -2;

	map<string, ClickAction *>::iterator iter;
	iter = viewSet->clickActionMap.find(name);

	if(iter == viewSet->clickActionMap.end()) return -1;

	viewSet->clickActionMap.erase(iter);

	return 0;
}

ViewSet * CViewMng::getViewSet(string name)
{
	map<string, ViewSet *>::iterator iter;

	iter = mViewSetMap.find(name);

	if(iter == mViewSetMap.end()) return NULL;

	return iter->second;
}

ViewSet * CViewMng::getViewSetByID(uint32_t id)
{
	map<uint32_t, ViewSet *>::iterator iter;

	iter = mViewSetIDMap.find(id);

	if(iter == mViewSetIDMap.end()) return NULL;

	return iter->second;
}

View * CViewMng::getView(string viewSetName, string name)
{
	ViewSet *viewSet = getViewSet(viewSetName);

	if(viewSet == NULL) return NULL;

	map<string, View *>::iterator iter;

	iter = viewSet->viewMap.find(name);

	if(iter == viewSet->viewMap.end()) return NULL;

	return iter->second;
}

ClickAction *CViewMng::getClickAction(string viewSetName, string name)
{
	ViewSet *viewSet = getViewSet(viewSetName);

	if(viewSet == NULL) return NULL;

	map<string, ClickAction *>::iterator iter;
	iter = viewSet->clickActionMap.find(name);

	if(iter == viewSet->clickActionMap.end()) return NULL;

	return iter->second;
}
