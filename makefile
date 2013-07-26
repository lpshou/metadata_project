CC=gcc
INCLUDE=/usr/local/include/
INCLUDE_ZOOKEEPER=/usr/local/include/c-client-src/
LIB=/usr/local/lib/
CFLAGS_ZOOKEEPER=-lpthread -lzookeeper_mt
CFLAGS_ZOOKEEPER=-lzookeeper_mt
CFLAGS_HIREDIS=-lhiredis

.PHONY: zookeeper_test clean

namespace_api:
	$(CC) -o namespace_api namespace_api.c -I$(INCLUDE_ZOOKEEPER) -L$(LIB) $(CFLAGS_ZOOKEEPER)

clean:
	rm namespace_api
