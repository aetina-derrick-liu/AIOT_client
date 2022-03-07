Q           	:=@
CC          	:=$(CROSS_COMPILE)gcc
SRCS        	:=$(wildcard *.c)
TARGETNAME  	:=lib_SysInfo
TARGET      	:=$(TARGETNAME).dll
TARGETHADER 	:=$(TARGETNAME).h
OBJECTS     	:=$(SRCS:.c=.o)
CFLAGS      	:=-I./include
CFLAGS      	+=-I../dependencies/include
CHECK       	:=-fsyntax-only -Wall -O0
OUTPATH_LIB		:=../lib
OUTPATH_HEADER	:=../include
LDFLAGS     	:=$(LIBS)
LIBS        	:=../dependencies/lib/libcpu.dll
LIBS			+=../dependencies/lib/EAPI_Library.dll
LIBS        	+=-loleaut32 -lwbemuuid -lole32 -liphlpapi -m32

.PHONY: all
all: .depend $(TARGET) UNITTEST

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) -shared -o $@ $(OBJECTS) $(LIBS)
	$(Q)cp $(TARGET) ../test

UNITTEST: 
	$(Q)cd ../test && $(MAKE)
	$(Q)mkdir -p $(OUTPATH_LIB)
	$(Q)mv $(TARGET) $(OUTPATH_LIB)
	$(Q)mkdir -p $(OUTPATH_HEADER)
	$(Q)cp include/$(TARGETHADER) $(OUTPATH_HEADER)
	$(Q)$(RM) $(OBJECTS)

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS)  -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif