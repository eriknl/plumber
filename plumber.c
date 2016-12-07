/*
This file is part of plumber Copyright (C) 2016 Erik de Jong

plumber is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

plumber is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with plumber.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

pthread_t readTCPthread;

int unixSocketFd;
int tcpSocketFd;

volatile sig_atomic_t signalStatus = 0;

void * readTCP()
{
    int n;
    char buffer[65535];
    while ( (n = read(tcpSocketFd, buffer, sizeof(buffer)-1)) > 0) {
        if (send(unixSocketFd, buffer, n, 0) == -1) {
            perror("send");
            exit(1);
        }
    }
    return NULL;
}

void terminate(int sig)
{
    signalStatus = 1;
}

int main(int argc, char ** argv[])
{
    int iret1;
    int len, n;
    char buffer[65535];
    struct sockaddr_un remote;
    struct sockaddr_in serv_addr;
    
    if (argc < 4) {
        printf("3 parameters required %s <named piped> <remote host> <remote port>\n", argv[0]);
        exit(1);
    }
    
    signal(SIGINT, terminate); 
    if ((unixSocketFd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    printf("Trying to connect...\n");
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, argv[1]);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(unixSocketFd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }
    printf("Connected to unix socket\n");


    if((tcpSocketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error : Could not create socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    if(inet_pton(AF_INET, argv[2], &serv_addr.sin_addr)<=0) {
        perror("inet_pton error occured");
        exit(1);
    }
    if(connect(tcpSocketFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error : Connect Failed");
        exit(1);
    }
    printf("Connected to tcp socket\n");
    iret1 = pthread_create( &readTCPthread, NULL, readTCP, NULL);
    // exit when sigint received and last connection closed
    while(!signalStatus) {
        if ((n = recv(unixSocketFd, buffer, sizeof(buffer)-1, 0)) > 0) {
            write(tcpSocketFd, buffer, n);
        } else {
            if (n < 0) {
                perror("recv");
            } else {
                printf("Server closed connection\n");
            }
            exit(1);
        }
    }

    close(unixSocketFd);
    close(tcpSocketFd);

    return 0;
}
