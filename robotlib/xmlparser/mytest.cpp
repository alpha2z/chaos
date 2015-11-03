#include <iostream>
#include "tinyxml.h"

using namespace std;

int main()
{
	const char* demoStart = "<?xml version=\"1.0\" encoding=\"GBK\" ?><Item>    <simi_result>        <CorrectPin name=\"Æ´Òô¾À´í\" />        <SimilarWords name=\"Ïà¹Ø´ÊÓï\" />    </simi_result>    <GBresult>        <GBresult_info>            <ret name=\"·µ»ØÖµ\">-6</ret>        </GBresult_info>    </GBresult></Item>";

	string d = demoStart;

	TiXmlDocument doc;

	doc.Parse(d.c_str());
	doc.Parse(d.c_str());

	doc.SaveFile("my.xml");
	return 0;
}
