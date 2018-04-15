CC=g++
CCFLAGS=-g -std=c++11 -fPIC

OLD_OBS  = lib_wind_obos_orig.o 
NEW_OBS  = lib_wind_obos.o lib_wind_obos_cable_vessel.o lib_wind_obos_defaults.o
TEST_OBS = test_wind_obos.o test_wind_obos_orig.o test_both.o

ifeq ($(OS),Windows_NT)
    ARCHFLAGS=-D WIN64
    LIB := lib_wind_obos.dll
    LDFLAGS=-shared
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        ARCHFLAGS=-D LINUX
	LIB := lib_wind_obos.so
	LDFLAGS=-shared -Wl,-soname,$(LIB)
    endif
    ifeq ($(UNAME_S),Darwin)
        ARCHFLAGS=-D OSX
	LIB := lib_wind_obos.so
	LDFLAGS=-dynamiclib
    endif
endif

CPPFLAGS=$(CCFLAGS) $(ARCHFLAGS)

all: shared

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

shared : $(NEW_OBS)
	$(CC) $(LDFLAGS) -o $(LIB) $(NEW_OBS)

test: $(NEW_OBS) $(OLD_OBS) $(TEST_OBS) 
	$(CC) $(CPPFLAGS) -o testBoth.exe $(NEW_OBS) $(OLD_OBS) test_both.o
	$(CC) $(CPPFLAGS) -o testNew.exe $(NEW_OBS) test_wind_obos.o
	$(CC) $(CPPFLAGS) -o testOrig.exe $(OLD_OBS) test_wind_obos_orig.o

clean:
	/bin/rm -rf $(NEW_OBS) $(OLD_OBS) $(TEST_OBS) *.exe $(LIB) *~ *.pyc *.dSYM

.PHONY: clean
