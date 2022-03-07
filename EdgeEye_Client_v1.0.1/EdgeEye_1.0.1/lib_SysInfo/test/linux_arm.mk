Q           :=@
CC          :=$(CROSS_COMPILE)gcc
SRCS        :=$(wildcard *.c)
TARGET      :=unit_test
OBJECTS     :=
CFLAGS      :=-O0 -I../src/include/ -g
LIBS        :=../src/lib_SysInfo_64.a
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
