image_tagger: imageTaggerServer.o playerRecord.o image_tagger.o
	gcc -Wall imageTaggerServer.o playerRecord.o image_tagger.o -o image_tagger -g

image_tagger.o: imageTaggerServer.h image_tagger.c
	gcc -Wall -c image_tagger.c -g

imageTaggerServer.o: imageTaggerServer.h imageTaggerConstant.h playerRecord.h imageTaggerServer.c
	gcc -Wall -c imageTaggerServer.c -g

playerRecord.o: playerRecord.h playerRecord.c
	gcc -Wall -c playerRecord.c -g