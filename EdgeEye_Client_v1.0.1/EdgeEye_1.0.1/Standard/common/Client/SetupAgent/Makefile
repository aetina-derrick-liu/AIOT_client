Q   := @

ifeq ($(OS),Windows_NT)
	TARGET     := iCAP.exe
	MAKEFILE   := Win32.mk
else
	TARGET     := iCAP
	MAKEFILE   := Linux.mk
endif


all:
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)rm -f .depend *~ *.bak *.o
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f $(TARGET)