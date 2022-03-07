Q               := @
MAKE			:= make

ifeq ($(OS), Windows_NT)
	MAKEFILE	:= windows.mk
	CFLAGS		:= -DOS_WIN
else ifeq ($(OS), QNX)
	MAKEFILE	:= qnxnto.mk
	CFLAGS		:= -DOS_QNX
else ifeq ($(shell uname), Linux)
	ifeq ($(OS), PPC)
		MAKEFILE	:= ppc.mk
	else ifeq ($(OS), ARM)
		MAKEFILE	:= arm.mk
	else
		MAKEFILE	:= linux.mk
	endif
	CFLAGS		:= -DOS_LINUX	
else ifeq ($(shell uname), SunOS)
	MAKEFILE	:= solaris.mk
	CFLAGS		:= -DOS_SOLARIS	
	MAKE		:= gmake
endif

export CFLAGS	

all:
	$(Q)$(MAKE) -f $(MAKEFILE)

libs:
	$(Q)$(MAKE) -f $(MAKEFILE) libs

clean:
	$(Q)$(MAKE) -f $(MAKEFILE) clean
	$(Q)rm -f .depend *~ *.bak *.res *.o32 *.o64 a.out*
