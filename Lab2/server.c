#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define SHM_SIZE 2048


int main() {
    int semFd;
    int shmFd;
    char *shmAddr;
    FILE *file;
    struct sembuf buf[2] = {{0, 1, 0}, {0, -5, 0}};
    
    semFd = semget(10, 1, IPC_CREAT | 0777);
    if(semFd == -1) {
        perror("Error occurred while creating set of semaphores\n");
        return -1;
    }
    
    shmFd = shmget(10, SHM_SIZE, IPC_CREAT | 0777);
    if(shmFd == -1) {
        perror("Error occurred while creating shared memory\n");
        return -1;
    }

    shmAddr = (char *) shmat(shmFd, NULL, 0);
    if(shmAddr == (char *) -1) {
        perror("Error occurred while attaching shared memory to the process\n");
        return -1;
    }

    file = popen("lpq -a | awk '{print $4}' | tail --lines=+2 | tr '\n' ' '", "r");
    fread(shmAddr, 1, SHM_SIZE, file);
    pclose(file);

    semop(semFd, &buf[0], 1);
    semop(semFd, &buf[1], 1);

    shmdt(shmAddr);
    if(shmctl(shmFd, IPC_RMID, NULL) == -1) {
        perror("Error occurred while deleting shared memory\n");
        return -1;
    }

    if(semctl(semFd, 0, IPC_RMID, NULL) == -1) {
        perror("Error occurred while deleting semafor\n");
        return -1;
    }

    return 0;
}
