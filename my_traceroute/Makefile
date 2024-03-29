CC=gcc

CFLAGS=-Wall -pedantic -g

EXECUTABLE=my_traceroute
CODE=*.c
HEADERS=*.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(CODE) $(HEADERS)
	$(CC) $(CFLAGS) $(CODE) -o $(EXECUTABLE)

clean:
	rm $(EXECUTABLE)
