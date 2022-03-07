Q           :=@

ifeq ($(OS),Windows_NT)
	TARGET     := unit_test.exe
	MAKEFILE   := windows.mk
else
	UNAME_P :=$(shell uname -p)
	ifeq ($(UNAME_P),aarch64)
		TARGET		:=unit_test
		MAKEFILE	:=linux_arm.mk
	else ifeq ($(UNAME_P),x86_64)
		TARGET		:= unit_test
		MAKEFILE	:= linux.mk
	endif
endif

all: clean
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)$(RM) $(TARGET)
