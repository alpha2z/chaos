#include <string.h>
#include <iostream>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "CoData.h"
#include "CoCycle.h"

using namespace std;
using namespace chaos;

extern CoCycle * g_cycle;

CoData::CoData() {}
CoData::~CoData() {}

int CoData::get(char *buf, int &offset, int8_t &data)
{
	data = *((int8_t *)(buf + offset));
	offset += sizeof(data);
	return sizeof(data);
}

int CoData::get(char *buf, int &offset, uint8_t &data)
{
	data = *((uint8_t *)(buf + offset));
	offset += sizeof(data);
	return sizeof(data);
}

int CoData::get(char *buf, int &offset, int16_t &data)
{
	data = ntohs(*((int16_t *)(buf + offset)));
	offset += sizeof(data);
	return sizeof(data);
}
int CoData::get(char *buf, int &offset, uint16_t &data)
{
	data = ntohs(*((uint16_t *)(buf + offset)));
	offset += sizeof(data);
	return sizeof(data);
}

int CoData::get(char *buf, int &offset, int32_t &data)
{
	data = ntohl(*((int32_t *)(buf + offset)));
	offset += sizeof(data);
	return sizeof(data);
}

int CoData::get(char *buf, int &offset, uint32_t &data)
{
	data = ntohl(*((uint32_t *)(buf + offset)));
	offset += sizeof(data);
	return sizeof(data);
}

int CoData::get(char *buf, int &offset, char *data, size_t datalen)
{
	if(data == NULL)	return -1;
	memcpy(data, buf + offset, datalen);
	offset += datalen;

	return datalen;
}

int CoData::get(char *buf, int &offset, std::string &data, size_t dataMaxLen, int prefixType)
{
	CoCycle *cycle = g_cycle;

	char *d = NULL;
	size_t l = 0;
	if(prefixType == 0) {
		int16_t len = 0;
		get(buf, offset, len);
		
		l = len;
	} else {
		int32_t len = 0;
        get(buf, offset, len);

		l = len;
	}

	if(l > dataMaxLen)		return -1;

	d = (char *)cycle->mPool->alloc(l + 1);
	if(d == NULL)	return -2;

	memset(d, 0, l + 1);

	memcpy(d, buf + offset, l);
	offset += l;

	data = "";
	data.append(d, l);

	cycle->mPool->free(d);
	if(prefixType == 0)	return sizeof(int16_t) + l;
	else 	return sizeof(int32_t) + l;
}

int CoData::put(char *buf, int &offset, const int8_t &data)
{
	memcpy(buf + offset, &data, sizeof(data));

	offset += sizeof(data);
	return sizeof(data);
}

int CoData::put(char *buf, int &offset, const uint8_t &data)
{
	memcpy(buf + offset, &data, sizeof(data));
	offset += sizeof(data);

	return sizeof(data);
}

int CoData::put(char *buf, int &offset, const int16_t &data)
{
	int16_t d = htons(data);
	memcpy(buf + offset, &d, sizeof(d));
	offset += sizeof(data);

	return sizeof(data);
}

int CoData::put(char *buf, int &offset, const uint16_t &data)
{
	uint16_t d = htons(data);
	memcpy(buf + offset, &d, sizeof(d));
	offset += sizeof(data);

	return sizeof(data);
}
int CoData::put(char *buf, int &offset, const int32_t &data)
{
	int32_t d = htonl(data);
	memcpy(buf + offset, &d, sizeof(d));
	offset += sizeof(data);

	return sizeof(data);
}
int CoData::put(char *buf, int &offset, const uint32_t &data)
{
	uint32_t d = htonl(data);
	memcpy(buf + offset, &d, sizeof(d));
	offset += sizeof(data);

	return sizeof(data);
}
int CoData::put(char *buf, int &offset, const char *data, size_t datalen)
{
	memcpy(buf + offset, data, datalen);
	offset += datalen;

	return datalen;

}
int CoData::put(char *buf, int &offset, const std::string &data, int prefixType)
{
	if(prefixType == 0) {
		int16_t len = 0;
		len = data.length();
		put(buf, offset, len);

		memcpy(buf + offset, data.data(), len);
		offset += len;

		return sizeof(len) + len;
	} else {
		int32_t len = 0;
		len = data.length();
		put(buf, offset, len);

		memcpy(buf + offset, data.data(), len);
		offset += len;

		return sizeof(len) + len;
	}

}

#define h_addr h_addr_list[0]

char *CoData::getIPByDomain(char *domain)
{
	struct hostent *host;
	struct ifreq req;
	int sock;

	char * ip = NULL;

	if (domain == NULL) return NULL;
	if (strcmp(domain, "localhost") == 0) {
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		strncpy(req.ifr_name, "eth0", IFNAMSIZ);
		if ( ioctl(sock, SIOCGIFADDR, &req) < 0 ) {
			return NULL;
		}
		ip = (char *)inet_ntoa(*(struct in_addr *) &((struct sockaddr_in *) &req.ifr_addr)->sin_addr);
		shutdown(sock, 2);
		close(sock);
	} else {
		host = gethostbyname(domain);
		if (host == NULL) 
            return NULL;
		ip = (char *)inet_ntoa(*(struct in_addr *)(host->h_addr));
	}
	return ip;
}
