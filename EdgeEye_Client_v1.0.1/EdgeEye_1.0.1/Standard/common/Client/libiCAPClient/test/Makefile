Q           :=@

ifeq ($(OS),Windows_NT)
	TARGET     := unit_test.exe
	MAKEFILE   := windows.mk
else
	TARGET     := unit_test
	MAKEFILE   := linux.mk
endif

all: clean
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)$(RM) $(TARGET)