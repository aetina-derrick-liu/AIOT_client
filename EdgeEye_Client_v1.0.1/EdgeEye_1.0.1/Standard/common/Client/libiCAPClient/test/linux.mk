Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=unit_test
OBJECTS     :=
CFLAGS      :=-O0 -I../src/include/
CFLAGS_32   :=-m32 -O0 -I../src/include/
LIBS        :=../src/libiCAPClient_64.a -ljson-c -lpthread
LIBS_32     :=../src/libiCAPClient_32.a -ljson-c -lpthread
LDFLAGS     :=$(LIBS)
LDFLAGS_32  :=$(LIBS_32)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)
	$(Q)echo "  Start 64-bit unit tests ..."
	$(Q)./unit_test
	$(Q)echo "  End of 64-bit unit tests ..."
	$(Q)$(RM) $@
#	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS_32) $(LDFLAGS_32)
#	$(Q)echo "  Start 32-bit unit tests ..."
#	$(Q)./unit_test
#	$(Q)echo "  End of 32-bit unit tests ..."
#	$(Q)$(RM) $@

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
