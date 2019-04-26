#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
struct player {
    int id;
    char *username;
    bool inGame;
};

struct record {
	int size;
	struct player *playerRecord;
};

bool add(struct record **recordListPtr);
void freeRecord(struct record **recordListPtr);

int main(int argc, char **argv) {
	struct record *recordList = (struct record *)malloc(sizeof(struct record));
	recordList->size = 0;
	recordList->playerRecord = NULL;
	add(&recordList);
	add(&recordList);
	add(&recordList);
	add(&recordList);
	printf("the size is %d\n", recordList->size);
	freeRecord(&recordList);
	return 0;
}

bool add(struct record **recordListPtr) {
	if((*recordListPtr)->size == 0){
		(*recordListPtr)->playerRecord = (struct player*)malloc(sizeof(struct player));
		(*recordListPtr)->playerRecord->id = (*recordListPtr)->size;
		(*recordListPtr)->playerRecord->username = NULL;
		(*recordListPtr)->playerRecord->inGame = false;
		(*recordListPtr)->size += 1;
		return true;
	} else {
		(*recordListPtr)->playerRecord = realloc((*recordListPtr)->playerRecord, sizeof(struct player) * ((*recordListPtr)->size + 1));
		(*recordListPtr)->playerRecord[(*recordListPtr)->size].id = (*recordListPtr)->size;
		(*recordListPtr)->playerRecord[(*recordListPtr)->size].username = NULL;
		(*recordListPtr)->playerRecord[(*recordListPtr)->size].inGame = false;
		(*recordListPtr)->size += 1;
		return true;
	}
	return false;
}

void freeRecord(struct record **recordListPtr) {
	free((*recordListPtr)->playerRecord);
	free((*recordListPtr));
}
