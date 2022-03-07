Q           	:=@
CC				:=g++ -std=c++11
SRCS			:=$(wildcard *.cpp)
TARGETNAME		:=libcrypto
TARGET			:=$(TARGETNAME)_64.a
# TARGET_32		:=$(TARGETNAME)_32.a
OBJECTS			:=$(SRCS:.cpp=.o64)
OBJECTS_32		:=$(SRCS:.cpp=.o32)
INCLUDE_PATH	:=include
MACROS			:=-DNEWER_OPENSSLLIB
CPPFLAGS		:=-Wall $(MACROS)
CPPFLAGS		+=-I$(INCLUDE_PATH)
CPPFLAGS		+=-I/usr/include
CPPFLAGS_32		:=-m32 $(CPPFLAGS)
LDFLAGS			:=
LIBS			:=
OUTPATH_LIB		:=../lib
OUTPUT_INCLUDE	:=../include

.PHONY: all
all: clean $(TARGET) $(TARGET_32) install

%.o64: %.cpp
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CPPFLAGS) -o $@ -c $<

%.o32: %.cpp
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CPPFLAGS_32) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' ..."
	$(Q)ar rcs -o $@ $(OBJECTS)

$(TARGET_32): $(OBJECTS_32)
	$(Q)echo "  Building '$@' ..."
	$(Q)ar rcs -o $@ $(OBJECTS)

.PHONY: install
install:
	$(Q)mkdir -p $(OUTPATH_LIB)
	$(Q)mkdir -p $(OUTPUT_INCLUDE)
	$(Q)cp $(TARGET) $(TARGET_32) $(OUTPATH_LIB) 
	$(Q)cp $(INCLUDE_PATH)/Crypto.hpp $(OUTPUT_INCLUDE)

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET) $(TARGET_32) $(OBJECTS) $(OBJECTS_32)
	$(Q)$(RM) -r $(OUTPATH_LIB)
	$(Q)$(RM) -r $(OUTPUT_INCLUDE)