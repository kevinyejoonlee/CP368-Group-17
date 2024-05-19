CC = gcc
CFLAGS = -Wall
TARGET = directory

all: $(TARGET)

$(TARGET): directory.c
	$(CC) $(CFLAGS) -o $(TARGET) directory.c

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)
q