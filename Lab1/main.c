#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>


sigjmp_buf buf;
int flag = 0;
char filename1[256];
char filename2[256];
int fileCounter = 1;
char command[300];


void personalHandler() {
    flag++;

    if(flag == 1 && strcmp(filename1, "") != 0) {
        strcpy(command, "wc -lwm ");
        strcat(command, filename1);
        system(command);
    }

    if(flag == 2 && strcmp(filename2, "") != 0) {
        strcpy(command, "wc -lwm ");
        strcat(command, filename2);
        system(command);
    }

    siglongjmp(buf, 1);
}


void getFilename(char *filename) {
    while(1) {
        memset(filename, 0, 256);
        if(fileCounter == 1) {
            printf("\nEnter first filename:\n");
        } else {
            printf("\nEnter second filename:\n");
        }

        read(0, filename, 256);
        filename[strlen(filename) - 1] = '\0';

        if(!access(filename, R_OK)) {
            fileCounter++;
            break;
        } else {
            printf("This file doesn't exist or it can't be read! Try again:\n");
        }
    }
}


int main() {
    int fd1;
    int fd2;
    struct stat fileStat1;
    struct stat fileStat2;
    char *newFile = NULL;
    size_t newFileSize;
    int pid;
    int fd3[2];
    char *sonFile;
    int fdNew;

    struct sigaction newSig;

    memset(filename1, 0, 256);
    memset(filename2, 0, 256);


    sigemptyset(&newSig.sa_mask);
    sigprocmask(0, 0, &newSig.sa_mask);
    newSig.sa_handler = personalHandler;
    newSig.sa_flags = 0;

    if(sigaction(SIGINT, &newSig, 0) == -1) {
        printf("ERROR!!!!\n");
        return -1;
    }

    sigsetjmp(buf, 1);
    getFilename(filename1);
    sigsetjmp(buf, 1);
    if(flag >= 1){
        strcpy(command, "wc -lwm ");
        strcat(command, filename1);

        system(command);
    }

    fd1 = open(filename1, 0);
    fstat(fd1, &fileStat1);

    sigsetjmp(buf, 1);
    getFilename(filename2);
    sigsetjmp(buf, 1);
    if(flag >= 2){
        strcpy(command, "wc -lwm ");
        strcat(command, filename2);

        system(command);
    }

    fd2 = open(filename2, 0);
    fstat(fd2, &fileStat2);

    newFileSize = fileStat1.st_size + fileStat2.st_size;

    newFile = malloc(newFileSize * sizeof(char));

    if(!newFile) {
        printf("memory exhausted\n");
        exit(1);
    }
    read(fd1, newFile, fileStat1.st_size);
    read(fd2, &newFile[fileStat1.st_size], fileStat2.st_size);

    system("rm -f newFile.txt");
    fdNew = creat("newFile.txt", 0664);
    write(fdNew, newFile, newFileSize);


    pipe(fd3);

    pid = fork();

    if(pid == 0) {
        close(fd3[1]);

        sonFile = malloc(newFileSize * sizeof(char));

        if(!sonFile) {
          printf("memory exhausted\n");
          exit(1);
        }

        read(fd3[0], sonFile, newFileSize);

        system("wc -lwm newFile.txt");

        exit(0);
    } else if(pid > 0) {
        close(fd3[0]);

        write(fd3[1], newFile, newFileSize);

        wait(0);
    } else {
        printf("some problems while executing fork() system call\n");
        exit(1);
    }

    sigsetjmp(buf, 1);

    sleep(3);

    return 0;
}