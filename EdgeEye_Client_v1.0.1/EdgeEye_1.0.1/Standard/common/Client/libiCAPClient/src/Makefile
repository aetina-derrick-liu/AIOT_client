Q           :=@
OUTPATH     :=../bin

ifeq ($(OS),Windows_NT)
	TARGET     := libiCAPClient.dll
	MAKEFILE   := windows.mk
else
	TARGET     := libiCAPClient_32.a libiCAPClient_64.a
	MAKEFILE   := linux.mk
endif


all: clean
	$(Q)make -f $(MAKEFILE)

clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)$(RM) $(OUTPATH)/$(TARGET)
	$(Q)$(RM) -r $(OUTPATH)/include/$(TARGETHADER)
	$(Q)$(RM) -r $(OUTPATH)/include
	$(Q)$(RM) -r $(OUTPATH)
	$(Q)$(RM) $(TARGET)
	$(Q)$(RM) *.o