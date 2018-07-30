CC = gcc
CFLAGS = -g -O0
LIBS =  -lyaml
SRCS = main.c
TARGET = executor
all:
	gcc ${CFLAGS} ${SRCS} $(LIBS) -o ${TARGET}
#run:
#	./${TARGET}.out 4 16
clean:
	rm -rf ${TARGET}
	rm -rf *.o
