CC = gcc
CFLAGS = -Wall

self-docker: self-docker.o
    $(CC) $(CFLAGS) -o self-docker self-docker.o
