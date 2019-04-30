image_tagger: userCookie.o imageTest.o imageTaggerServer.o  image_tagger.o 
	gcc -Wall userCookie.o imageTest.o imageTaggerServer.o  image_tagger.o -o image_tagger -g
	
image_tagger.o: image_tagger.c imageTaggerServer.h
	gcc -Wall -c image_tagger.c -g
	
imageTaggerServer.o: imageTaggerServer.h imageTaggerServer.c imageTaggerConstant.h userCookie.h imageTest.h
	gcc -Wall -c imageTaggerServer.c -g
	
imageTest.o: imageTest.h imageTest.c userCookie.h
	gcc -Wall -c imageTest.c -g
	
userCookie.o: userCookie.h userCookie.c
	gcc -Wall -c userCookie.c -g
