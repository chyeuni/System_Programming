CC=gcc
CFLAGS=-g -Wall
OBJS = 20181617.o
TARGET=20181617.out

$(TARGET): $(OBJS)
			 $(CC) -o $@ $(OBJS)

20181617.o: 20181617.h  20181617.c 

clean:
	rm -f *.o
	rm -f $(TARGET)
