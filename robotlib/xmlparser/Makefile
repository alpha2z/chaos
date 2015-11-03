LIBOBJS = tinystr.o tinyxml.o tinyxmlparser.o tinyxmlerror.o

LIBNAME = libtinyxml.so

SOFLAGS = -shared

TARGET = $(LIBNAME)

.c.o:
	$(CC) $(CC_ARGS)  -c $^ 

.cpp.o:
	$(CXX) $(CXX_ARGS)  -c $^

.o:
	$(CXX) $(CXX_ARGS) -o $@ $^ 

all:$(TARGET)

$(LIBNAME):$(LIBOBJS)
	$(CXX) $(SOFLAGS) $^ -o $@

mytest:mytest.o
	$(CXX) $(CXX_ARGS) -o $@ $^  -L./ -ltinyxml
clean:
	rm -rf $(LIBOBJS) $(LIBNAME) *.o

    
