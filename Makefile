# asetniop-term makefile
# Tim Brooks 2015 <brooks@skoorb.net>
#
# To debug, use:
# > DEBUG=<type> make
# where <type> is DEBUG, DEBUG_EVENT or DEBUG_PRINT

OPT= -std=c99 -Wall
ifdef DEBUG
OPT+= -g -D$(DEBUG)
endif

all: asetniop.exe

asetniop.c: gen_maps.py
	cog.py -r $@
	touch $@

asetniop.exe: asetniop.c
	$(CC) $(OPT) -o $@ $<

run: asetniop.exe
	./asetniop.exe /dev/input/by-path/*-kbd
