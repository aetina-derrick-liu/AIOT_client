Q				:=@
CC				:=g++
SRCS			:=$(wildcard *.cpp)
TARGETNAME		:=libcrypto
TARGET			:=$(TARGETNAME).dll
OBJECTS			:=$(SRCS:.cpp=.o)
INCLUDE			:=include
OPENSSL_INCLUDE	:=../Build-OpenSSL-MinGW-64/$(INCLUDE)
OPENSSL_LIB		:=../Build-OpenSSL-MinGW-64/lib
CPPFLAGS		:=-g -w -Wall -Wextra -I$(INCLUDE) -I$(OPENSSL_INCLUDE)
LIBS			:=-llibcrypto -llibssl -lws2_32
LDFLAGS			:=-L$(OPENSSL_LIB) $(LIBS)
OUTPATH_LIB		:=../lib
OUTPUT_INCLUDE	:=../include

.PHONY: all 
all: clean $(TARGET) install

%.o: %.cpp
	$(Q)echo "  Compiling '$<' ..."
	$(Q)$(CC) $(CPPFLAGS) -o $@ -c $< 

$(TARGET): $(OBJECTS)
	$(Q)echo "  Building '$@' ..."
	$(Q)$(CC) -shared -o $@ $(OBJECTS) $(LDFLAGS)

.PHONY: install
install:
	$(Q)mkdir -p $(OUTPATH_LIB)
	$(Q)mkdir -p $(OUTPUT_INCLUDE)
	$(Q)-cp $(TARGET) $(OUTPATH_LIB) 
	$(Q)-cp $(INCLUDE)/* $(OUTPUT_INCLUDE)

.PHONY: clean
clean:
	$(Q)$(RM) $(TARGET) $(OBJECTS) 
	$(Q)$(RM) -r $(OUTPATH_LIB)
	$(Q)$(RM) -r $(OUTPUT_INCLUDE)