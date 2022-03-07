Q           :=@
MACROS		:=DEBUG

ifeq ($(OS),Windows_NT)
	TARGET     := iCAP_DIOCtl.exe
	MAKEFILE   := ./windows.mk
else
	TARGET     := iCAP_DIOCtl
	MAKEFILE   := ./linux.mk
endif

all: clean
	$(Q)make -f $(MAKEFILE) MACROS=$(MACROS)

clean:
	$(Q)$(RM) $(TARGET)