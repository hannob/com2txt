# Makefile for Turbo C
CC = tcc
CFLAGS =
OBJS = com2txt.obj hdr_data.obj encode.obj getopt.obj
LIBS = 

com2txt.exe: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS)

.c.obj:
	$(CC) $(CFLAGS) -c $<

com2txt.obj encode.obj: common.h
