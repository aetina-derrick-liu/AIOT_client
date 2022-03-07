Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=example
OBJECTS     :=
CFLAGS      :=-I../bin/include/
LIBS        :=../bin/libiCAPClient_64.a -ljson-c -lpthread
LDFLAGS     :=$(LIBS)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
