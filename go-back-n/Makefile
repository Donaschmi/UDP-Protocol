CC = gcc
CFLAGS = -Wall  -Wshadow -Wextra -g
EXEC = sender receiver
LDFLAGS = -lcunit -lz -lm -Wl

all : $(EXEC)

.PHONY: server client test clean mrproper

receiver: receiver.o socket.o packet_implem.o server.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

sender: sender.o socket.o packet_implem.o client.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

tests: tests/test.o packet_implem.o socket.o client.o server.o
	$(CC) $^ -o test $(CFLAGS) $(LDFLAGS)
	export LD_LIBRARY_PATH

clean:
	rm -rf *.o tests/*.o

mrproper: clean
	rm -rf $(EXEC) test
