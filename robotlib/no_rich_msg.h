#ifndef __RICH_MSG_H
#define __RICH_MSG_H

#include <string>
#include <list>

#include "struct.h"

typedef struct {
	std::string pair;
	std::string	viewSetName;
	std::string	viewName;
//	std::string	actionType;		//J:jump view set,  P: show view
}ViewNamePair;

class CRichMsg
{
	public:
		CRichMsg();
		~CRichMsg();

	public:

		int replace(std::string &text, std::string oldstr, std::string newstr);
		std::string getRichMsg(int cmd, std::string name, std::string action);

		int replace(std::string &text);
	private:
		int replaceView(std::string &text);
		int replaceClickAction(std::string &text);
		ssize_t subZHStr(std::string src, std::string &desc, size_t index, size_t len);
		ViewNamePair find(std::string content, size_t &start);
	public:
		std::list<std::string>		mMsgList;
		std::string				mQuestList;
		std::string			mBusinessPara;
		std::string			mUserName;
		int					mQuitFlag;
		int					mToManFlag;
		int					mSearchFlag;
};

#endif

