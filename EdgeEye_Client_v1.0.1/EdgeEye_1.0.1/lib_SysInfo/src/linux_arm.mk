Q           	:=@
CC          	:=$(CROSS_COMPILE)gcc -Wno-unused-variable
SRCS        	:=$(wildcard *.c)
TARGETNAME  	:=lib_SysInfo
TARGET      	:=$(TARGETNAME)_64.a
TARGETHADER 	:=$(TARGETNAME).h
OBJECTS     	:=$(SRCS:.c=.o64)
CFLAGS      	:=-o0 -I include -g
CHECK       	:=-fsyntax-only -Wall
OUTPATH_LIB 	:=../bin
OUTPATH_HEADER	:=../include
LDFLAGS     	:=$(LIBS)
LIBS        	:=


.PHONY: all
all: clean check $(TARGET) UNITTEST

%.o64: %.c Makefile
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' , LIB name=$(TARGET) ...$(COLOR_W)"
	$(Q)ar rcs -o $@ $(OBJECTS) $(LDFLAGS)

UNITTEST:
	$(Q)cd ../test && $(MAKE)
	$(Q)mkdir -p $(OUTPATH_LIB)
	$(Q)mv $(TARGET) $(OUTPATH_LIB)
	$(Q)mkdir -p $(OUTPATH_HEADER)
	$(Q)cp include/$(TARGETHADER) $(OUTPATH_HEADER)
	$(Q)$(RM) $(OBJECTS)

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
