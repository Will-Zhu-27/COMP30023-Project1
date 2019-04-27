/**
 * COMP30023 Project 1
 * This header file playerRecord.h is used in imageTaggerServer.c 
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */
#include <stdbool.h>
struct record;

struct record *createRecordList();
void freeRecordList(struct record **recordListPtr);
bool addPlayerRecord(struct record **recordListPtr);
void setUsername(struct record **recordListPtr, int index, char *username);
bool setPlayerInGame(int index, bool newStatus, struct record **recordListPtr);
bool setPlayerWin(int index, bool newStatus, struct record **recordListPtr);
bool checkRivalStatus(int requestedIndex, struct record **recordListPtr);
bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr);
int getRivalId(int requestedIndex, struct record **recordListPtr);
bool checkRivalKeywordList(int requestedIndex, char *keyword, struct record **recordListPtr);
bool checkWinningPlayer(struct record **recordListPtr);
char *getAllKeywords(int index, struct record **recordListPtr);
char *getUsername(int index, struct record **recordListPtr);
int getRecordSize(struct record **recordListPtr);
void playerLeaveGame(int index, struct record **recordListPtr);
