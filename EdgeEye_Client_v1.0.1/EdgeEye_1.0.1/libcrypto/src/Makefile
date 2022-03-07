Q				:=@
OUTPATH_LIB		:=../lib
OUTPUT_INCLUDE	:=../include

ifeq ($(OS), Windows_NT)
	MAKEFILE	:= windows.mk
else
	MAKEFILE	:= linux.mk
endif

.PHONY: all
all: 
	$(Q)make -f $(MAKEFILE)

.PHONY: clean
clean:
	$(Q)$(RM) *.a *.o32 *.o64
	$(Q)$(RM) -r $(OUTPATH_LIB)
	$(Q)$(RM) -r $(OUTPUT_INCLUDE)