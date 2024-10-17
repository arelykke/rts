# Define the compiler and compilation flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# List of source files
SOURCES = main.c dinner.c getters_setters.c init.c parsing.c philo.c safe_functions.c sync_utils.c utils.c write.c monitoring.c

# Create object file names by replacing .c with .o
OBJECTS = $(SOURCES:.c=.o)

# Define the target executable name
TARGET = philo

# Default target: compile the program
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean target to remove compiled files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
