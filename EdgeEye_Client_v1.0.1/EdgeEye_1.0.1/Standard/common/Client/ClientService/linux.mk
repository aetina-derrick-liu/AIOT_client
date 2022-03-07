Q           :=@
CC          :=g++ -std=c++11 -w 
SRCS        :=$(wildcard *.cpp)
TARGET      :=iCAP_ClientService_64
TARGET_32   :=iCAP_ClientService_32
OBJECTS     :=
CFLAGS	    :=-lm -D_GNU_SOURCE -fPIC -g
CFLAGS	    :=-I/usr/local/include
CFLAGS     	:=-I/usr/local/lib
CFLAGS     	+=-Iinclude
CFLAGS     	+=-I../../../../lib_SysInfo/bin/include
CFLAGS     	+=-I../../../../lib_SysInfo/src/include
#CFLAGS     	+=-I../../../../libsmart/x86_x64/include
CFLAGS     	+=-I../../../../libsmart/sample/include
CFLAGS	    +=-I../../../../libcrypto/include
# CFLAGS		+=-I../Dependencies/linux/libpaho-mqtt/include
CFLAGS		+=-I../Dependencies/arm/libpaho-mqtt/include
LIBS	    :=../../../../libcrypto/lib/libcrypto_64.a
LIBS        +=../../../../lib_SysInfo/bin/lib_SysInfo_64.a
# LIBS        +=../Dependencies/linux/libpaho-mqtt/lib/libpaho-mqtt3a.a
LIBS        +=../Dependencies/arm/libpaho-mqtt/lib/libpaho-mqtt3a.a
#LIBS        +=../../../../libsmart/x86_x64/libsmart_64.a
LIBS        +=../../../../libsmart/sample/libsmart.a
LIBS        +=-lcrypto -ljson-c -lpthread -lcurl -lsqlite3 -lstdc++
LIBS_32     :=-ljson-c -lpthread -lcurl -lsqlite3
LIBS_32     +=../../../../../lib_SysInfo/bin/lib_SysInfo_32.a
LIBS_32     +=../../../../../libsmart/x86_x64/libsmart_32.a
MACROS	    :=

.PHONY: all
all: clean $(TARGET) #$(TARGET_32) UNITTEST

$(TARGET):
	$(Q)echo "  Building 64-bit '$@' ..."
	$(Q)$(CC) $(SRCS) -o $@ $(LDFLAGS) $(LIBS) $(CFLAGS) -D$(MACROS)

$(TARGET_32):
	$(Q)echo "  Building 32-bit '$@' ..."
	$(Q)$(CC) -m32 $(SRCS) -o $@ $(LDFLAGS) $(LIBS_32) $(CFLAGS) -D$(MACROS)

UNITTEST:
	$(Q)cd unittest/ && $(MAKE)
	$(Q)$(RM) $(OBJECTS)
	$(Q)$(RM) $(OBJECTS_32)

.PHONY: clean
clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)$(RM) $(TARGET)
