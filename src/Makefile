CC=g++
CCFLAGS=-g -std=c++11 -fPIC

OBS  = lib_wind_obos.o cable_vessel.o 

ifeq ($(OS),Windows_NT)
    ARCHFLAGS=-D WIN64
    LIB := lib_wind_obos.dll
    LDFLAGS=-shared
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        ARCHFLAGS=-D LINUX
	LIB := lib_wind_obos.so
	LDFLAGS=-shared -W1,-soname,$(LIB)
    endif
    ifeq ($(UNAME_S),Darwin)
        ARCHFLAGS=-D OSX
	LIB := lib_wind_obos.dylib
	LDFLAGS=-dynamiclib
    endif
endif

CPPFLAGS=$(CCFLAGS) $(ARCHFLAGS)

all: shared

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

shared : $(OBS)
	$(CC) $(LDFLAGS) -o $(LIB) $(OBS)

clean:
	/bin/rm -rf $(OBS) $(LIB) *~ *.pyc

.PHONY: clean
