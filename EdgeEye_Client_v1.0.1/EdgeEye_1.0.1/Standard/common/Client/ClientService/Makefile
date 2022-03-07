Q           :=@
MACROS		:=DEBUG
ifeq ($(OS),Windows_NT)
	TARGET     := iCAP_ClientService.exe
	MAKEFILE   := ./windows.mk
else
	TARGET     := iCAP_ClientService_64
	TARGET     += iCAP_ClientService_32
	MAKEFILE   := ./linux.mk
endif


all:
	$(Q)cd wwwroot && npm run build
	$(Q)make -f $(MAKEFILE) MACROS=$(MACROS)

clean:
	$(Q)rm -f .depend *~ *.bak *.res *.o
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f $(TARGET)
