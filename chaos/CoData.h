#ifndef __CHAOS_DATA_H
#define __CHAOS_DATA_H

#include <string>
#include <netinet/in.h>

namespace chaos {
	class CoData
	{
		public:
			CoData();
			virtual ~CoData();
			static int get(char *buf, int &offset, int8_t &data);
			static int get(char *buf, int &offset, uint8_t &data);
			static int get(char *buf, int &offset, int16_t &data);
			static int get(char *buf, int &offset, uint16_t &data);
			static int get(char *buf, int &offset, int32_t &data);
			static int get(char *buf, int &offset, uint32_t &data);

			/* prefixType: 0 长度前缀数据类型为int16_t, 1 类型为int32_t
			 * dataMaxLen: 此string允许的最大长度
			 */
			static int get(char *buf, int &offset, char *data, size_t datalen);
			static int get(char *buf, int &offset, std::string &data, size_t dataMaxLen, int prefixType);

			static int put(char *buf, int &offset, const int8_t &data);
			static int put(char *buf, int &offset, const uint8_t &data);
			static int put(char *buf, int &offset, const int16_t &data);
			static int put(char *buf, int &offset, const uint16_t &data);
			static int put(char *buf, int &offset, const int32_t &data);
			static int put(char *buf, int &offset, const uint32_t &data);
			static int put(char *buf, int &offset, const char *data, size_t datalen);
			static int put(char *buf, int &offset, const std::string &data, int prefixType);
	
			static char * getIPByDomain(char * domain);
	};
};
#endif
