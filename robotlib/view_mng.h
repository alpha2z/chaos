#ifndef __VIEW_MNG_H
#define __VIEW_MNG_H

#include <string>
#include <map>
#include <mysql++.h>

using namespace std;

typedef struct {
	uint32_t    id;
	string  name;
	string  content;
}View;

typedef struct {
	uint32_t    id;
	string  name;
	string  title;
	string  action;
	short   type;
}ClickAction;

typedef struct {
	uint32_t	id;
	string		name;
	string		remark;
	bool		interruptable;
	short		type;
	map<string, View *>		viewMap;
	map<string, ClickAction *>  clickActionMap;
}ViewSet;

class CViewMng
{
	public:
		CViewMng();
		~CViewMng();

		enum {
			VIEW_SET_TYPE_REPEAT = 0,
			VIEW_SET_TYPE_SEARCH = 1,
			VIEW_SET_TYPE_REQ_WEB = 2,
			VIEW_SET_TYPE_TO_MAN = 3,
		};

		int init(mysqlpp::Connection *conn);

		int insertViewSet(ViewSet *viewSet);
		int insertView(string viewSetName, View *view);
		int insertClickAction(string viewSetName, ClickAction *clickAction);
	
		int removeViewSet(string name);
		int removeView(string viewSetName, string name);
		int removeClickAction(string viewSetName, string name);

		ViewSet * getViewSet(string name);
		ViewSet * getViewSetByID(uint32_t id);

		View * getView(string viewSetName, string name);
		ClickAction *getClickAction(string viewSetName, string name);
		
	public:
		map<string, ViewSet *>		mViewSetMap;
		map<uint32_t, ViewSet *>	mViewSetIDMap;
		mysqlpp::Connection 	*mDBConn;
};

#endif


