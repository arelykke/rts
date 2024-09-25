# Makefile for compiling the dynamic array program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -fsanitize=address -std=gnu11
#CFLAGS = -g -std=gnu11

# Source files
SRCS = main.c array.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = taskc

# Build the target executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Clean up the build
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
