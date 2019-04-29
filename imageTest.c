/**
 * COMP30023 Project 1
 * This C file imageTest.c is linked with  imageTest.h
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */

#include "imageTest.h"
#include "userCookie.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test {
	int player1;
	int player2;
	int gameRound;
	bool player1InGame;
    bool player2InGame;
    bool win;
};

// function prototype
struct test *createTest();
bool playerEnterGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr);
bool isTestFinished(struct test *imageTestPtr);
void playerFinishGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr);
void playerLeaveGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr);
bool checkEnoughPlayers(struct test *imageTestPtr);
bool playerInTest(int playerId, struct test *imageTestPtr);
bool isTestEmpty(struct test *imageTestPtr);
bool bothPlayerInGame(struct test *imageTestPtr);
bool checkRivalKeywordList(int requestedPlayerId, char *keyword, 
    struct test *imageTestPtr, struct cookieList *recordListPtr);
int getRivalPlayerId(int requestedPlayerId, struct test *imageTestPtr);
void testFinish(struct test *imageTestPtr);
int getGameRound(struct test *imageTestPtr);
bool isPlayerInGame(int playerId, struct test *imageTestPtr);
void freeTest(struct test *imageTestPtr);

struct test *createTest() {
    struct test *newTestPtr = (struct test *)malloc(sizeof(struct test));
    newTestPtr->player1 = -1;
    newTestPtr->player2 = -1;
    newTestPtr->player1InGame = false;
    newTestPtr->player2InGame = false;
    newTestPtr->gameRound = 0;
    newTestPtr->win = false;
    return newTestPtr;
}

/* check whether the player restarts the game */
bool restartGame(int playerId, struct test *imageTestPtr) {
    if (playerInTest(playerId, imageTestPtr) == false) {
        return false;
    }
    if (checkEnoughPlayers(imageTestPtr) == false) {
        return false;
    }
    bool player1 = false;
    if (playerId == imageTestPtr->player1) {
        player1 = true;
    }
    if (imageTestPtr->player2InGame == false && 
        imageTestPtr->player1InGame == false) {
        if (player1 == true) {
            imageTestPtr->player1InGame = true;
        } else {
            imageTestPtr->player2InGame = true;
        }
        imageTestPtr->gameRound++;
        return true;
    }
    return false;
}

/* set the test infomation when player press the start button */
bool playerEnterGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr) {
	if (playerExist(playerId, recordListPtr) == false) {
        printf("Id %d is not in the cookie list!\n\n", playerId);
        return false;
    }
    
    if (restartGame(playerId, imageTestPtr) == true) {
        return true;
    }

    imageTestPtr->win = false;
    // player joins the game as first player
    if(isTestEmpty(imageTestPtr) == true) {
        imageTestPtr->player1 = playerId;
        imageTestPtr->player1InGame = true;
        imageTestPtr->gameRound = 0;
        printf("Id %d enters the game firstly\n\n", playerId);
        return true;
    }

    // player restarts the game
    if (playerId == imageTestPtr->player1) {
        imageTestPtr->player1InGame = true;
        printf("Id %d restarts the game\n\n", playerId);
        
    } else if (playerId == imageTestPtr->player2){
        imageTestPtr->player2InGame = true;
        printf("Id %d restarts the game\n\n", playerId);
    }
    // player joins the game
    else {
        if(imageTestPtr->player1 == -1) {
            imageTestPtr->player1 = playerId;
            imageTestPtr->player1InGame = true;
            printf("Id %d joins the game as 1st player\n\n", playerId);
        } else {
            imageTestPtr->player2 = playerId;
            imageTestPtr->player2InGame = true;
            printf("Id %d joins the game as 2nd player\n\n", playerId);
        }
    }
	return true;
}

bool isTestFinished(struct test *imageTestPtr) {
    return imageTestPtr->win;
}

void playerFinishGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr) {
    freePlayerkeywordList(playerId, recordListPtr);
    imageTestPtr->player1InGame = false;
    imageTestPtr->player2InGame = false;
}

void playerLeaveGame(int playerId, struct test *imageTestPtr, 
    struct cookieList *recordListPtr) {
    freePlayerkeywordList(playerId, recordListPtr);
    if (playerId == imageTestPtr->player1) {
        imageTestPtr->player1 = -1;
        imageTestPtr->player1InGame = false;
        printf("Id %d quits the game as 1st player\n\n", playerId);
    } else if(playerId == imageTestPtr->player2){
        imageTestPtr->player2 = -1;
        imageTestPtr->player2InGame = false;
        printf("Id %d quits the game as 2nd player\n\n", playerId);
    }
}

bool checkEnoughPlayers(struct test *imageTestPtr) {
    if (imageTestPtr->player1 != -1 && imageTestPtr->player2 != -1) {
        return true;
    }
    return false;
}

bool playerInTest(int playerId, struct test *imageTestPtr) {
    if (imageTestPtr->player1 == playerId 
        || imageTestPtr->player2 == playerId) {
        return true;
    } else {
        return false;
    }
}

bool isTestEmpty(struct test *imageTestPtr) {
    if (imageTestPtr->player1 == -1 && imageTestPtr->player2 == -1) {
        return true;
    }
    return false;
}

bool bothPlayerInGame(struct test *imageTestPtr) {
    if (imageTestPtr->player1InGame == true 
        && imageTestPtr->player2InGame == true) {
        return true;
    }
    return false;
}

bool isPlayerInGame(int playerId, struct test *imageTestPtr) {
    if (playerId == imageTestPtr->player1) {
        return imageTestPtr->player1InGame;
    }
    if (playerId == imageTestPtr->player2) {
        return imageTestPtr->player2InGame;
    }
    return false;
}

bool checkRivalKeywordList(int requestedPlayerId, char *keyword, 
    struct test *imageTestPtr, struct cookieList *recordListPtr) {
    int rivalPlayerId = getRivalPlayerId(requestedPlayerId, imageTestPtr);
    return checkPlayerKeywordList(rivalPlayerId, keyword, recordListPtr);
}

int getRivalPlayerId(int requestedPlayerId, struct test *imageTestPtr) {
    if (requestedPlayerId != imageTestPtr->player1) {
        return imageTestPtr->player1;
    } else {
        return imageTestPtr->player2;
    }
}

void testFinish(struct test *imageTestPtr) {
    imageTestPtr->win = true;
}

int getGameRound(struct test *imageTestPtr) {
    return imageTestPtr->gameRound;
}

void freeTest(struct test *imageTestPtr) {
    free(imageTestPtr);
}