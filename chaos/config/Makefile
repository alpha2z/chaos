include ../../Makefile.inc

LIBOBJS = color.o configfile.o configitem_base.o ipaddress.o

LIBNAME = ../lib/libconfig.so
LIB_STATIC_NAME = ../lib/libconfig.a

SOFLAGS = -shared

TARGET = $(LIBNAME)

#.c.o:
#	$(CC) $(CC_ARGS)  -c $^ 

#.cpp.o:
#	$(CXX) $(CXX_ARGS)  -c $^

#.o:
#	$(CXX) $(CXX_ARGS) -o $@ $^ 

#all: $(LIB_STATIC_NAME)
all: $(LIBOBJS)

$(LIBNAME):$(LIBOBJS)
	$(CXX) $(SOFLAGS) $^ -o $@

$(LIB_STATIC_NAME):$(LIBOBJS)
	$(AR) $@ $^

#ar -r $(LIBNAME) $(LIBOBJS)

clean:
	rm -rf $(LIBOBJS) $(LIBNAME) *.o $(LIB_STATIC_NAME)

    
