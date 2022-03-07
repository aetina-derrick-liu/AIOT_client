Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGETNAME  :=libiCAPClient
TARGET      :=$(TARGETNAME).dll
TARGETHADER :=$(TARGETNAME).h
OBJECTS     :=$(SRCS:.c=.o)
CFLAGS      :=-o0
CFLAGS		+=-I../../Dependencies/Windows -Iinclude
CHECK       :=-fsyntax-only -Wall
OUTPATH     :=../bin
LDFLAGS     := $(LIBS)
LIBS        :=-lws2_32
LIBS        +=../../Dependencies/Windows/json-c/libjson-c-2.dll

.PHONY: all
all: $(TARGET) UNITTEST

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) $(LIBS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) $(CFLAGS) -shared -o $@ $(SRCS) $(LIBS)
	$(Q)strip $@
	$(Q)cp $(TARGET) ../test

UNITTEST:
	$(Q)cd ../test && $(MAKE)
	$(Q)mkdir -p $(OUTPATH)
	$(Q)mv $(TARGET) $(OUTPATH)
	$(Q)mkdir -p $(OUTPATH)/include
	$(Q)cp include/$(TARGETHADER) $(OUTPATH)/include
	$(Q)cp ../../Dependencies/Windows/json-c/libjson-c-2.dll $(OUTPATH)
	$(Q)$(RM) $(OBJECTS)