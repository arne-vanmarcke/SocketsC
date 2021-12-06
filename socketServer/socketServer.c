#include "ud_ucase.h"
#include "gpiod.h"
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

int main(int argc, char *argv[])
{
    struct sockaddr_un svaddr, claddr;
    int sfd, j;
    ssize_t numBytes;
    socklen_t len;
    char buf[BUF_SIZE];

    typedef struct Gpiod{
        _gpio gpio;
        struct gpiod_line *line;
        char state;
    }_gpiod;

    _gpiod _Gpiod;
    _Gpiod.state=0;

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
        errExit("socket");

    if (strlen(SV_SOCK_PATH) > sizeof(svaddr.sun_path) - 1)
        fatal("Server socket path too long: %s", SV_SOCK_PATH);

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SV_SOCK_PATH);

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    for (;;) {
        len = sizeof(struct sockaddr_un);
        numBytes = recvfrom(sfd, buf, BUF_SIZE, 0,
                            (struct sockaddr *) &claddr, &len);
        if (numBytes == -1)
            errExit("recvfrom");

        printf("Server received %ld bytes from %s\n", (long) numBytes,
                claddr.sun_path);

        printf("Pinnr:%d\n",buf[0]);
        printf("Time:%d\n",buf[1]);
        _Gpiod.gpio.pin=buf[0];
        _Gpiod.gpio.t_speed=buf[1];

        char *chipname = "gpiochip0";
        struct gpiod_chip *chip;
        int ret;

        for(;;){
            chip=gpiod_chip_open_by_name(chipname);
            if(!chip){
                errExit("no chip");
            }

            _Gpiod.line=gpiod_chip_get_line(chip, _Gpiod.gpio.pin);
            if(!_Gpiod.line){
                errExit("no line");
            }

            ret = gpiod_line_request_output(_Gpiod.line, CONSUMER,_Gpiod.state);
            if(ret!=0){
                errExit("no request output");
            }
            _Gpiod.state^=1;
            gpiod_line_set_value(_Gpiod.line,_Gpiod.state);

            printf("GpioState: %d\n",_Gpiod.state);

            gpiod_line_release(_Gpiod.line);
            gpiod_chip_close(chip);

            char bytes[]={_Gpiod.gpio.pin, _Gpiod.state};

            if (sendto(sfd, &bytes, sizeof(bytes), 0, (struct sockaddr *) &claddr, len) != sizeof(bytes))
                fatal("sendto");
            sleep(_Gpiod.gpio.t_speed);
        }
    }
}
