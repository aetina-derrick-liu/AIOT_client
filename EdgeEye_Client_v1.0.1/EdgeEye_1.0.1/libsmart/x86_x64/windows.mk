Q               := @
CC				:= $(CROSS_COMPILE)gcc	
SRCS            := $(wildcard *.c)
OBJS            := $(SRCS:.c=.o)
TARGET          := iSMART.exe
LIBS			:= libsmart.dll
CFLAGS          += -Wall -I ./include 
LDFLAGS         := $(LIBS) 

LIB_PACKAGE		:= $(PRJ_LIB_DIR)/libsmart/

.PHONY: all clean distclean	xinstall

all: .depend libs $(TARGET)

%.o: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS)

libs:
	$(Q)echo "  Generating 'LIBs' ..."	
	@for i in $(LIB_PACKAGE); do\
		(cd $$i && make);\
	done

	$(Q)cp $(PRJ_LIB_DIR)/libsmart/libsmart.dll .
	$(Q)cp $(PRJ_LIB_DIR)/libsmart/include/libsmart.h ./include/.

clean distclean: 
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f .depend *~ $(OBJS) $(TARGET) a.out*

.depend dep depend:
	$(Q)echo "  Generating '$@' ..."
	$(Q)$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend, $(wildcard .depend))
    include .depend
endif
