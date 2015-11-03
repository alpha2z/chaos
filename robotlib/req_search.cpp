#include <sstream>
#include "CoData.h"
#include "CoCycle.h"
#include "CoChannel.h"

#include "req_search.h"

using namespace std;
using namespace chaos;
using namespace robot;

extern CoCycle *g_cycle;
extern RobotContext *g_context;

int CReqSearch::encode()
{
	mQ = URLEncode(mQ);
	mFL = URLEncode(mFL);
	mURL = g_context->mSearchUrl;
	mIndent = "on";
	mVer = "2.2";
	mFQ = "";
	mQT = "imdismax";
	mWT = "imdismax";
	mExplainOther = "";
	mHL = "on";
	mHlFl = "";
	mTV = "on";
	mTvAll = "true";

	mReqData.str("");

	mReqData << "GET " << mURL << "?indent=" << mIndent << "&version=" << mVer 
		<<"&q=" << mQ 
		<<"&fq="<< mFQ << "&start=" << mStart
		<<"&rows="<< mRows << "&fl=" << mFL
		<<"&qt="<< mQT <<"&wt="<< mWT
		<<"&explainOther="<< mExplainOther
		<<"&hl="<< mHL << "&hl.fl=" << mHlFl
		<<"&tv="<< mTV << "&tv.all=" << mTvAll
		<<" HTTP/1.1\r\nUser-Agent: Mozilla/4.0 \r\n"
		<<"Host:" << g_context->mSearchDomain << ":" << g_context->mSearchPort << "\r\n"
       	<<"Content-Type: text/xml;charset=UTF-8\r\n"
		<<"Accept: */*\r\nConnection: Keep-Alive\r\n\r\n";

	int offset = 0;
	CoConnection *c = mConn;

	string reqData = mReqData.str();
	LOG(5, "reqstr[%s]", reqData.c_str());

	c->mSendBuf->append(mReqData.str().c_str(), mReqData.str().length());

	return offset;
}

int CReqSearch::decode()
{
	CoConnection *c = mConn;

	string endFlag = "</" + SEARCH_XML_ITEM_STR + ">";

	u_char *end = c->mRecvBuf->find((u_char *)endFlag.c_str(), endFlag.length());

	if(end == NULL) {
		return 0;
	}

	char * beginFlag = "<?xml";
	u_char *begin = c->mRecvBuf->find((u_char *)beginFlag, strlen(beginFlag));

	if(begin == NULL) {
		return -1;
	}

	LOG(5, "begin[%d], end[%d], size[%d]", begin, end, c->mRecvBuf->getSize());
	c->logBuf(6, 0);

	mDoc.Clear();
	mDoc.Parse((char *)(begin));

	return c->mRecvBuf->getSize();
}

int CReqSearch::processReq(CoConnection *c)
{
	mConn = c;

	int ret = 0;
	if((ret = encode()) < 0)	{
		LOG(2, "encode fail[%d]", ret);
		return -1;
	}

	mConn->logBuf(6, 1);
    if(mConn->sendData() < 0)   return -2;
	
	mConn->mHandler->addPkgWithTimeoutEvent(0, NULL);
	return ret;
}

int CReqSearch::processResp(CoConnection *c)
{
	mConn = c;

	int ret = 0;
	if((ret = decode()) < 0) {
		LOG(2, "decode fail[%d]", ret);
		return -1;
	}

	mConn->mHandler->clearPkgWithTimeoutEvent(0);

	mConn->mRecvBuf->drain(ret);

	TiXmlHandle hDoc(&mDoc);
	TiXmlElement* pElem;
	TiXmlElement* pResult;
	TiXmlElement* pNode;

	TiXmlHandle hRoot(0);
	TiXmlHandle hResult(0);
	TiXmlHandle hElem(0);

	pElem = hDoc.FirstChildElement().Element();
	if (!pElem) { LOG(2, "xml node error"); return -10; }


	hRoot = TiXmlHandle(pElem);

	pResult = hRoot.FirstChild( "result" ).Element();

	if(!pResult) { LOG(2, "result node error"); return -10; }

	mNum = pResult->Attribute("numFound");
	mStart = pResult->Attribute("start");

	pResult = hRoot.FirstChild( "result" ).FirstChild("doc").Element();
LOG(5, "num:%s, start:%s", mNum.c_str(), mStart.c_str());	
	SearchResult	sr;

	string elemName = "";
	string text = "";
	mResult.clear();
	for(pResult; pResult != NULL; (pResult = pResult->NextSiblingElement()))
	{
		hResult = TiXmlHandle(pResult);

//		pElem = hResult.FirstChild("float").Element();
//		if (!pElem) { LOG(2, "node error"); }

//		pElem->QueryFloatAttribute("name", &sr.score);

		pElem = hResult.FirstChildElement().Element();

		for(pElem; pElem != NULL; pElem = pElem->NextSiblingElement()) {
			elemName = pElem->Attribute("name");

			hElem = TiXmlHandle(pElem);


			if(elemName.compare("answer") == 0 || elemName.compare("question") == 0) {
				pNode = hElem.FirstChild("str").Element();
			} else {
				pNode = hElem.Element();
			}
			
			if(pNode == NULL) 	break;

			if(elemName.compare("answer") == 0) {

				sr.anwser = pNode->GetText();
			} else if(elemName.compare("question") == 0) {
				sr.question = pNode->GetText();
			} else if(elemName.compare("id") == 0) {
				sr.id = pNode->GetText();
			} else {
				sr.keywords = pNode->GetText();
			}
		}

		mResult.insert(pair<string, SearchResult>(sr.question, sr));
		
	}

	map<string, SearchResult>::iterator iter;
	
	LOG(5, "result size:%d", mResult.size());
	for(iter = mResult.begin(); iter != mResult.end(); iter++) {
		SearchResult	sr2;
		sr2 = iter->second;
		LOG(5, "score:%f, id:%s, keywords:%s", sr2.score, sr2.id.c_str(), sr2.keywords.c_str());

		LOG(5, "question:%s, anwser:%s", sr2.question.c_str(), sr2.anwser.c_str());
/*
		CMcTransformMsg mcTransformMsg;

		mcTransformMsg.mContent = sr2.anwser;
		mcTransformMsg.mSubCmd = CMcTransformMsg::SUB_CMD_REQ_MSG;
		mcTransformMsg.mSystemID = mSystemID;
		mcTransformMsg.mSessionID = mSessionID;
		mcTransformMsg.mSeq = getSeq();
		mcTransformMsg.mSessionTicket = "";
		mcTransformMsg.mIMType = IM_TYPE_QQ;
		mcTransformMsg.mSenderType = CMcTransformMsg::SENDER_TYPE_ROBOT;
		mcTransformMsg.mFromUserName = mKFName;
		mcTransformMsg.mToUserName = mOpenID;

		cout<<"conn:"<<c<<endl;
		if((ret = mcTransformMsg.processReq(c)) < 0) {
			LOG(2, "mc transform msg fail[%d]", ret);
			return offset;
		}
*/	}
	return ret;
}

string CReqSearch::URLEncode(const string &c)
{
	string escaped="";
	int max = c.length();
	for(int i=0; i<max; i++)
	{
		if ( (48 <= c[i] && c[i] <= 57) ||//0-9
				(65 <= c[i] && c[i] <= 90) ||//abc...xyz
				(97 <= c[i] && c[i] <= 122) || //ABC...XYZ
				(c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
		   )
		{
			escaped.append( &c[i], 1);
		}
		else
		{
			escaped.append("%");
			escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
		}
	}
	return escaped;
}

string CReqSearch::char2hex( char dec )
{
	char dig1 = (dec&0xF0)>>4;
	char dig2 = (dec&0x0F);
	if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
	if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
	if ( 0<= dig2 && dig2<= 9) dig2+=48;
	if (10<= dig2 && dig2<=15) dig2+=97-10;

	string r;
	r.append( &dig1, 1);
	r.append( &dig2, 1);
	return r;
}

/*
int main(int argc, char *argv[])
{
	string address = "123 #5 M&M Street ?¨°¨º?¨°???¡À? hi";
	cout << "address=" << address << endl;
	cout << "address=" << urlencode(address) <<endl;
}
 */
