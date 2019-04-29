/**
 * COMP30023 Project 1
 * This C file imageTaggerServer.c is linked with  imageTaggerServer.h
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 */
 
// my header file
#include "imageTaggerServer.h"
#include "imageTaggerConstant.h"
#include "userCookie.h"
#include "imageTest.h"

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

// represents the types of method
typedef enum
{
    GET,
    GET_START,
    POST_USER,
    POST_QUIT,
    POST_GUESS,
    UNKNOWN
} METHOD;

void runServer(char *mainProgram, char *ip, char *port);
static bool handle_http_request(int sockfd, struct test *imageTestPtr, struct cookieList *recordListPtr);
bool getMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordList);
bool getStartMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordList);
bool postUserMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr);
bool postGuessMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr);
bool postQuitMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr);
int getIdCookie(char *data);
bool sendPage(int sockfd, char *page, struct test *imageTestPtr);
bool sendInitialPage(int sockfd, struct cookieList *recordListPtr, struct test *imageTestPtr);
bool sendDynamicPage(int sockfd, char *page, char *newContent, int before, struct test *imageTestPtr);
bool sendAcceptedPage(int playerId, struct cookieList *recordListPtr, int sockfd, struct test *imageTestPtr);
char *createHeaderwithNewIdCookie(struct cookieList *recordListPtr);
char *createCookieString(int id);
char *setPageImageSrc(struct test *imageTestPtr, char *data);
char *getKeyword(char *data);
METHOD getMethod(char **buffPtr);


void runServer(char *mainProgram, char *ip, char *port) {
    struct cookieList *recordListPtr = createRecordList();
    struct test *imageTestPtr = createTest();

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
    listen(sockfd, 5);

    // initialise an active file descriptors set
    fd_set masterfds;
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    // record the maximum socket number
    int maxfd = sockfd;

    //server begins to work
    printf("%s server is now running at IP: %s on port %s\n",
        mainProgram, ip, port);

    while (1) {
        // monitor file descriptors
        fd_set readfds = masterfds;
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // loop all possible descriptor
        for (int i = 0; i <= maxfd; ++i) {
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
                else if (!handle_http_request(i, imageTestPtr, recordListPtr)) {
                    close(i);
                    FD_CLR(i, &masterfds);
                }
            }
        }
    }
    freeRecordList(recordListPtr);
}

static bool handle_http_request(int sockfd, struct test *imageTestPtr, struct cookieList *recordListPtr) {
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
    /*
    printf("\n************print received data****************\n");
    printf("%s", curr);
    printf("\n************END****************\n\n");
    */
    // parse the method
    METHOD method = getMethod(&curr);
    if (method == UNKNOWN) {
        if (write(sockfd, HTTP_400, HTTP_400_LENGTH) < 0) {
            printf("\nMETHOD UNKNOWN\n");
            perror("write");
            return false;
        }
    }

    printf("\n\nGet method = %d \n\n", method);

    // sanitise the URI
    while (*curr == '.' || *curr == '/') {
        ++curr;
    }
        
    // assume the only valid request URI is "/" but it can be modified to accept more files
    switch(method) {
        case GET: {
            return getMethodProcess(sockfd, curr, imageTestPtr, recordListPtr);
        }
        case GET_START: {
            return getStartMethodProcess(sockfd, curr, imageTestPtr, recordListPtr);
        }
        case POST_USER: {
            return postUserMethodProcess(sockfd, curr, imageTestPtr, recordListPtr);
        }
        case POST_GUESS: {
            return postGuessMethodProcess(sockfd, curr, imageTestPtr, recordListPtr);
        }
        case POST_QUIT: {
            return postQuitMethodProcess(sockfd, curr, imageTestPtr, recordListPtr);
        }
        default: {
            if (write(sockfd, HTTP_404, HTTP_404_LENGTH) < 0) {
                perror("write");
                return false;
            }
        }
    }
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
        } else if (strstr(*buffPtr, "quit") != NULL) {
            method = POST_QUIT;
        } else if (strstr(*buffPtr, "guess") != NULL) {
            method = POST_GUESS;
        } 
        *buffPtr += 5;
    }
    return method;
}

bool getMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int playerId = getIdCookie(data);
    if (playerId == -1)
    {
        return sendInitialPage(sockfd, recordListPtr, imageTestPtr);
    }
    else
    {
        char *username = getUsername(playerId, recordListPtr);
        printf("\n\nAccording to Cookies username: %s\n\n", username);
        if (username == NULL) {
            return sendPage(sockfd, WELCOME_PAGE, imageTestPtr);
        } else {
            return sendDynamicPage(sockfd, MAIN_MENU_PAGE, username, 214, imageTestPtr);
        }    
    }
}

bool getStartMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int playerId = getIdCookie(data);
    playerEnterGame(playerId, imageTestPtr, recordListPtr);
    return sendPage(sockfd, GAME_PLAYING_PAGE, imageTestPtr);
}

bool postUserMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int playerId = getIdCookie(data);
    char *username = strstr(data, "user=") + 5;
    setUsername(recordListPtr, playerId, username);
    //printf("\n\nNow recordListPtr gets a username: %s\n\n", getUsername(playerId, recordListPtr));
    return sendDynamicPage(sockfd, MAIN_MENU_PAGE, username, 214, imageTestPtr);
}

bool postGuessMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int playerId = getIdCookie(data);
    if (isTestFinished(imageTestPtr) == true) {
        playerFinishGame(playerId, imageTestPtr, recordListPtr);
        return sendPage(sockfd, GAME_COMPLETED_PAGE, imageTestPtr);
    } if(isPlayerInGame(playerId, imageTestPtr) == false) {
        freePlayerkeywordList(playerId, recordListPtr);
        return sendPage(sockfd, GAME_COMPLETED_PAGE, imageTestPtr);
    }else if (bothPlayerInGame(imageTestPtr) == false) {
        return sendPage(sockfd, KEYWORD_DISCARDED_PAGE, imageTestPtr);
    } else {
        char *keyword = getKeyword(data);
        if (checkRivalKeywordList(playerId, keyword, imageTestPtr, recordListPtr) == true) {
            testFinish(imageTestPtr);
            //free(keyword);
            playerFinishGame(playerId, imageTestPtr, recordListPtr);
            return sendPage(sockfd, GAME_COMPLETED_PAGE, imageTestPtr);
        }
        else
        {
            addPlayerKeyword(keyword, playerId, recordListPtr);
            return sendAcceptedPage(playerId, recordListPtr, sockfd, imageTestPtr);
        }
    }
}

bool postQuitMethodProcess(int sockfd, char *data, struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int playerId = getIdCookie(data);
    playerLeaveGame(playerId, imageTestPtr, recordListPtr);
    return sendPage(sockfd, GAME_OVER_PAGE, imageTestPtr);
}

int getIdCookie(char *data) {
    char idString[2];
    char *start = strstr(data, "Cookie: id=");
    if (start == NULL) {
        return -1;
    }
    char *src = start + 11;
    strncpy(idString, src, 2);
    return atoi(idString);
}

bool sendPage(int sockfd, char *page, struct test *imageTestPtr) {
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
    // read the content of the HTML file
    int filefd = open(page, O_RDONLY);
    n = read(filefd, buff, 2048);
    if (n < 0)
    {
        perror("read");
        close(filefd);
        return false;
    }
    close(filefd);
    char *dynamicContent = setPageImageSrc(imageTestPtr, buff);
    long size = strlen(dynamicContent);
    if (write(sockfd, dynamicContent, size) < 0)
    {
        perror("write");
        return false;
    }
    //printf("\n%s\n", buff);
    //free(dynamicContent);
    return true;
}

bool sendInitialPage(int sockfd, struct cookieList *recordListPtr, struct test *imageTestPtr) {
    struct stat st;
    int n;
    char buff[2049];
    stat(WELCOME_PAGE, &st);
    char *header = createHeaderwithNewIdCookie(recordListPtr);
    n = sprintf(buff, header, st.st_size);
    // send the header first
    if (write(sockfd, buff, n) < 0)
    {
        perror("write");
        free(header);
        return false;
    }
    free(header);
    // read the content of the HTML file
    int filefd = open(WELCOME_PAGE, O_RDONLY);
    n = read(filefd, buff, 2048);
    if (n < 0)
    {
        perror("read");
        close(filefd);
        return false;
    }
    close(filefd);
    char *dynamicContent = setPageImageSrc(imageTestPtr, buff);
    long size = strlen(dynamicContent);
    if (write(sockfd, dynamicContent, size) < 0)
    {
        perror("write");
        free(dynamicContent);
        return false;
    }
    //printf("\n%s\n", buff);
    free(dynamicContent);
    return true;
}

bool sendDynamicPage(int sockfd, char *page, char *newContent, int before, struct test *imageTestPtr) {
    char formatStart[] = "<h3>"; 
    char formatEnd[] = "</h3>";
    int sizeWithFormat = strlen(formatStart) + strlen(formatEnd) + strlen(newContent) + 1;
    char *newContentWithFormat = (char *)calloc(sizeWithFormat, sizeof(char));
    strcat(newContentWithFormat, formatStart);
    strcat(newContentWithFormat, newContent);
    strcat(newContentWithFormat, formatEnd);
    //printf("test the newContentWithFormat:\n%s\n\nEND\n\n", newContentWithFormat);
    //printf("\n");
    int length = strlen(newContentWithFormat);
    int n;
    // get the size of the file
    struct stat st;
    char buff[2049];
    stat(page, &st);
    // increase file size to accommodate the username
    //long size = st.st_size + length;
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
    for (p1 = size - 1, p2 = p1 - length; p1 >= size - before; --p1, --p2)
        buff[p1] = buff[p2];
    ++p2;
    // copy the username
    strncpy(buff + p2, newContentWithFormat, length);
    char *dynamicContent = setPageImageSrc(imageTestPtr, buff);
    size = strlen(dynamicContent);
    if (write(sockfd, dynamicContent, size) < 0)
    {
        perror("write");
        free(newContentWithFormat);
        free(dynamicContent);
        return false;
    }
    //printf("\n%s\n", buff);
    free(newContentWithFormat);
    free(dynamicContent);
    return true;
}

bool sendAcceptedPage(int playerId, struct cookieList *recordListPtr, int sockfd, struct test *imageTestPtr) {
	char *keywordsString = getAllKeywords(playerId, recordListPtr);
    //printf("\n\nThe keywordList = %s\n\n", keywordsString);
	return sendDynamicPage(sockfd, KEYWORD_ACCEPTED_PAGE, keywordsString, 264, imageTestPtr);
}

char *createHeaderwithNewIdCookie(struct cookieList *recordListPtr) {
	int newId = newPlayerRecord(recordListPtr);
    char *cookieString = createCookieString(newId);
    char *header = (char *)malloc(sizeof(char) * (strlen(HTTP_200_FORMAT_NEED_COOKIE) + strlen(cookieString) + 1));
    strcat(header, HTTP_200_FORMAT_NEED_COOKIE);
    strcat(header, cookieString);
    return header;
}

char *createCookieString(int id) {
    char *cookieString = (char *)calloc(24, sizeof(char));
    strcat(cookieString, "Set-Cookie: id = ");
    char newIdString[2];
    sprintf(newIdString, "%d", id);
    strcat(cookieString, newIdString);
    strcat(cookieString, "\r\n\r\n");
    return cookieString;
}

char *setPageImageSrc(struct test *imageTestPtr, char *data) {
	if(strstr(data, IMAGE_SRC_PREFIX) == NULL) {
		return data;
	}
    char *aim = strstr(data, IMAGE_SRC_PREFIX) + 96;
    //printf("%s\n\n", aim);
    int location = strlen(data) - strlen(aim);
    //printf("%d\n\n", location);
    char *ret = (char *)calloc((strlen(data) + 20), sizeof(char));
    strncpy(ret, data, location);
    int suffix = getGameRound(imageTestPtr) % 4 + 1;
    char imageSrcSuffix[2];
    sprintf(imageSrcSuffix, "%d", suffix);
    strcat(ret, imageSrcSuffix);
    strcat(ret, aim + 1);
    //printf("%s\n\n", ret);
    return ret;
}

char *getKeyword(char *data) {
	int length = strstr(data,"&guess=Guess") - strstr(data, "keyword=") - 8;
    //printf("%d\n", length);
    char *keyword = (char *)calloc(length, sizeof(char));
    strncpy(keyword, strstr(data, "keyword=") + 8, length);
    //printf("keyword = %s\n\n", keyword);
    return keyword;
}
