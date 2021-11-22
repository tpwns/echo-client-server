TARGET=../bin/ts
CPPFLAGS+=-Wall -O2
LDLIBS+=-pthread
#LDLIBS+=-lws2_32 # for mingw

all: echo-client echo-server

echo-client: echo-client.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

echo-server: echo-server.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f echo-client echo-server *.o
