CC      = gcc
CFLAGS  = -Wall -Wextra -g -O0 -Iinclude
LIBS    = -lncurses -lcapstone
SRCS    = main.c src/log.c src/debugger.c src/registers.c src/memory.c \
          src/breakpoint.c src/elf_parser.c src/tui.c src/repl.c
TARGET  = rupture

all: $(TARGET)

$(TARGET): $(SRCS) include/debugger.h include/tui.h include/log.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

test-target:
	$(MAKE) -C tests

clean:
	rm -f $(TARGET)
	$(MAKE) -C tests clean

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/rupture
	@echo "installed rupture -> /usr/local/bin/rupture"

.PHONY: all clean install test-target
