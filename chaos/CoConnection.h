#ifndef __CHAOS_CONNECTION_H
#define __CHAOS_CONNECTION_H

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include <event.h>

#include "CoBuffer.h"
#include "CoHandler.h"

namespace chaos {

	class CoConnection {
		public:
			CoConnection();
			~CoConnection();

			enum {
				CONN_STATUS_READY = 0x00,
				CONN_STATUS_ESTABLISH = 0x01,
				CONN_STATUS_TIMEOUT = 0x02,
				CONN_STATUS_CLOSED = 0x03,
				CONN_STATUS_RETRYING = 0x04,
			};

			enum {
				CONN_TYPE_UNKNOW = 0x00,
				CONN_TYPE_CLIENT = 0x01,
				CONN_TYPE_SERVER = 0x02,
			};

			enum ConnMode {
				CONN_MODE_TCP = 0,
				CONN_MODE_UDP = 1,
			};

			enum {
				MAX_CONN_NAME_LEN = 16,
			};

			static CoConnection *getConn(int fd);	

			void close();
			void closeConn();

			ssize_t sendData();

			static void readCB(int fd, short events, void *arg);
			static void writeCB(int fd, short events, void *arg);
			static void acceptCB(int fd, short event, void *arg);

		public:
			int put(char data);
			int get(char &data);

			int put(int8_t data);
			int get(int8_t &data);

			int put(uint8_t data);
			int get(uint8_t &data);

			int put(int16_t data);
			int get(int16_t &data);

			int put(uint16_t data);
			int get(uint16_t &data);

			int put(int32_t data);
			int get(int32_t &data);

			int put(uint32_t data);
			int get(uint32_t &data);

			int put(int64_t data);
			int get(int64_t &data);

			int put(uint64_t data);
			int get(uint64_t &data);

			int put(const void *data, size_t dataLen);
			int get(char *data, size_t dataLen);

			int put(string &data, int prefixType = 0);
			int get(string &data, int prefixType = 0);

			void logBuf(int level, int type);
		private:
			static void addEvent(CoConnection *c, short events);
			void free();
			void addToRetryList();
		
		public:
			CoBuffer     		*mRecvBuf;
			CoBuffer     		*mSendBuf;

			CoConnection        *mNext;
			struct event        *mRead;
			struct event        *mWrite;
			struct event        *mTimeout;

			int                 mFD;

			struct sockaddr_in  mAddr;

			CoHandler           *mHandler;

			/*通道类型
			 * CONN_TYPE_UNKNOW, CONN_TYPE_SERVER, CONN_TYPE_CLIENT
			 */
			unsigned            mType;

			/*在clientChannelList中的索引
			*/
			int        mClientIndex;

			/*在retryList中的索引
			*/
			int        mRetryIndex;

			char		mName[MAX_CONN_NAME_LEN];

			/*连接状态
			*/
			unsigned            mStatus;
		
			ConnMode			mConnMode;

			bool				mIsReconnect;
	};
};
#endif
