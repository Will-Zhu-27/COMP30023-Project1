/**
 * Name: Yuqiang Zhu. ID: 853912
 * email: yuqiangz@student.unimelb.edu.au
 * COMP30023 Project 1 
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// constants
static char const * const HTTP_200_FORMAT = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n\r\n";
static char const * const HTTP_400 = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_400_LENGTH = 47;
static char const * const HTTP_404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_404_LENGTH = 45;

#define WELCOME_PAGE "1_intro.html"
#define MAIN_MENU_PAGE "2_start.html"
#define GAME_PLAYING_PAGE "3_first_turn.html"
#define NUM_PLAYERS 2

// represents the types of method
typedef enum
{
    GET,
    GET_START,
    POST_USER,
    UNKNOWN
} METHOD;

bool sendPage(int sockfd, char *page);
static bool handle_http_request(int sockfd);
bool sendDynamicPage(int sockfd, char *page, char *newContent);
METHOD getMethod(char **buffPtr);

int main(int argc, char **argv) {
    // check whether input IP and port before start
    if (argc < 3) {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return 0;
    }

    char *ip = argv[1];
    char *port = argv[2];

    // create TCP socket which only accept IPv4
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // reuse the socket if possible
    int const reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // create and initialise address we will listen on
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // if ip parameter is not specified
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    // bind address to socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // listen on the socket
    listen(sockfd, NUM_PLAYERS);

    // initialise an active file descriptors set
    fd_set masterfds;
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    // record the maximum socket number
    int maxfd = sockfd;

    //server begins to work
    printf("%s server is now running at IP: %s on port %s\n",
        argv[0], ip, port);

    while (1) {
        // monitor file descriptors
        fd_set readfds = masterfds;
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // loop all possible descriptor
        for (int i = 0; i <= maxfd; ++i)
            // determine if the current file descriptor is active
            if (FD_ISSET(i, &readfds)) {
                // create new socket if there is new incoming connection request
                if (i == sockfd) {
                    struct sockaddr_in cliaddr;
                    socklen_t clilen = sizeof(cliaddr);
                    int newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
                    if (newsockfd < 0)
                        perror("accept");
                    else {
                        // add the socket to the set
                        FD_SET(newsockfd, &masterfds);
                        // update the maximum tracker
                        if (newsockfd > maxfd)
                            maxfd = newsockfd;
                        // print out the IP and the socket number
                        char ip[INET_ADDRSTRLEN];
                        printf(
                            "new connection from %s on socket %d\n",
                            // convert to human readable string
                            inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN),
                            newsockfd
                        );
                        //sendHtmlPage(i, WELCOME_PAGE);
                    }
                }
                // a request is sent from the client
                else if (!handle_http_request(i)) {
                    close(i);
                    FD_CLR(i, &masterfds);
                }
            }
    }
    return 0;
}

static bool handle_http_request(int sockfd)
{
    // try to read the request
    char buff[2049];
    int n = read(sockfd, buff, 2049);
    if (n <= 0)
    {
        if (n < 0)
            perror("read");
        else
            printf("socket %d close the connection\n", sockfd);
        return false;
    }

    // terminate the string
    buff[n] = 0;

    char * curr = buff;

    printf("\n************print received data****************\n");
    printf("%s", curr);
    printf("\n************END****************\n\n");

    // parse the method
    METHOD method = getMethod(&curr);
    if (method == UNKNOWN) {
        if (write(sockfd, HTTP_400, HTTP_400_LENGTH) < 0)
        {
            printf("\nMETHOD UNKNOWN\n");
            perror("write");
            return false;
        }
    }

    printf("\n\nGet method = %d \n\n", method);

    // sanitise the URI
    while (*curr == '.' || *curr == '/')
        ++curr;
    // assume the only valid request URI is "/" but it can be modified to accept more files

    if (method == GET) {
        if (!sendPage(sockfd, WELCOME_PAGE)) {
            return false;
        }
    } else if (method == GET_START) {
        if (!sendPage(sockfd, GAME_PLAYING_PAGE))
        {
            return false;
        }
    } else if (method == POST_USER) {
        printf("POST:\n%s\n\nPOST END\n\n", buff);
        char *username = strstr(buff, "user=") + 5;
        printf("username = %s\n\n\n", username);
        if (!sendDynamicPage(sockfd, MAIN_MENU_PAGE, username)) {
            return false;
        }
    } 
    // send 404
    else if (write(sockfd, HTTP_404, HTTP_404_LENGTH) < 0)
    {
        perror("write");
        return false;
    }

    return true;
}

bool sendPage(int sockfd, char *page) {
    struct stat st;
    int n;
    char buff[2049];
    stat(page, &st);
    n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
    // send the header first
    if (write(sockfd, buff, n) < 0)
    {
        perror("write");
        return false;
    }
    // send the file
    int filefd = open(page, O_RDONLY);
    do
    {
        n = sendfile(sockfd, filefd, NULL, 2048);
    } while (n > 0);
    if (n < 0)
    {
        perror("sendfile");
        close(filefd);
        return false;
    }
    close(filefd);
    return true;
}

bool sendDynamicPage(int sockfd, char *page, char *newContent)
{
    char formatStart[] = "<h3>"; 
    char formatEnd[] = "</h3>";
    int sizeWithFormat = strlen(formatStart) + strlen(formatEnd) + strlen(newContent) + 1;
    char *newContentWithFormat = (char *)calloc(sizeWithFormat, sizeof(char));
    strcat(newContentWithFormat, formatStart);
    strcat(newContentWithFormat, newContent);
    strcat(newContentWithFormat, formatEnd);
    printf("test the newContentWithFormat:\n%s\n\nEND\n\n", newContentWithFormat);
    printf("\n");
    int length = strlen(newContentWithFormat);
    int n;
    // get the size of the file
    struct stat st;
    char buff[2049];
    stat(page, &st);
    // increase file size to accommodate the username
    long size = st.st_size + length;
    n = sprintf(buff, HTTP_200_FORMAT, size);
    // send the header first
    if (write(sockfd, buff, n) < 0)
    {
        perror("write");
        free(newContentWithFormat);
        return false;
    }
    // read the content of the HTML file
    int filefd = open(page, O_RDONLY);
    n = read(filefd, buff, 2048);
    if (n < 0)
    {
        perror("read");
        close(filefd);
        free(newContentWithFormat);
        return false;
    }
    close(filefd);
    // move the trailing part backward
    int p1, p2;
    for (p1 = size - 1, p2 = p1 - length; p1 >= size - 214; --p1, --p2)
        buff[p1] = buff[p2];
    ++p2;
    // copy the username
    strncpy(buff + p2, newContentWithFormat, length);
    if (write(sockfd, buff, size) < 0)
    {
        perror("write");
        free(newContentWithFormat);
        return false;
    }
    printf("\n%s\n", buff);
    free(newContentWithFormat);
    return true;
}

METHOD getMethod(char **buffPtr) {
    METHOD method = UNKNOWN;
    if (strncmp(*buffPtr, "GET ", 4) == 0) {
        if (strstr(*buffPtr, "start") != NULL) {
            method = GET_START;
        } else {
          method = GET;  
        }
        *buffPtr += 4;
    } else if (strncmp(*buffPtr, "POST ", 5) == 0) {
        if (strstr(*buffPtr, "user=") != NULL) {
            method = POST_USER;
        }
        *buffPtr += 5;
    }
    return method;
}