CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = covid

all: $(TARGET)

$(TARGET): covid.c
	$(CC) $(CFLAGS) -o $(TARGET) covid.c

clean:
	rm -f $(TARGET)

