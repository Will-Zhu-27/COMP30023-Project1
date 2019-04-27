#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

bool addPlayerRecord(struct record **recordListPtr);
bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr);
char *getAllKeywords(int index, struct record **recordListPtr);
void freeRecord(struct record **recordListPtr);

int main(int argc, char **argv) {
	struct record *recordList = (struct record *)malloc(sizeof(struct record));
	recordList->playerRecord = NULL;
	recordList->size = 0;
	addPlayerRecord(&recordList);
	addPlayerRecord(&recordList);
	addPlayerKeyword("1", 0, &recordList);
	addPlayerKeyword("yuqiang", 0, &recordList);
	addPlayerKeyword("will", 0, &recordList);
	addPlayerKeyword("ciao", 0, &recordList);
	addPlayerKeyword("2", 1, &recordList);
	addPlayerKeyword("yuqiangZhu", 1, &recordList);
	addPlayerKeyword("willG", 1, &recordList);
	addPlayerKeyword("ciao444", 1, &recordList);
	char *keywords1 = getAllKeywords(0, &recordList);
	printf("\n\n%s\n\n", keywords1);
	char *keywords2 = getAllKeywords(0, &recordList);
	printf("\n\n%s\n\n", keywords2);
	free(keywords1);
	free(keywords2);
	freeRecord(&recordList);
	return 0;
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

bool addPlayerKeyword(char *keyword, int index, struct record **recordListPtr) {
	if ((*recordListPtr)->size <= index) {
		exit(EXIT_FAILURE);
	}
	//printf("\nkeyword is %s, length is %ld\n\n", keyword, strlen(keyword));
	char *addedKeyword = (char *)malloc(sizeof(char) * (strlen(keyword)+1));
	strcpy(addedKeyword, keyword);

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
	
	(*recordListPtr)->playerRecord[index].keywordListPtr[keywordListSize] = addedKeyword;
	(*recordListPtr)->playerRecord[index].keywordListSize++;
	return true;
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
	return allKeywords;
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
