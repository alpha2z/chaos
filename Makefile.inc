INC_MYSQL =  -I/usr/local/mysql/include/mysql
LIB_MYSQL = -L/usr/local/mysql/lib/mysql -lmysqlclient

CXX_ARGS = -g -O2 -Wall -DCO_USE_UCONEXT 
CC_ARGS = -g -O2 -Wall -DCO_USE_UCONEXT 

CC = gcc
CXX = g++

AR = ar -sr

SOFLAGS = -shared

RM = rm -rf

COPY = cp -f

.PHONY : all

EVENT_PATH = /imapp/lib/libevent
EVENT_INC = $(EVENT_PATH)/include
EVENT_LIB = $(EVENT_PATH)/lib

ORACLE_PATH = /imapp/lib/oracle
ORACLE_INC = $(ORACLE_PATH)/include
ORACLE_LIB = $(ORACLE_PATH)/lib

TCMALLOC_LIB = /imapp/lib/tcmalloc/lib

PROJECT_PATH = /home/zhoujunjie/robot_workspace/robotPassContent/

LIBCHAOS_INC = $(PROJECT_PATH)chaos
CONFIG_INC = $(LIBCHAOS_INC)/config
ROBOT_INC = $(PROJECT_PATH)robotlib/

PCRE_PATH = /home/zhoujunjie/lib/pcre-8.12
PCRE_INC = $(PCRE_PATH)
PCRE_LIB = $(PCRE_PATH)/.libs

#-lworker -lcurl -lpthread -lchaos -lconfig \

LINK_LIB =-L$(EVENT_LIB) -levent -L./ \
		  -lworker -lcurl -lpthread \
		  -L$(ORACLE_LIB) -lclntsh -lnnz11 -laio \
		  -L$(TCMALLOC_LIB) -ltcmalloc -L$(PCRE_LIB) -lpcre \
		  -Wl,-rpath,$(ORACLE_LIB) -Wl,-rpath,$(EVENT_LIB) \
		  -Wl,-rpath,./ -Wl,-rpath,$(TCMALLOC_LIB)


.c.o:
	$(CC) $(CC_ARGS)  -c $^ -I$(ROBOT_INC) -I$(CONFIG_INC) -I$(EVENT_INC) -I$(LIBCHAOS_INC)

.cpp.o:
	$(CXX) $(CXX_ARGS)  -c $^ -I$(ROBOT_INC) -I$(CONFIG_INC) -I$(EVENT_INC) -I$(LIBCHAOS_INC) -I$(ORACLE_INC) -I$(PCRE_INC)

.o:
	$(CXX) $(CXX_ARGS) -o $@ $^ $(LINK_LIB) -I$(ROBOT_INC) -I$(CONFIG_INC) 


