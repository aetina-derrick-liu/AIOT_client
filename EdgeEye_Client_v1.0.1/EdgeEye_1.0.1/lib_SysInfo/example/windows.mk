Q           :=@
CC          :=gcc
SRCS        :=$(wildcard *.c)
TARGET      :=example.exe
OBJECTS     :=
CFLAGS      :=-I../bin/include/
LIBS        :=../bin/lib_SysInfo.dll
LDFLAGS     :=$(LIBS) -m32
UAC_RC		:=uac

.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)$(CC) $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS) $(UAC_RC).res
	$(Q)cp ../bin/lib_SysInfo.dll .

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET)
