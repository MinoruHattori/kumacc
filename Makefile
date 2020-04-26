CFLAGS=-std=c11 -g -static -fno-common

kumacc: main.o
	$(CC) -o $@ $? $(LDFLAGS)

test: kumacc
	./test.sh

clean:
	rm -f kumacc *.o *~ tmp*

.PHONY: test clean
