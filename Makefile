all:
	gcc src/*/*.c -o ./bin/probe

# Set compiler flags
CFLAGS = -Wall -Wextra -g

all: main.o

main.o:
	gcc $(CFLAGS) src/*/*.c -o ./bin/probe
