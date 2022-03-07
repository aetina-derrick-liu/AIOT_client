Q           :=@
CC          :=g++ -static -w -Wall
SRCS        :=$(wildcard  *.cpp)
DEBUGSRCS   :=../lib_SysInfo/src/*.cpp
TARGET      :=iCAP_ClientService.exe
OBJECTS     :=
OPENSSL_LIB	:=../../../../../../../../trunk/library/libcrypto/Build-OpenSSL-MinGW-64/lib
CFLAGS      :=-Iinclude/ -I/usr/local/include
CFLAGS		+=-I../../../../../../../../trunk/library/lib_SysInfo/bin/include
CFLAGS		+=-I../../../../../../../../trunk/library/libsmart/include
CFLAGS		+=-I../../../../../../../../trunk/library/libcrypto/bin/include
CFLAGS		+=-I../../../../../../../../trunk/library/libcrypto/Build-OpenSSL-MinGW-64/include
CFLAGS		+=-I../../../../../../../../../Inno/Trunk/EP/iRAID/library/ExSS/library/include
CFLAGS		+=-I../../../../../../../../../Inno/Trunk/EP/iRAID/library/ExPS/library/include
CFLAGS		+=-I../Dependencies/Windows/
CFLAGS      +=-I../../../../../../../../trunk/library/eclipse-paho-mqtt-c-windows-1.1.0/include/
CFLAGS 		+=-I../Dependencies/Windows/curl-7.44.0/include
CFLAGS      +=-I../Dependencies/Windows/sqlite3/
LIBS        :=-lm -lpthread -loleaut32 -lwbemuuid -lole32 -lws2_32 -static -lstdc++ -mwindows -mconsole -lgdiplus -lssl -lcrypto
LIBS        +=../../../../../../../../trunk/library/lib_SysInfo/bin/lib_SysInfo.dll
LIBS		+=../../../../../../../../trunk/library/libcrypto/bin/libcrypto.dll
LIBS    	+=../Dependencies/Windows/json-c/libjson-c-2.dll
LIBS   		+=../../../../../../../../trunk/library/libsmart/libsmart.dll
LIBS   		+=../../../../../../../../../Inno/Trunk/EP/iRAID/dll/lib_ExSS.dll
LIBS   		+=../../../../../../../../../Inno/Trunk/EP/iRAID/dll/lib_ExPS.dll
LIBS     	+=../../../../../../../../trunk/library/eclipse-paho-mqtt-c-windows-1.1.0/lib/paho-mqtt3a.dll
LIBS 		+=../Dependencies/Windows/curl-7.44.0/lib/libcurl.dll
LIBS 		+=../Dependencies/Windows/sqlite3/sqlite3.dll
LDFLAGS     :=-L$(OPENSSL_LIB) $(LIBS)
DEBUG       :=-g
UAC_RC		:= uac
OUTPATH		:= ../../bin
MACROS		:= 

ifeq ($(MACROS), ICHAMBER)
CHCC		:=gcc
CHFLAGS		:=-I../ChamberCtl/include
CHFLAGS		+=-I../../../../../../../../../Inno/Chamber/Sample_code/include
CHLDFLAGS 	+=../ChamberCtl/bin/lib_chamberctl.dll
CHLDFLAGS	+=-mwindows -lgdiplus -mconsole -lws2_32
endif

.PHONY: all
all: clean $(TARGET)

$(TARGET):
	$(Q)echo "  Building '$@' ..."
	$(Q)windres --input-format=rc -O coff -i $(UAC_RC).rc -o $(UAC_RC).res
ifeq ($(MACROS), ICHAMBER)
	$(Q)$(CHCC) -c $(CHSRCS) -o icham.o $(CFLAGS) $(CHFLAGS) -D $(MACROS)
	$(Q)$(CC) -g $(SRCS) -o $@ icham.o $(CFLAGS) $(LDFLAGS) $(CHFLAGS) $(CHLDFLAGS) $(UAC_RC).res -D $(MACROS)
else
	$(Q)$(CC) -g $(SRCS) -o $@ $(CFLAGS) $(LDFLAGS) $(UAC_RC).res -D $(MACROS)
endif
	$(Q)rm -f .depend *~ *.bak *.o *.res
	$(Q)strip $@

.PHONY: clean
clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)$(RM) $(TARGET)
