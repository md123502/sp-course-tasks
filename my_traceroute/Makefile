CC=gcc

CFLAGS=-Wall -pedantic -g -fsanitize=address

EXECUTABLE=my_traceroute
CODE=*.c
HEADERS=*.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(CODE) $(HEADERS)
	$(CC) $(CFLAGS) $(CODE) -o $(EXECUTABLE)

clean:
	rm $(EXECUTABLE)
