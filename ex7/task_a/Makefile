# Compiler
CC = clang

# Compiler flags
CFLAGS = -std=gnu11 -pthread -Wall

# Linker flags
LDFLAGS = -lwiringPi -lwiringPiDev

# Source files
SRCS = main.c

# Output executable
TARGET = task_a

# Default rule
all: $(TARGET)

# Compile the target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)
