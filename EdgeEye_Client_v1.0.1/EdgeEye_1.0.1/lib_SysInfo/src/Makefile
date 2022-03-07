Q           	:=@
OUTPATH_LIB		:=../lib
OUTPATH_HEADER	:=../include

ifeq ($(OS),Windows_NT)
	TARGET     := lib_SysInfo.dll
	MAKEFILE   := windows.mk
	
	# Set the path of the dependencies
	LIBCPU_PATH         := "../../libcpu"
	LIBEAPI_PATH        := "../../EAPI"
	DEPENDENCIES_LIB    := "../dependencies/lib"
	DEPENDENCIES_HEADER := "../dependencies/include"
else
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),aarch64)
		TARGET     := lib_SysInfo_64.a
		MAKEFILE   := linux_arm.mk
	else ifeq ($(UNAME_P),x86_64)
		TARGET     := lib_SysInfo_32.a lib_SysInfo_64.a
		MAKEFILE   := linux.mk
	endif
endif


.PHONY: all
all: clean
	$(Q)make -f $(MAKEFILE)

.PHONY: clean
clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)$(RM) -r $(OUTPATH_LIB)
	$(Q)$(RM) -r $(OUTPATH_HEADER)
	$(Q)$(RM) $(TARGET)
	$(Q)$(RM) *.o

.PHONY: dep
dep:
ifeq ($(OS),Windows_NT)
	$(Q)echo "  Copy libcpu ..."
	$(Q)cp $(LIBCPU_PATH)/cpuz_bin/cpuidsdk.dll $(DEPENDENCIES_LIB)
	$(Q)cp $(LIBCPU_PATH)/lib/libcpu.dll $(DEPENDENCIES_LIB)
	$(Q)cp $(LIBCPU_PATH)/lib/include/libcpu.h $(DEPENDENCIES_HEADER)
	$(Q)echo "  Copy EAPI ..."
	$(Q)cp $(LIBEAPI_PATH)/lib/windows/x86/shared/EAPI_Library.dll $(DEPENDENCIES_LIB)
	$(Q)cp $(LIBEAPI_PATH)/lib/EApi.h $(DEPENDENCIES_HEADER)
endif