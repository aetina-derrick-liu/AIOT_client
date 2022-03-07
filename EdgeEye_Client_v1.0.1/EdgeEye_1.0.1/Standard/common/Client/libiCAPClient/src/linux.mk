Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGETNAME  :=libiCAPClient
TARGET      :=$(TARGETNAME)_64.a
TARGET_32   :=$(TARGETNAME)_32.a
TARGETHADER :=$(TARGETNAME).h
OBJECTS     :=$(SRCS:.c=.o64)
OBJECTS_32  :=$(SRCS:.c=.o32)
CFLAGS      :=-o0 -I include -static
CFLAGS_32   :=-m32 -o0 -I include -static
CHECK       :=-fsyntax-only -Wall
OUTPATH     :=../bin
LDFLAGS     := $(LIBS)
LIBS        :=-lpthread -ljson-c

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
	$(Q)mkdir -p $(OUTPATH)
	$(Q)mv $(TARGET) $(OUTPATH)
	$(Q)mv $(TARGET_32) $(OUTPATH)
	$(Q)mkdir -p $(OUTPATH)/include
	$(Q)cp include/$(TARGETHADER) $(OUTPATH)/include
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
	$(Q)$(RM) $(OUTPATH)/$(TARGET)
	$(Q)$(RM) $(OUTPATH)/include/$(TARGETHADER)
	$(Q)$(RM) -r $(OUTPATH)/include
	$(Q)$(RM) -r $(OUTPATH)
	$(Q)$(RM) $(TARGET)
