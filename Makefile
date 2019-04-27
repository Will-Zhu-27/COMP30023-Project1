image_tagger: imageTaggerServer.o image_tagger.o
	gcc -Wall image_tagger.o imageTaggerServer.o -o image_tagger -g

image_tagger.o: imageTaggerServer.h image_tagger.c
	gcc -Wall -c image_tagger.c -g

imageTaggerServer.o: imageTaggerServer.h imageTaggerConstant.h imageTaggerServer.c
	gcc -Wall -c imageTaggerServer.c -g