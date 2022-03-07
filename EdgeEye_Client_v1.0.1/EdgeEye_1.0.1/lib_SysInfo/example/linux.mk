Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=example
OBJECTS     :=
CFLAGS      :=-I../include/
LIBS        :=../lib/lib_SysInfo_64.a
LDFLAGS     :=$(LIBS)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)