#include "ud_ucase.h"

int main(int argc, char *argv[])
{
    struct sockaddr_un svaddr, claddr;
    int sfd, j;
    size_t msgLen=1;
    ssize_t numBytes;
    char resp[BUF_SIZE];
    //char ByteArr[2];
    
    _gpio _Gpio={23,2};
    // if (argc < 2 || strcmp(argv[1], "--help") == 0)
    //     usageErr("%s msg...\n", argv[0]);

    /* Create client socket; bind to unique pathname (based on PID) */
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
        errExit("socket");

    memset(&claddr, 0, sizeof(struct sockaddr_un));
    claddr.sun_family = AF_UNIX;
    snprintf(claddr.sun_path, sizeof(claddr.sun_path),
            "/tmp/ud_ucase_cl.%ld", (long) getpid());

    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    /* Construct address of server */

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (sendto(sfd, &_Gpio, sizeof(_Gpio), 0, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) != sizeof(_Gpio))
        fatal("sendto");
    for(;;){
        numBytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
        if (numBytes == -1)
            errExit("recvfrom");
        
        printf("Response %d: pin:%d, niveau: %d\n", j, (int) numBytes, resp[1],resp[2]);
    }
    remove(claddr.sun_path);
    exit(EXIT_SUCCESS);
}
