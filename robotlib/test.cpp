#include <iostream>
#include <sstream>
#include "md5.h"

using namespace std;


string genKey(uint32_t uin)
{
	string key = "";

	stringstream    ss;
	ss << uin << 1307975472 << "vlo-olv@qq.com";

	CMD5    md5;
	md5.GenerateMD5((unsigned char *)ss.str().c_str(), ss.str().length());

	key = md5.ToString();

	key = key.substr(key.length() - 10, 10);
	return key;
}

int main()
{
	cout << "key:" << genKey(276053392) <<endl;
	return 0;
}
