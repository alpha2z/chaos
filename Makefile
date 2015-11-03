include Makefile.inc

OBJS = 	user_mng.o \
		ogz_mng.o \
		oracleprocess.o

LIBOBJS = myworker.o 

LIBNAME = libworker.so

BIN = robotif

TARGET = $(LIBNAME) \
		 $(BIN)
all:

CHAOS_DIR=chaos
CONFIG_DIR = chaos/config

chaos_lib:
	for dir in $(CHAOS_DIR); do \
		make all -C $$dir; \
	done;

all: chaos_lib $(TARGET)
	@echo make all success
	
$(LIBNAME):$(LIBOBJS)
	$(CXX) $(SOFLAGS) $^ -o $@

ROBOT_DIR = robotlib

robotif:robotif.o $(OBJS)
	for dir in $(ROBOT_DIR); do \
		make all -C $$dir; \
	done;
	$(CXX) $(CXX_ARGS) $(ROBOT_DIR)/*.o $(CHAOS_DIR)/*.o $(CONFIG_DIR)/*.o -o $@ $^ $(LINK_LIB)

clean:
	for dir in $(CHAOS_DIR); do \
		make all -C $$dir; \
	done;
	for dir in $(ROBOT_DIR); do \
		make clean -C $$dir; \
	done;
	$(RM) $(LIBOBJS) $(BIN) $(LIBNAME) *.o robotif

    
