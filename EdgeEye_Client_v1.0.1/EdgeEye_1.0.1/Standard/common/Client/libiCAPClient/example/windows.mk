Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=example.exe
OBJECTS     :=
CFLAGS      :=-I../bin/include/
LIBS        :=../bin/libiCAPClient.dll
LDFLAGS     :=$(LIBS)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)
	$(Q)cp ../bin/*.dll .

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
