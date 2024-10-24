# Makefile for periodic pthread example

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pthread -o -D_GNU_SOURCE

# Source files
SRC = main.c # Replace with your source file name

# Target executable name
TARGET = task_d

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lwiringPi

# Clean target to remove compiled files
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
