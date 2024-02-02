CC=gcc

CFLAGS=-Wall -pedantic

all: lecture task

lecture: from_lec/sys1.c from_lec/sys2.c
	$(CC) $(CFLAGS) from_lec/sys1.c -o from_lec/sys1
	$(CC) $(CFLAGS) from_lec/sys2.c -o from_lec/sys2

task: remove_shm_from_sys_n.c
	$(CC) $(CFLAGS) remove_shm_from_sys_n.c -o remove_shm

clean:
	rm remove_shm from_lec/sys1 from_lec/sys2
