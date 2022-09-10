# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#  -lrt  needed to use shared memory
#  -pthread needed to use semaphores
CFLAGS  = -g -Wall -lrt -pthread -fsanitize=address -std=gnu99

# the build target executable:
objects = application view slave
all: $(objects)

$(objects): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(objects)