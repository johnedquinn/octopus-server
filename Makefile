CC=		gcc
CFLAGS=		-g -Werror -Wall -std=gnu99 -Iinclude
LD=		gcc
LDFLAGS=	-L.
AR=		ar
ARFLAGS=	rcs
TARGETS=	bin/spidey

all:		$(TARGETS)

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) lib/*.a src/*.o *.log *.input

.PHONY:		all test clean

%.o: %.c
	@echo "Compiling $@..."
	$(CC) $(CFLAGS) -c -o $@ $^

lib/libspidey.a: src/forking.o src/handler.o src/request.o src/single.o src/socket.o src/utils.o
	@echo "Linking $@..."
	$(AR) $(ARFLAGS) $@ $^

bin/spidey: src/spidey.o lib/libspidey.a
	@echo "Linking $@..."
	$(CC) $(LDFLAGS) -o $@ $^

deploy:
	sudo setcap CAP_NET_BIND_SERVICE=+eip ./bin/spidey
