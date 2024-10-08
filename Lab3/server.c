#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifndef MSGMAX
#define MSGMAX 2048
#endif


struct msgbuf {
    long msgtype;
    char msgtext[MSGMAX];
};


int main() {
    int fd;
    struct msgbuf message = {0, ""};
    FILE *file;
    char commands[3][64] = {"find -maxdepth 1 -type f -size +50c | tr '\n' ' '",
                            "file * | grep \"C source\" | cut -d: -f 1 | tr '\n' ' '",
                            "find -maxdepth 1 -mtime -2 -type f | tr '\n' ' '"};

    fd = msgget(10, 0);
    if(fd != -1 && msgctl(fd, IPC_RMID, NULL)) {
        perror("Permission denied: unable to delete message queue with key = 10");
    }

    fd = msgget(11, IPC_CREAT | 0777);

    for(int i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        memset(message.msgtext, 0, sizeof(message.msgtext));
        file = popen(commands[i], "r");
        fread(message.msgtext, 1, MSGMAX, file);
        pclose(file);

        message.msgtype += 1;
        if(strlen(message.msgtext) == 0) {
            strcpy(message.msgtext, "No files");
        }

        if(fd == -1 || msgsnd(fd, &message, strlen(message.msgtext) + 1, IPC_NOWAIT)) {
            char error[64];
            sprintf(error, "%s %d\n", "Error occurred while sending message №", i + 1);
            perror(error);
        }
    }

    return 0;
}
