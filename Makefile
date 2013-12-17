INCS=everything.h bitio.h tab.h dict.h	comp.h			#include .h files
SRCS=main.c  bitio.c tab.c dict.c comp.c		#include .c files
PROG=lz78			#programm name
CFLAGS= -Wall -O2 -g	#compiler options
OBJS= $(SRCS:.c=.o)
$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)
	rm *.o
