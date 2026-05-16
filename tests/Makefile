CC     = gcc
CFLAGS = -g -O0 -no-pie
TARGET = target

all: $(TARGET)

$(TARGET): target.c
	$(CC) $(CFLAGS) -o $(TARGET) target.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
