
SRCS= epoll_test.c opt_test.c server_init.c
PROG= epoll_test
CC=gcc
CFLAGS=-g
OBJS=$(SRCS:.c=.o)
$(PROG):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(OBJS):epoll_test.h opt_test.h server_init.h
clean:
	rm -rf $(OBJS)
