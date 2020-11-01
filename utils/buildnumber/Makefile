# ----------------------------------------------------------
# Makefile for BuildNumber
#.
# The first time BuildNumber is created, the executable will
#   of course not yet exist, so BuildNumber can't make use
#   of itself on the very first make. See below and edit the
#   "all:" lines after BuildNumber is successfully compiled.
#
# BuildNumber (c) 2006 John M. Stoneham. All rights reserved.
# ----------------------------------------------------------

CC=gcc
CFLAGS=-c -Wall
# The following uses buildnumber in the current directory
# If you are adapting this Makefile for other projects, please
#   be sure to include the entire path. See the examples folder.
BN=buildnumber

# After BuildNumber is created, comment out the first "all:" 
#   line below and uncomment the second (only one can be used)
all: BuildNumber
# all: buildnumber BuildNumber

BuildNumber: buildnumber.o
	$(CC) buildnumber.o -o buildnumber

buildnumber.o: buildnumber.c
	$(CC) $(CFLAGS) buildnumber.c

buildnumber:
	$(BN)

clean:
	rm -f *.o buildnumber
