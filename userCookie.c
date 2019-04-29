/**
 * COMP30023 Project 1
 * This C file userCookie.c is linked with  userCookie.h
 * Name: Yuqiang Zhu. ID: 853912
 * Email: yuqiangz@student.unimelb.edu.au
 *
 */
 
#include "userCookie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct player {
    int id;
    char *username;
    int keywordListSize;
    char **keywordListPtr;
};

struct cookieList {
	int size;
	struct player *playerRecord;
};

// function prototype
struct cookieList * createRecordList();
bool playerExist(int playerId, struct cookieList *recordListPtr);
int getRecordSize(struct cookieList *recordListPtr);
int newPlayerRecord(struct cookieList *recordListPtr);
char *getUsername(int playerId, struct cookieList *recordListPtr);
void setUsername(struct cookieList *recordListPtr, int playerId, char *username);
bool checkPlayerKeywordList(int playerId, char *keyword, 
	struct cookieList *recordListPtr);
bool addPlayerKeyword(char *keyword, int playerId, 
	struct cookieList *recordListPtr);
char *getAllKeywords(int playerId, struct cookieList *recordListPtr);
bool freePlayerkeywordList(int playerId, struct cookieList *recordListPtr);
void freeRecordList(struct cookieList *recordListPtr);

/* create a new cookieList to store the cookie information */
struct cookieList *createRecordList(){
	struct cookieList *recordListPtr = 
		(struct cookieList *)malloc(sizeof(struct cookieList));
	recordListPtr->playerRecord = NULL;
	recordListPtr->size = 0;
	return recordListPtr;
}

bool playerExist(int playerId, struct cookieList *recordListPtr) {
	if (recordListPtr->size == 0) {
		return false;
	}
	int i;
	for (i = 0; i < recordListPtr->size; i++) {
		if (recordListPtr->playerRecord[i].id == playerId) {
			return true;
		}
	}
	return false;
}

int getRecordSize(struct cookieList *recordListPtr) {
	return recordListPtr->size;
}

/* add a new player cookie infomation */
int newPlayerRecord(struct cookieList *recordListPtr) {
	int newId = recordListPtr->size;
	recordListPtr->playerRecord = (struct player *)realloc(
			recordListPtr->playerRecord, sizeof(struct player) 
			* (recordListPtr->size + 1));
	recordListPtr->playerRecord[recordListPtr->size].id = recordListPtr->size;
	recordListPtr->playerRecord[recordListPtr->size].username = NULL;
	recordListPtr->playerRecord[recordListPtr->size].keywordListPtr = NULL;
	recordListPtr->playerRecord[recordListPtr->size].keywordListSize = 0;
	recordListPtr->size += 1;
	return newId;
}

char *getUsername(int playerId, struct cookieList *recordListPtr) {
	if (recordListPtr->playerRecord == NULL ||
		 recordListPtr->playerRecord[playerId].username == NULL) {
		return NULL;
	}
	return recordListPtr->playerRecord[playerId].username;
}

void setUsername(struct cookieList *recordListPtr, int playerId, char *username) {
	recordListPtr->playerRecord[playerId].username = 
		(char *)calloc(strlen(username)+1, sizeof(char));
	strcpy(recordListPtr->playerRecord[playerId].username, username);
}

/* check this keyword whether is ever submitted by this player */
bool checkPlayerKeywordList(int playerId, char *keyword, 
	struct cookieList *recordListPtr) {
	if (playerExist(playerId, recordListPtr) == false) {
		exit(EXIT_FAILURE);
	}
	int i;
	char **keywordListPtr = 
		recordListPtr->playerRecord[playerId].keywordListPtr;
	for (i = 0; i < recordListPtr->playerRecord[playerId].keywordListSize; i++){
		if (strcmp(keyword, keywordListPtr[i]) == 0){
			return true;
		}
	}
	return false;
}

/* add the keyword into player's keyword list */
bool addPlayerKeyword(char *keyword, int playerId, 
	struct cookieList *recordListPtr) {
	if (playerExist(playerId, recordListPtr) == false) {
		exit(EXIT_FAILURE);
	}
	int keywordListSize = recordListPtr->playerRecord[playerId].keywordListSize;
	recordListPtr->playerRecord[playerId].keywordListPtr = (char **)realloc
		(recordListPtr->playerRecord[playerId].keywordListPtr, 
		sizeof(char *) * (keywordListSize + 1));
	recordListPtr->playerRecord[playerId].keywordListPtr[keywordListSize] 
		= keyword;
	recordListPtr->playerRecord[playerId].keywordListSize++;
	return true;
}

char *getAllKeywords(int playerId, struct cookieList *recordListPtr) {
	if (recordListPtr->playerRecord[playerId].keywordListSize == 0) {
		//printf("return NULL");
		return NULL;
	}

	char *allKeywords = NULL;
	int keywordListSize = recordListPtr->playerRecord[playerId].keywordListSize;
	int allKeywordsSize = 0;
	int i;
	int temp;
	for (i = 0; i < keywordListSize; i++) {
		temp = strlen(recordListPtr->playerRecord[playerId].keywordListPtr[i]);
		//printf("%d length is %d\n\n", i, temp);
		allKeywordsSize += temp;
		allKeywordsSize++;
	}
	allKeywords = (char *)calloc(allKeywordsSize, sizeof(char));
	for (i = 0; i < keywordListSize; i++) {
		strcat(allKeywords, 
			recordListPtr->playerRecord[playerId].keywordListPtr[i]);
		if (i == keywordListSize - 1){
			continue;
		}
		strcat(allKeywords, ",");
	}
    strcat(allKeywords, "\n");
	return allKeywords;
}

bool freePlayerkeywordList(int playerId, struct cookieList *recordListPtr) {
	if (playerExist(playerId, recordListPtr) == false) {
		exit(EXIT_FAILURE);
	}
	if (recordListPtr->playerRecord[playerId].keywordListPtr == NULL) {
		return true;
	}
	int i;
	for(i = 0; i < recordListPtr->playerRecord[playerId].keywordListSize; i++) {
		free(recordListPtr->playerRecord[playerId].keywordListPtr[i]);
	}
	recordListPtr->playerRecord[playerId].keywordListSize = 0;
	return true;	
}

void freeRecordList(struct cookieList *recordListPtr) {
	if (recordListPtr == NULL) {
		return;
	}
	int i, j;
	int size = recordListPtr->size;
	for(i = 0; i < size; i++) {
		free(recordListPtr->playerRecord[i].username);
		
		if (recordListPtr->playerRecord[i].keywordListPtr == NULL) {
			continue;
		}
		for(j = 0; j < recordListPtr->playerRecord[i].keywordListSize; j++) {
			free(recordListPtr->playerRecord[i].keywordListPtr[j]);
		}
		free(recordListPtr->playerRecord[i].keywordListPtr);
	}
	free(recordListPtr->playerRecord);
	free(recordListPtr);
}