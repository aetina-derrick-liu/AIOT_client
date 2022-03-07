Q	:=@

ifeq ($(OS),Windows_NT)
	TARGET     := iCAP_ClientService_Installer.exe
	MAKEFILE   := windows.mk
else
	MAKEFILE   := linux.mk
endif


all:
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)rm -f .depend *~ *.bak *.res *.o
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f $(TARGET)
