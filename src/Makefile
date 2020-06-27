#
# Makefile for simcpu
#

#MYLIBDIR=../mynet
#MYLIB=-lmynet
#CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

#all: echo_server2 echo_client2
simcpu: main.o cpuboard.o
	${CC} -o $@ $^

clean:
	${RM} *.o simcpu
