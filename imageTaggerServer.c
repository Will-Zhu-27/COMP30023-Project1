/**
 * COMP30023 Project 1
 * This C file imageTaggerServer.c is linked with  imageTaggerServer.h
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 */
 
// my header file
#include "imageTaggerServer.h"
#include "imageTaggerConstant.h"

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

struct player {
    int id;
    char *username;
    int keywordListSize;
    char **keywordListPtr;
    bool inGame;
    bool win;
};

struct record {
	int size;
	struct player *playerRecord;
};

/* function prototype */
void runServer(char *mainProgram, char *ip, char *port);
bool sendPage(int sockfd, char *page);
static bool handle_http_request(int sockfd, struct record **recordListPtr);
bool sendDynamicPage(int sockfd, char *page, char *newContent, int before);
METHOD getMethod(char **buffPtr);
char *createIdCookie(int lastestId);
int getIdCookie(char *data);
bool sendInitialPage(int sockfd, char *page, struct record **recordListPtr);
char *createHeaderwithNewIdCookie(struct record **recordListPtr);
void freeRecord(struct record **recordListPtr);
bool addPlayerRecord(struct record **recordListPtr);
void setUsername(struct record **recordListPtr, int index, char *username);
bool setPlayerInGame(int index, bool newStatus, struct record **recordListPtr);
bool checkRivalStatus(int requestedIndex, struct record **recordListPtr);
bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr);
int getRivalId(int requestedIndex, struct record **recordListPtr);
bool checkRivalKeywordList(int requestedIndex, char *keyword, struct record **recordListPtr);
bool checkWinningPlayer(struct record **recordListPtr);
char *getAllKeywords(int index, struct record **recordListPtr);
bool sendAcceptedPage(char *keyword, int index, struct record **recordListPtr, int sockfd);
char *getKeyword(char *data);

void runServer(char *mainProgram, char *ip, char *port) {
	// record players who have ever connected the server
	struct record *recordList = (struct record *)malloc(sizeof(struct record));
	recordList->playerRecord = NULL;
	recordList->size = 0;
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
                else if (!handle_http_request(i, &recordList)) {
                    close(i);
                    FD_CLR(i, &masterfds);
                }
            }
        }
    }
    freeRecord(&recordList);
}

static bool handle_http_request(int sockfd, struct record **recordListPtr) {
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

    if (method == GET) {
        if (getIdCookie(curr) == -1) {
            if (!sendInitialPage(sockfd, WELCOME_PAGE, recordListPtr)) {
                return false;
            }
        } else {
        	printf("\n\nAccording to Cookies username: %s\n\n", (*recordListPtr)->playerRecord[getIdCookie(curr)].username);
            if ((*recordListPtr)->playerRecord[getIdCookie(curr)].username == NULL){
                if (!sendPage(sockfd, WELCOME_PAGE)) {
                    return false;
                }
            } else {
                sendDynamicPage(sockfd, MAIN_MENU_PAGE, (*recordListPtr)->playerRecord[getIdCookie(curr)].username, 214);
            }
        }
    } else if (method == GET_START) {
        if (!sendPage(sockfd, GAME_PLAYING_PAGE))
        {
            return false;
        }
        setPlayerInGame(getIdCookie(curr), true, recordListPtr);
    } else if (method == POST_USER) {
        //printf("POST:\n%s\n\nPOST END\n\n", buff);
        char *username = strstr(buff, "user=") + 5;
        //(*recordListPtr)->playerRecord[getIdCookie(curr)].username = username;
        setUsername(recordListPtr, getIdCookie(curr), username);
        printf("\n\nNow recordListPtr gets a username: %s\n\n", (*recordListPtr)->playerRecord[getIdCookie(curr)].username);
        //printf("username = %s\n\n\n", username);
        if (!sendDynamicPage(sockfd, MAIN_MENU_PAGE, username, 214)) {
            return false;
        }
    } else if(method == POST_GUESS) {
    	if(checkRivalStatus(getIdCookie(curr), recordListPtr) == false) {
    		if (!sendPage(sockfd, KEYWORD_DISCARDED_PAGE))
        	{
            	return false;
        	}
		} else if (checkWinningPlayer(recordListPtr) == true) {
			if (!sendPage(sockfd, GAME_COMPLETED_PAGE)) {
            	return false;
        	}
		} else {
			char *keyword = getKeyword(curr);
			if(checkRivalKeywordList(getIdCookie(curr), keyword, recordListPtr) == true) {
				(*recordListPtr)->playerRecord[getIdCookie(curr)].win = true;
                free(keyword);
				if (!sendPage(sockfd, GAME_COMPLETED_PAGE)) {
            		return false;
        		}
			} else {
				sendAcceptedPage(keyword, getIdCookie(curr), recordListPtr, sockfd);
			}
		}		
	} else if (method == POST_QUIT) {
        if (!sendPage(sockfd, GAME_OVER_PAGE)) {
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

char *getKeyword(char *data) {
	int length = strstr(data,"&guess=Guess") - strstr(data, "keyword=") - 8;
    printf("%d\n", length);
    char *keyword = (char *)calloc(length, sizeof(char));
    strncpy(keyword, strstr(data, "keyword=") + 8, length);
    printf("keyword = %s\n\n", keyword);
    return keyword;
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

bool sendInitialPage(int sockfd, char *page, struct record **recordListPtr) {
    struct stat st;
    int n;
    char buff[2049];
    stat(page, &st);
    char *header = createHeaderwithNewIdCookie(recordListPtr);
    
    n = sprintf(buff, header, st.st_size);
    // send the header first
    if (write(sockfd, buff, n) < 0)
    {
        perror("write");
        free(header);
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
        free(header);
        return false;
    }
    close(filefd);
    free(header);
    return true;
}

bool sendDynamicPage(int sockfd, char *page, char *newContent, int before) {
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
    if (write(sockfd, buff, size) < 0)
    {
        perror("write");
        free(newContentWithFormat);
        return false;
    }
    //printf("\n%s\n", buff);
    free(newContentWithFormat);
    return true;
}

bool sendAcceptedPage(char *keyword, int index, struct record **recordListPtr, int sockfd) {
	addPlayerKeyword(keyword, index, recordListPtr);
	char *newContent = getAllKeywords(index, recordListPtr);
    printf("\n\nThe keywordList = %s\n\n", newContent);
	bool ret = sendDynamicPage(sockfd, KEYWORD_ACCEPTED_PAGE, newContent, 264);
	free(newContent);
	return ret;
}

char *getAllKeywords(int index, struct record **recordListPtr) {
	if ((*recordListPtr)->playerRecord[index].keywordListSize == 0) {
		//printf("return NULL");
		return NULL;
	}

	char *allKeywords = NULL;
	int keywordListSize = (*recordListPtr)->playerRecord[index].keywordListSize;
	int allKeywordsSize = 0;
	int i;
	int temp;
	for (i = 0; i < keywordListSize; i++) {
		temp = strlen((*recordListPtr)->playerRecord[index].keywordListPtr[i]);
		//printf("%d length is %d\n\n", i, temp);
		allKeywordsSize += temp;
		allKeywordsSize++;
	}
	allKeywords = (char *)calloc(allKeywordsSize, sizeof(char));
	for (i = 0; i < keywordListSize; i++) {
		strcat(allKeywords, (*recordListPtr)->playerRecord[index].keywordListPtr[i]);
		if (i == keywordListSize - 1){
			continue;
		}
		strcat(allKeywords, ",");
	}
    strcat(allKeywords, "\n");
	return allKeywords;
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

char *createIdCookie(int id) {
    char *cookie = (char *)calloc(24, sizeof(char));
    strcat(cookie, "Set-Cookie: id = ");
    char newIdString[2];
    sprintf(newIdString, "%d", id);
    strcat(cookie, newIdString);
    strcat(cookie, "\r\n\r\n");
    //printf("cookie: %s\n\n", cookie);
    return cookie;
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

char *createHeaderwithNewIdCookie(struct record **recordListPtr) {
	int id = (*recordListPtr)->size;
    char *cookie = createIdCookie(id);
    char *header = (char *)malloc(sizeof(char) * (strlen(HTTP_200_FORMAT_NEED_COOKIE) + strlen(cookie) + 1));
    strcat(header, HTTP_200_FORMAT_NEED_COOKIE);
    strcat(header, cookie);
    addPlayerRecord(recordListPtr);
    return header;
}

bool addPlayerRecord(struct record **recordListPtr) {
	if((*recordListPtr)->size == 0) {
		(*recordListPtr)->playerRecord = (struct player *)malloc(sizeof(struct player));
	} else {
		(*recordListPtr)->playerRecord = realloc((*recordListPtr)->playerRecord, sizeof(struct player) * ((*recordListPtr)->size + 1));
	}
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].id = (*recordListPtr)->size;
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].username = NULL;
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].inGame = false;
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].keywordListPtr = NULL;
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].keywordListSize = 0;
	(*recordListPtr)->playerRecord[(*recordListPtr)->size].win = false;
	(*recordListPtr)->size += 1;
	return true;
}

bool checkWinningPlayer(struct record **recordListPtr) {
	int i;
	for (i = 0; i < (*recordListPtr)->size; i++) {
		if ((*recordListPtr)->playerRecord[i].win == true) {
			return true;
		}
	}
	return false;
}

bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr) {
	if ((*recordListPtr)->size <= index) {
		exit(EXIT_FAILURE);
	}
	//printf("\nkeyword is %s, length is %ld\n\n", keyword, strlen(keyword));
	//char *addedKeyword = (char *)malloc(sizeof(char) * (strlen(keyword)+1));
	//strcpy(addedKeyword, keyword);

	int keywordListSize = (*recordListPtr)->playerRecord[index].keywordListSize;
	//printf("Now the keyword list size is %d \n", keywordListSize);
	if (keywordListSize == 0) {
		//printf("use malloc\n");
		(*recordListPtr)->playerRecord[index].keywordListPtr = (char **)malloc(sizeof(char *));
	} else {
		//printf("use realloc\n");
		(*recordListPtr)->playerRecord[index].keywordListPtr = (char **)realloc((*recordListPtr)->playerRecord[index].keywordListPtr, sizeof(char *) * (keywordListSize + 1));
	}
	/*if(keywordListSize == 0) {
		*((*recordListPtr)->playerRecord[index].keywordListPtr) = addedKeyword;
	} else {
		*((*recordListPtr)->playerRecord[index].keywordListPtr)[keywordListSize] = addedKeyword;
	}*/
	
	(*recordListPtr)->playerRecord[index].keywordListPtr[keywordListSize] = keyword;
	(*recordListPtr)->playerRecord[index].keywordListSize++;
	return true;
}

void freeRecord(struct record **recordListPtr) {
	if ((*recordListPtr) == NULL) {
		return;
	}
	
	int i, j;
	int size = (*recordListPtr)->size;
	for(i = 0; i < size; i++) {
		free((*recordListPtr)->playerRecord[i].username);
		
		if ((*recordListPtr)->playerRecord[i].keywordListPtr == NULL) {
			continue;
		}
		for(j = 0; j < (*recordListPtr)->playerRecord[i].keywordListSize; j++) {
			free((*recordListPtr)->playerRecord[i].keywordListPtr[j]);
		}
		free((*recordListPtr)->playerRecord[i].keywordListPtr);
	}
	free((*recordListPtr)->playerRecord);
	free((*recordListPtr));
}

void setUsername(struct record **recordListPtr, int index, char *username) {
	(*recordListPtr)->playerRecord[index].username = (char *)calloc(strlen(username), sizeof(char));
	strcat((*recordListPtr)->playerRecord[index].username, username);
}

bool setPlayerInGame(int index, bool newStatus, struct record **recordListPtr) {
	if ((*recordListPtr)->size <= index) {
		return false;
	}
	(*recordListPtr)->playerRecord[index].inGame = newStatus;
	return true;
}

bool checkRivalStatus(int requestedIndex, struct record **recordListPtr) {
	if ((*recordListPtr)->size <= requestedIndex) {
		exit(EXIT_FAILURE);
	}
	int i;
	for (i = 0; i < (*recordListPtr)->size; i++) {
		if (i == requestedIndex) {
			continue;
		}
		if ((*recordListPtr)->playerRecord[i].inGame == true) {
			return true;
		}
	}
	return false;
}

bool checkRivalKeywordList(int requestedIndex, char *keyword, struct record **recordListPtr) {
	if ((*recordListPtr)->size <= requestedIndex) {
		exit(EXIT_FAILURE);
	}
	int rivalId = getRivalId(requestedIndex, recordListPtr);
	int i;
	char **keywordListPtr = (*recordListPtr)->playerRecord[rivalId].keywordListPtr;
	for (i = 0; i < (*recordListPtr)->playerRecord[rivalId].keywordListSize; i++) {
		if (strcmp(keyword, keywordListPtr[i]) == 0){
			return true;
		}
	}
	return false;
}

int getRivalId(int requestedIndex, struct record **recordListPtr) {
	int i;
	int size = (*recordListPtr)->size;
	for (i = 0; i < size; i++) {
		if (i == requestedIndex) {
			continue;
		}
		if ((*recordListPtr)->playerRecord[i].inGame == true) {
			return i;
		}
	}
	return -1;
}
