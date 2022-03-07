Q			:=@
TARGET_E	:= encrypt_example
TARGET_D	:= decrypt_example
LIB_PATH	:=../lib
OPENSSL_LIB	:=../Build-OpenSSL-MinGW-64/bin

ifeq ($(OS),Windows_NT)
	MAKEFILE	:= windows.mk
else
	MAKEFILE	:= linux.mk
endif

.PHONY: all
all: 
	$(Q)make -f $(MAKEFILE)