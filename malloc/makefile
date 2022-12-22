CC=gcc
CC_FLAGS=-g -O0 -fPIC -shared

dyn:
	${CC} ${CC_FLAGS} mymalloc.c -o libmymalloc.so -ldl

exe:
	${CC} -g -O0 -o main main.c

run:
	LD_PRELOAD=./libmymalloc.so ./main