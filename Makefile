CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS = -lsqlite3 -lcmocka

SRCS = src/main.c src/database.c src/parser.c src/toml.c src/utils.c
OBJS = $(SRCS:.c=.o)

TARGET = pipsqueak
TEST_TARGET = test_runner

MANPAGE = man/pipsqueak.1

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile the test files and link with CMocka
$(TEST_TARGET): tests/test_database.c src/database.o src/utils.o
	$(CC) $(CFLAGS) -Iinclude -o $(TEST_TARGET) tests/test_database.c src/database.o src/utils.o $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	cp $(MANPAGE) /usr/local/share/man/man1/

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJS) tests/test_database.o

.PHONY: all clean install test

