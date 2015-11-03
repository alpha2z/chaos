#include <syslog.h>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "configfile.h"
#include "configitem.h"

#include "robotlib/robot.h"
#include "user_mng.h"
#include "ogz_mng.h"
#include "worker_struct.h"
#include "data_channel_mng.h"
#include <vector>


using namespace configlib;
using namespace std;

void parse_string(string str, vector<uint32_t> &vec) {
    if (str.length() == 0) {
        return;
    }
    const char* start = str.c_str();
    const char* now = start;
    size_t len = str.length();
    uint32_t data = 1;
    for (; len > 0; len--, now++) {
        if (*now == ',') {
            if (now - start > 0) {
                int l = now - start;
                char temp[l+1];
                memcpy(temp, start, l);
                temp[l] = '\0';
                data = strtol(temp, NULL, 10);
                cout<<"len:"<<(l)<<",data:"<<data<<endl;
                if (data > 0) {
                    vec.push_back(data);
                }
                start = now + 1;
            }       
        }
    }
}

WorkerContext	*g_workerContext;

int main(int argc, char *argv[])
{
	if(argc != 2) {
		cout<<"Usage: "<<argv[0]<<" confile"<<endl;
		return 0;
	}

	configfile config(argv[1]);
	configitem<int> userexpiresec(config, "user", "expiresec", "", 0);
	configitem<int> userexpireusec(config, "user", "expireusec", "", 0);

	configitem<std::string> dbname(config, "db", "name", "", "");
	configitem<std::string> dbuser(config, "db", "user", "", "");
	configitem<std::string> dbpass(config, "db", "pass", "", "");

	configitem<std::string> clientaddr(config, "client", "client_address", "", "");
	configitem<std::string> clientport(config, "client", "client_port", "", "");

    configitem<std::string> strprovince(config, "block", "province", "", "");
    configitem<std::string> strcity(config, "block", "city", "", "");

	config.read();

	CRobot	robot;
	int ret = robot.init(argc, argv);
	if(ret < 0) {
		cout<<"robot init fail:"<< ret<<endl;
		return -1;
	}

	WorkerContext workerContext ;
	robot.setWorkerContext((uintptr_t) &workerContext);
	workerContext.mUserExpireSec = (int) userexpiresec;
	workerContext.mUserExpireUsec = (int) userexpireusec;
	g_workerContext = &workerContext;

	robot.run();

	return 0;
}



