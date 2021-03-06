#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h> // for getnameinfo()
#include "gpiod.h"

// Usual socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

#define SIZE 1024
#define BACKLOG 10  // Passed to listen()

char *chipname = "gpiochip0";
struct gpiod_chip *chip;
struct gpiod_line *line;

void report(struct sockaddr_in *serverAddress);

int addInput(char *textptr,int pin){
    char Input[50];
    chip=gpiod_chip_open_by_name(chipname);
    if(!chip){
        return 0;
    }

    line=gpiod_chip_get_line(chip, pin);
    if(!line){
        return 0;
    }
    int ret = gpiod_line_request_input(line, CONSUMER);
    if(ret!=0){
        return 0;
    }
    sprintf(Input,"<div><span>Gpio %d: </span><span>%d</span></div>",pin,gpiod_line_get_value(line));
    strcat(textptr,Input);

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 1;
}
int addOutput(char *textptr,int pin){
    char Output[50];
    chip=gpiod_chip_open_by_name(chipname);
    if(!chip){
        return 0;
    }
    line=gpiod_chip_get_line(chip, pin);
    if(!line){
        return 0;
    }

    int ret = gpiod_line_request_output(line, CONSUMER,0);
    if(ret!=0){
        return 0;
    }
    sprintf(Output,"<div><span>Gpio %d: </span><span>%d</span></div>",pin,gpiod_line_get_value(line));
    strcat(textptr,Output);

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 1;
}

void setHttpHeader(char httpHeader[])
{
    char responseData[8000];
    strcat(responseData,"<html><head><title>Hello World example</title></head><body><h1> Hello World!</h1>");
    strcat(responseData,"<div><h1>Inputs</h1></div>");
    int err=addInput(responseData,24);
    if(!err)
        strcat(responseData,"<div><h1>err</h1></div>");
    strcat(responseData,"<div><h1>Outputs</h1></div>");
    err=addOutput(responseData,23);
    if(!err)
        strcat(responseData,"<div><h1>err</h1></div>");
    printf("Out: %d",err);
    strcat(responseData,"</body></html>");

    strcat(httpHeader, responseData);
}

int main(void)
{
    char httpHeader[8000] = "HTTP/1.1 200 OK\r\n\n";

    // Socket setup: creates an endpoint for communication, returns a descriptor
    // -----------------------------------------------------------------------------------------------------------------
    int serverSocket = socket(
        AF_INET,      // Domain: specifies protocol family
        SOCK_STREAM,  // Type: specifies communication semantics
        0             // Protocol: 0 because there is a single protocol for the specified family
    );

    // Construct local address structure
    // -----------------------------------------------------------------------------------------------------------------
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8001);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//inet_addr("127.0.0.1");

    // Bind socket to local address
    // -----------------------------------------------------------------------------------------------------------------
    // bind() assigns the address specified by serverAddress to the socket
    // referred to by the file descriptor serverSocket.
    bind(
        serverSocket,                         // file descriptor referring to a socket
        (struct sockaddr *) &serverAddress,   // Address to be assigned to the socket
        sizeof(serverAddress)                 // Size (bytes) of the address structure
    );

    // Mark socket to listen for incoming connections
    // -----------------------------------------------------------------------------------------------------------------
    int listening = listen(serverSocket, BACKLOG);
    if (listening < 0) {
        printf("Error: The server is not listening.\n");
        return 1;
    }
    report(&serverAddress);     // Custom report function
    setHttpHeader(httpHeader);  // Custom function to set header
    int clientSocket;

    // Wait for a connection, create a connected socket if a connection is pending
    // -----------------------------------------------------------------------------------------------------------------
    while(1) {
        clientSocket = accept(serverSocket, NULL, NULL);
        send(clientSocket, httpHeader, sizeof(httpHeader), 0);
        close(clientSocket);
    }
    return 0;
}

void report(struct sockaddr_in *serverAddress)
{
    char hostBuffer[INET6_ADDRSTRLEN];
    char serviceBuffer[NI_MAXSERV]; // defined in `<netdb.h>`
    socklen_t addr_len = sizeof(*serverAddress);
    int err = getnameinfo(
        (struct sockaddr *) serverAddress,
        addr_len,
        hostBuffer,
        sizeof(hostBuffer),
        serviceBuffer,
        sizeof(serviceBuffer),
        NI_NUMERICHOST
    );
    if (err != 0) {
        printf("It's not working!!\n");
    }
    printf("\n\n\tServer listening on http://%s:%s\n", hostBuffer, serviceBuffer);
}