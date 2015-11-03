include ../Makefile.inc

EVENT_PATH = /imapp/lib/libevent
EVENT_INC = $(EVENT_PATH)/include
EVENT_LIB = $(EVENT_PATH)/lib

INSTALL_PATH = /imapp/lib/chaos

OBJS = CoMmpool.o CoLoger.o CoCycle.o CoConnection.o CoArray.o \
	   CoChannel.o CoData.o CoBuffer.o CoHandler.o

LIB_STATIC_NAME = lib/libchaos.a

SOFLAGS = -shared

#.c.o:
#	$(CC) -c $(CC_ARGS)  $^

#.cpp.o:
#	$(CXX) $(CXX_ARGS)  -c $^ -I$(EVENT_INC)

#.o:
#	$(CXX) $(CXX_ARGS) -o $@ $^ -L$(EVENT_LIB) -levent -lpthread 

all:$(OBJS)
	for dir in config; do \
		make -C $$dir; \
	done

$(LIB_STATIC_NAME):$(OBJS)
	$(AR) $@ $^


install:
	test -z $(INSTALL_PATH)/lib || /bin/mkdir -p $(INSTALL_PATH)/lib
	test -z $(INSTALL_PATH)/include || /bin/mkdir -p $(INSTALL_PATH)/include
	/usr/bin/install -c -m 644 *.h config/*.h $(INSTALL_PATH)/include
	/usr/bin/install -c $(LIBNAME) config/*.so $(INSTALL_PATH)/lib
#	strip $(INSTALL_PATH)/lib/*.so

clean:
	for dir in config; do\
		make clean -C $$dir;\
	done
	rm -rf *.o $(LIBNAME) $(LIB_STATIC_NAME)

    
