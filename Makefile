
# On Linux:
# DEFINES = -DCACHE_LINE_SIZE=$(getconf LEVEL1_DCACHE_LINESIZE)
DEFINES = -DCACHE_LINE_SIZE=64

CFLAGS = -O2 -Wall -g -std=gnu99 $(DEFINES)

all: remote solution

solution: localgraderlib.o shm_interface.o
	$(CC) $^ -o $@ $(LDFLAGS)

remote: remotegraderlib.o shm_interface.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o solution remote

