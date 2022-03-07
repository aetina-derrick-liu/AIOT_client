Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=unit_test.exe
OBJECTS     :=
CFLAGS      :=-O0 -I../src/include/
LIBS        :=../src/lib_SysInfo.dll
LDFLAGS     :=$(LIBS) -m32

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