CC = clang
CFLAGS = -Wall -Wextra -Werror -std=gnu11
LDFLAGS = -lpthread
TARGET = taskd

all: $(TARGET)

$(TARGET): main.c barrier.o
	$(CC) $(CFLAGS) -o $(TARGET) main.c barrier.o $(LDFLAGS)

main.o: main.c barrier.h
	$(CC) $(CFLAGS) -c main.c

barrier.o: barrier.c barrier.h
	$(CC) $(CFLAGS) -c barrier.c

clean:
	rm -f $(TARGET)

.PHONY: clean
