Q               := @
CC              := gcc -std=gnu99
SRCS            := $(wildcard *.c)
OBJS            := $(SRCS:.c=.o)
TARGET          := iCAP

GTKCFG          := pkg-config
CFLAGS          := `$(GTKCFG) --cflags gtk+-3.0`
LDFLAGS         := `$(GTKCFG) --libs gtk+-3.0`

LIBS            :=
CFLAGS          += -Wall -I ./include
LDFLAGS         += $(LIBS) 

.PHONY: all

all: .depend $(TARGET)

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(Q)echo -e "  $(COLOR_G)Building '$@' VER=$(AP_VER)... $(COLOR_W)"
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS)

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif