#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "include/ewpdef.h"

#define BUFFER_SIZE 12000

int main(int argc, char *argv[]) {

    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    char buffer[BUFFER_SIZE];

    int port = 2525;

    /*
        Read command line arguments
    */

    for(int i = 1; i < argc; i++) {

        if(strcmp(argv[i], "-port") == 0 && i + 1 < argc) {

            port = atoi(argv[i + 1]);
        }
    }

    /*
        Create socket
    */

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd < 0) {

        printf("Socket creation failed\n");

        return 1;
    }

    /*
        Configure server
    */

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_addr.sin_port = htons(port);

    /*
        Bind
    */

    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {

        printf("Bind failed\n");

        close(server_fd);

        return 1;
    }

    /*
        Listen
    */

    listen(server_fd, 1);

    printf("Server listening on port %d\n", port);

    /*
        Accept
    */

    client_len = sizeof(client_addr);

    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    if(client_fd < 0) {

        printf("Accept failed\n");

        close(server_fd);

        return 1;
    }

    /*
        Send 220 greeting
    */

    struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT acceptMsg;

    memset(&acceptMsg, 0, sizeof(acceptMsg));

    memcpy(acceptMsg.stHead.acMagicNumber, "EWP", 3);

    memcpy(acceptMsg.stHead.acDataSize, "0064", 4);

    memcpy(acceptMsg.stHead.acDelimeter, "|", 1);

    memcpy(acceptMsg.acStatusCode, "220", 3);

    acceptMsg.acHardSpace[0] = ' ';

    snprintf(
        acceptMsg.acFormattedString,
        51,
        "127.0.0.1 SMTP SmtpTest"
    );

    send(client_fd, &acceptMsg, sizeof(acceptMsg), 0);

    /*
        Main communication loop
    */

    while(1) {

        memset(buffer, 0, BUFFER_SIZE);

        int received = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if(received <= 0) {

            break;
        }

        /*
            HELO
        */

        if(strstr(buffer, "HELO")) {

            struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO reply;

            memset(&reply, 0, sizeof(reply));

            memcpy(reply.stHead.acMagicNumber, "EWP", 3);

            memcpy(reply.stHead.acDataSize, "0064", 4);

            memcpy(reply.stHead.acDelimeter, "|", 1);

            memcpy(reply.acStatusCode, "250", 3);

            reply.acHardSpace[0] = ' ';

            snprintf(
                reply.acFormattedString,
                51,
                "127.0.0.1 Hello"
            );

            send(client_fd, &reply, sizeof(reply), 0);
        }

        /*
            MAIL FROM / RCPT TO
        */

        else if(strstr(buffer, "MAIL FROM") || strstr(buffer, "RCPT TO")) {

            struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY reply;

            memset(&reply, 0, sizeof(reply));

            memcpy(reply.stHead.acMagicNumber, "EWP", 3);

            memcpy(reply.stHead.acDataSize, "0064", 4);

            memcpy(reply.stHead.acDelimeter, "|", 1);

            memcpy(reply.acStatusCode, "250", 3);

            reply.acHardSpace[0] = ' ';

            snprintf(reply.acFormattedString, 51, "OK");

            send(client_fd, &reply, sizeof(reply), 0);
        }

        /*
            DATA
        */

        else if(strstr(buffer, "DATA")) {

            struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY reply;

            memset(&reply, 0, sizeof(reply));

            memcpy(reply.stHead.acMagicNumber, "EWP", 3);

            memcpy(reply.stHead.acDataSize, "0064", 4);

            memcpy(reply.stHead.acDelimeter, "|", 1);

            memcpy(reply.acStatusCode, "354", 3);

            reply.acHardSpace[0] = ' ';

            snprintf(reply.acFormattedString, 51, "Ready for message");

            send(client_fd, &reply, sizeof(reply), 0);

            /*
                Receive file content
            */

            memset(buffer, 0, BUFFER_SIZE);

            received = recv(client_fd, buffer, BUFFER_SIZE, 0);

            if(received > 0) {

                FILE *f = fopen("received.eml", "wb");

                fwrite(buffer, 1, received, f);

                fclose(f);
            }
        }

        /*
            QUIT
        */

        else if(strstr(buffer, "QUIT")) {

            struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY reply;

            memset(&reply, 0, sizeof(reply));

            memcpy(reply.stHead.acMagicNumber, "EWP", 3);

            memcpy(reply.stHead.acDataSize, "0064", 4);

            memcpy(reply.stHead.acDelimeter, "|", 1);

            memcpy(reply.acStatusCode, "221", 3);

            reply.acHardSpace[0] = ' ';

            snprintf(reply.acFormattedString, 51, "Closing connection");

            send(client_fd, &reply, sizeof(reply), 0);

            break;
        }
    }

    close(client_fd);

    close(server_fd);

    return 0;
}
