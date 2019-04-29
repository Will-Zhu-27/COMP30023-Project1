/**
 * COMP30023 Project 1
 * This header file playerRecord.h is used in imageTaggerServer.c 
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */
#include <stdbool.h>
struct player;
struct cookieList;

struct cookieList *createRecordList();
bool playerExist(int playerId, struct cookieList *recordListPtr);
int getRecordSize(struct cookieList *recordListPtr);
int newPlayerRecord(struct cookieList *recordListPtr);
char *getUsername(int playerId, struct cookieList *recordListPtr);
void setUsername(struct cookieList *recordListPtr, int playerId, char *username);
bool checkPlayerKeywordList(int playerId, char *keyword, struct cookieList *recordListPtr);
bool addPlayerKeyword(char *keyword, int playerId, struct cookieList *recordListPtr);
char *getAllKeywords(int playerId, struct cookieList *recordListPtr);
bool freePlayerkeywordList(int playerId, struct cookieList *recordListPtr);
void freeRecordList(struct cookieList *recordListPtr);