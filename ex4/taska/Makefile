CC = clang
CFLAGS = -Wall -Wextra -Werror -std=gnu11
LDFLAGS = -lpthread
TARGET = taska

all: $(TARGET)

$(TARGET): main.o barrier.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o barrier.o $(LDFLAGS)

main.o: main.c barrier.h
	$(CC) $(CFLAGS) -c main.c

barrier.o: barrier.c barrier.h
	$(CC) $(CFLAGS) -c barrier.c

sanitize: CFLAGS += -g -fsanitize=thread
sanitize: LDFLAGS += -fsanitize=thread
sanitize: $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean sanitize
