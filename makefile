# PingpongOS: ppos-core-aux.c
# 	gcc -Wall -o ppos-test ppos-core-aux.c disk.c pingpong-disco1.c libppos_static.a -lrt

# clean:
# 	-rm -rf *.o

CC = gcc
CFLAGS = -Wall
LFLAGS = -lrt
LIBS = -L/home/caio-/UTFPR/UTFPR2023.1/SO/ProjetoA/PingPongOS -lppos_static

OBJS = ppos-core-aux.o disk.o ppos_disk.o
PROG = pingpong-disco1 pingpong-disco2
 
# default rule
all: $(PROG)
 
# linking rules
$(PROG): %: $(OBJS) %.o
	$(CC) $^ -o $@ $(LIBS) $(LFLAGS)

# compilation rules
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

# compile with debug flags
debug: CFLAGS += -DDEBUG -g
debug: all

# remove temporary files
clean:
	-rm -rf *.o

# remove all except source code
purge: clean
	-rm -f $(PROG)

