//
// Created by alex on 27.08.22.
//

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef MSGMAX
#define MSGMAX 2048
#endif

struct msgbuf {
    long msgtype;
    char msgtext[MSGMAX];
};


int main() {
    int fd;
    struct msqid_ds buf;
    struct msgbuf msg;
    char command[MSGMAX + 10];


    memset(command, 0, strlen(command));
    fd = msgget(11, 0);

    if (fd == -1 || msgctl(fd, IPC_STAT, &buf)) {
        perror("Error occurred while getting message queue descriptor or message queue stats\n");
        exit(1);
    }

    if(buf.msg_qnum > 2) {
        if(msgrcv(fd, &msg, MSGMAX, 2, MSG_NOERROR) == -1) {
            perror("Error occurred while reading 2nd message from the message queue\n");
            exit(1);
        }
        if(strcmp(msg.msgtext, "No files") != 0) {
            strcpy(command, "wc -l ");
            strcat(command, msg.msgtext);
            system(command);
        } else {
            printf("%s\n", msg.msgtext);
        }
    } else {
        if(buf.msg_qnum == 0) {
            printf("There is no messages in the message queue with key = 10\n");
        } else {
            printf("Last message was sent %s", asctime(localtime(&buf.msg_stime)));
        }

        if(msgctl(fd, IPC_RMID, NULL) == -1) {
            perror("Error occurred while deleting message queue\n");
            exit(1);
        }
    }

    return 0;
}
