INCS=everything.h bitio.h				#include .h files
SRCS=main.c  bitio.c 		#include .c files
PROG=lz78			#programm name
CFLAGS= -Wall -Werror -O2	#compiler options
OBJS= $(SRCS:.c=.o)
$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)
