include ../Makefile.inc

OBJS = handler.o mc_msg.o mc_handler.o mc_login.o \
		  mc_transform_msg.o mc_shift_route.o \
		  business_notify_result.o base64.o md5.o \
		  api.o pcl.o worker.o \
		  robot.o data_channel_mng.o shmqueue.o shmem.o \
		  openid_converter.o mc_pass.o mc_getTxsession.o \
		  qt_check.o qt_client.o qt_msg.o qt_sync.o qt_opt_conv.o

TARGET = $(LIBNAME) $(LIB_STATIC_NAME)

all:$(OBJS)

clean:
	$(RM) *.o

    
