#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_SIZE 2048


int main() {
    int semFd;
    int shmFd;
    char *shmAddr;
    char buf[SHM_SIZE + 50];
    struct shmid_ds info;
    struct sembuf semBuf[2] = {{0, -1, 0}, {0, 2, 0}};


    for(int i = 0; i < 30; ++i) {
        semFd = semget(10, 0, IPC_NOWAIT);
        if(semFd == -1) {
            sleep(1);
        } else {
            break;
        }
    }

    if(semFd == -1) {
        perror("Error occurred while creating set of semaphores\n");
        return -1;
    }
    semop(semFd, &semBuf[0], 1);
    
    for(int i = 0; i < 45; ++i) {
        shmFd = shmget(10, 0, IPC_NOWAIT);
        if(shmFd == -1) {
            sleep(1);
        } else {
            break;
        }
    }
    if(shmFd == -1) {
        perror("Error occurred while creating shared memory\n");
        semop(semFd, &semBuf[1], 1);
        return -1;
    }

    shmAddr = shmat(shmFd, 0, 0);
    if(shmAddr == (char *) -1) {
        perror("Error occurred while attaching shared memory to the process\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));

    if(strcmp(shmAddr, "") != 0) {
        strcpy(buf, "wc -l ");
        strcat(buf, shmAddr);
        strcat(buf, " | awk '{if ($1 > 10 && $2 != \"итого\") {print $2}}'");
        
        system(buf);
    } else {
        printf("No files\n");
    }

    if(shmctl(shmFd, IPC_STAT, &info) == -1) {
        perror("Error occurred while getting control parameters of the shared memory\n");
        return -1;
    } else {
        printf("Number of attached processes to shared memory is %lu\n", info.shm_nattch);
    }

    semop(semFd, &semBuf[1], 1);

    shmdt(shmAddr);

    return 0;
}
