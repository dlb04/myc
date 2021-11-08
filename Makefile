.PHONY: clean all

CC = cc
CFLAGS = -g -Wall -Wextra -std=c11
CPPFLAGS +=

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

all: myc

myc: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(OBJ)
	rm myc
