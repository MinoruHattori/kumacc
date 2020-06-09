CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kumacc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): kumacc.h

test: kumacc
	./test.sh

clean:
	rm -f kumacc *.o *~ tmp*

.PHONY: test clean
