/**
 * COMP30023 Project 1
 * This header file imageTest.h is used in imageTaggerServer.c 
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */
#include <stdbool.h>
#include "userCookie.h"
struct test;

struct test *createTest();
bool playerEnterGame(int playerId, struct test *imageTestPtr, struct cookieList *recordListPtr);
bool isTestFinished(struct test *imageTestPtr);
void playerFinishGame(int playerId, struct test *imageTestPtr, struct cookieList *recordListPtr);
void playerLeaveGame(int playerId, struct test *imageTestPtr, struct cookieList *recordListPtr);
bool checkEnoughPlayers(struct test *imageTestPtr);
bool playerInTest(int playerId, struct test *imageTestPtr);
bool isTestEmpty(struct test *imageTestPtr);
bool bothPlayerInGame(struct test *imageTestPtr);
bool checkRivalKeywordList(int requestedPlayerId, char *keyword, struct test *imageTestPtr, struct cookieList *recordListPtr);
int getRivalPlayerId(int requestedPlayerId, struct test *imageTestPtr);
void testFinish(struct test *imageTestPtr);
int getGameRound(struct test *imageTestPtr);
bool isPlayerInGame(int playerId, struct test *imageTestPtr);