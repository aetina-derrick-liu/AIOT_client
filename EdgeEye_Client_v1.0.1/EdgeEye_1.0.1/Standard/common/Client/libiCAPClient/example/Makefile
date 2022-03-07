Q           :=@

ifeq ($(OS),Windows_NT)
	TARGET     := example.exe
	MAKEFILE   := windows.mk
else
	TARGET     := example
	MAKEFILE   := linux.mk
endif

all: clean
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)$(RM) $(TARGET)