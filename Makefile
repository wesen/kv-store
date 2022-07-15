all: hello

%.o: %.c
	$(CC) $(CFLAGS) -c -Wall -Werror -o $@ $<

hello: hello.o
	$(CC) $(LDFLAGS) -Wall -Werror -o $@ $<

clean:
	rm -f *.o hello
