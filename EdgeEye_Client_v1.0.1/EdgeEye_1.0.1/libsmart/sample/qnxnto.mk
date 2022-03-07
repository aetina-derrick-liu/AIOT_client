Q               := @
CC				:= $(CROSS_COMPILE)qcc	
SRCS            := $(wildcard *.c)
OBJS            := $(SRCS:.c=.o)
TARGET          := iSMART
LIBS			:= libsmart.a
CFLAGS          += -Wall -I ./include 
LDFLAGS         := $(LIBS) 
XINSTALL		:= ~/ide-7.0-workspace/x86-64-generic/prebuilt/x86_64/bin

.PHONY: all clean distclean	xinstall

all: $(TARGET) xinstall

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS)

xinstall:
	$(Q)echo "  Install $(TARGET) -> $(XINSTALL)"
	$(Q)cp -rf $(TARGET) $(XINSTALL)
	$(Q)cp -rf mo.txt $(XINSTALL)
	$(Q)cp -rf Flash.ini $(XINSTALL)

clean distclean: 
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f .depend *~ $(OBJS) $(TARGET) a.out*

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif
