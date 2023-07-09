CFLAGS = -Wall -Wextra -g -O2

all: exec

exec: bcode.o main.o
	$(CC) $(CFLAGS) $^ -o $@

bcode.o: bcode.c
	$(CC) $(CFLAGS) -c $^ -o $@

bcode.c: rules.py
	../bcgen rules.py

.PHONY: clean
clean:
	rm -rf *.o exec
