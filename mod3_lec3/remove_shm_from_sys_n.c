#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    int shmid;
    char pathname[] = "from_lec/sys1.c";
    key_t key;

    errno = 0;

    if ((key = ftok(pathname,1)) < 0) {
        perror("Can\'t generate key");
        return -1;
    }

    if ((shmid = shmget(key, 0, 0)) < 0) {
        if(errno == ENOENT) {
            printf("Our shared memory segment does not exist; nothing to delete\n");
            return -1;
        } 
        else {
            perror("Error when obtaining shmid");
            return -1;
        }
    }
    
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("Shmctl failed");
        return -1;
    }
    
    return 0;
}
