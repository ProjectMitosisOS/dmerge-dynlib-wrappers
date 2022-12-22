CC=gcc
CC_FLAGS=-g -O0 -fPIC -shared

dyn:
	${CC} ${CC_FLAGS} mymalloc.c -o libmymalloc.so -ldl
	gcc -g -O0 -fPIC -shared malloc.cc -o libmymalloc.so -ldl

exe:
	${CC} -g -O0 -o main main.c

run:
	LD_PRELOAD=./libmalloc_wrapper.so ./main
	LD_PRELOAD=./libmymalloc.so ./build/main
	# LD_PRELOAD=./libmymalloc_lib.so ./main