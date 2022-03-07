Q           	:=@
CC          	:=gcc -Wno-unused-variable
SRCS        	:=$(wildcard *.c)
TARGETNAME  	:=lib_SysInfo
TARGET      	:=$(TARGETNAME)_64.a
TARGET_32   	:=$(TARGETNAME)_32.a
OBJECTS     	:=$(SRCS:.c=.o64)
OBJECTS_32  	:=$(SRCS:.c=.o32)
CFLAGS      	:=-o0 -I include -g
CFLAGS_32   	:=-m32 -o0 -I include -g
CHECK       	:=-fsyntax-only -Wall
OUTPATH_LIB  	:=../lib
OUTPATH_HEADER	:=../include
LDFLAGS     	:= $(LIBS)
LIBS        	:=

.PHONY: all
all: clean check $(TARGET) $(TARGET_32) UNITTEST

%.o64: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

%.o32: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS_32) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' ..."
	$(Q)ar rcs -o $@ $(OBJECTS) $(LDFLAGS)

$(TARGET_32): $(OBJECTS_32)
	$(Q)echo "  Building '$@' ..."
	$(Q)ar rcs -o $@ $(OBJECTS_32) $(LDFLAGS)

UNITTEST:
	$(Q)cd ../test && $(MAKE)
	$(Q)mkdir -p $(OUTPATH_LIB)
	$(Q)mv $(TARGET) $(OUTPATH_LIB)
	$(Q)mv $(TARGET_32) $(OUTPATH_LIB)
	$(Q)mkdir -p $(OUTPATH_HEADER)
	$(Q)cp include/lib_SysInfo.h $(OUTPATH_HEADER)
	$(Q)cp include/CMDParser.h $(OUTPATH_HEADER)
	$(Q)cp include/OSInfo.h $(OUTPATH_HEADER)
	$(Q)$(RM) $(OBJECTS)
	$(Q)$(RM) $(OBJECTS_32)

.depend:
	$(Q)echo "  Generating '$@' ..."
	$(CC) $(CFLAGS) -M *.c > $@

.PHONY: check
check:
	$(Q)echo "  Check syntax"
	$(Q)$(CC) $(SRCS) $(CHECK) $(CFLAGS)

.PHONY: clean
clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)$(RM) -r $(OUTPATH_LIB)
	$(Q)$(RM) -r $(OUTPATH_HEADER)
	$(Q)$(RM) $(TARGET)