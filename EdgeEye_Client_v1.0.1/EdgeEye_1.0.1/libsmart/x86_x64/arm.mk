Q               := @
CC				:= $(CROSS_COMPILE)gcc
SRCS            := $(wildcard *.c)
TARGET			:= iSMART
OBJS			:= $(SRCS:.c=.o)
LIBS			:= libsmart.a -lm
CFLAGS			:= $(CFLAGS) -Wall -I ./include 
LDFLAGS			:= $(LIBS) 

.PHONY: all clean libs

all: .depend $(TARGET)

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean distclean: 
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f .depend *~ $(OBJS) $(TARGET) a.out*

libs:
	$(Q)echo "  Generating 'LIBs' ..."	
	@for i in $(LIB_PACKAGE); do\
		(cd $$i && make clean all OS=PPC);\
	done

	$(Q)cp ../../../library/libsmart/*.a .

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif

