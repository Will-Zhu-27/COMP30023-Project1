/**
 * COMP30023 Project 1
 * This C file playerRecord.c is linked with  playerRecord.h
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */
 
#include "playerRecord.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

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

// function prototype
struct record *createRecordList();
void freeRecordList(struct record **recordListPtr);
bool addPlayerRecord(struct record **recordListPtr);
void setUsername(struct record **recordListPtr, int index, char *username);
bool setPlayerInGame(int index, bool newStatus, struct record **recordListPtr);
bool checkRivalStatus(int requestedIndex, struct record **recordListPtr);
bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr);
int getRivalId(int requestedIndex, struct record **recordListPtr);
bool checkRivalKeywordList(int requestedIndex, char *keyword, struct record **recordListPtr);
bool checkWinningPlayer(struct record **recordListPtr);
char *getAllKeywords(int index, struct record **recordListPtr);
char *getUsername(int index, struct record **recordListPtr);
int getRecordSize(struct record **recordListPtr);
bool setPlayerWin(int index, bool newStatus, struct record **recordListPtr);
bool freePlayerkeywordList(int index, struct record **recordListPtr);
void playerLeaveGame(int index, struct record **recordListPtr);

struct record *createRecordList() {
	struct record *recordList = (struct record *)malloc(sizeof(struct record));
	recordList->playerRecord = NULL;
	recordList->size = 0;
	return recordList;
}

void freeRecordList(struct record **recordListPtr) {
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

bool checkWinningPlayer(struct record **recordListPtr) {
	int i;
	for (i = 0; i < (*recordListPtr)->size; i++) {
		if ((*recordListPtr)->playerRecord[i].win == true) {
			return true;
		}
	}
	return false;
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

char *getUsername(int index, struct record **recordListPtr) {
	if ((*recordListPtr)->playerRecord == NULL || (*recordListPtr)->playerRecord[index].username == NULL) {
		return NULL;
	}
	return (*recordListPtr)->playerRecord[index].username;
}

int getRecordSize(struct record **recordListPtr) {
	return (*recordListPtr)->size;
}

bool setPlayerWin(int index, bool newStatus, struct record **recordListPtr) {
	if ((*recordListPtr)->playerRecord == NULL || (*recordListPtr)->size <= index) {
		return false;
	}
	(*recordListPtr)->playerRecord[index].win = newStatus;
	return true;
}

bool freePlayerkeywordList(int index, struct record **recordListPtr) {
	if ((*recordListPtr)->playerRecord == NULL || (*recordListPtr)->size <= index) {
		return false;
	}
	if ((*recordListPtr)->playerRecord[index].keywordListPtr == NULL) {
		return true;
	}
	int i;
	for(i = 0; i < (*recordListPtr)->playerRecord[index].keywordListSize; i++) {
		free((*recordListPtr)->playerRecord[index].keywordListPtr[i]);
	}
	free((*recordListPtr)->playerRecord[index].keywordListPtr);
	return true;	
}

void playerLeaveGame(int index, struct record **recordListPtr) {
	freePlayerkeywordList(index, recordListPtr);
	setPlayerInGame(index, false, recordListPtr);
}
