Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=unit_test.exe
OBJECTS     :=
CFLAGS      :=-O0 -I../src/include/
LIBS        :=../src/libiCAPClient.dll
LIBS        +=../../Dependencies/Windows/json-c/libjson-c-2.dll
LDFLAGS     :=$(LIBS)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)
	$(Q)echo "  Start 64-bit unit tests ..."
	$(Q)./unit_test
	$(Q)echo "  End of 64-bit unit tests ..."
	$(Q)$(RM) $@

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
