Q               := @
CC              := gcc
SRCS            := $(wildcard *.c)
OBJS            := $(SRCS:.c=.o)
TARGET          := iCAP_ClientService_Setting.exe
RC              := icon

GTKCFG          := pkg-config
CFLAGS          += `$(GTKCFG) --cflags gtk+-3.0`
LDFLAGS         += `$(GTKCFG) --libs gtk+-3.0`

LDFLAGS         += -mwindows
LDFLAGS			+= -lm
CFLAGS          += -Wall -Iinclude -I/usr/local/include
LDFLAGS         += $(LIBS)

CFLAGS 			+=-I../Dependencies/Windows/
CFLAGS      	+=-I../Dependencies/Windows/sqlite3/

LIBS    		:=../Dependencies/Windows/json-c/libjson-c-2.dll
LIBS 			+=../Dependencies/Windows/sqlite3/sqlite3.dll

UAC_RC			:= uac
OUTPATH			:= ../../bin

.PHONY: all

all: uac $(TARGET)

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET):
	$(Q)echo -e "  $(COLOR_G)Building '$@' VER=$(AP_VER)... $(COLOR_W)"
	$(Q)$(CC) -o $@ $(SRCS) $(RC).res $(LDFLAGS) $(CFLAGS) $(UAC_RC).res
	$(Q)rm -f .depend *~ *.bak *.o

ico:
	$(Q)echo "  Generating '$(RC).res' ..."
	$(Q)windres $(RC).rc -O coff -o $(RC).res

uac:
	$(Q)echo "  Generating '$(UAC_RC).res' ..."
	$(Q)windres --input-format=rc -O coff -i $(UAC_RC).rc -o $(UAC_RC).res

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif