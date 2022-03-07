Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=example.exe
OBJECTS     :=
CFLAGS      :=-Iinclude/
LIBS        :=libiCAPClient.dll
LDFLAGS     :=$(LIBS)


.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
