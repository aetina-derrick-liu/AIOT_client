Q					:=@	
CC 					:=g++
SRCS_E    			:=encrypt_example.cpp
SRCS_D				:=decrypt_example.cpp
INCLUDE				:=include
LIB_PATH			:=../lib
LIB_INCLUDE			:=../include
OPENSSL_INCLUDE		:=../Build-OpenSSL-MinGW-64/$(INCLUDE)
OPENSSL_LIB			:=../Build-OpenSSL-MinGW-64/lib
CPPFLAGS 			:=-Wall -ggdb -I$(LIB_INCLUDE) -I$(OPENSSL_INCLUDE)
LIBS 				:=-lssl -lcrypto libcrypto.dll
LDFLAGS				:=-L$(OPENSSL_LIB) $(LIBS)
EXAMPLE_TARGET_E	:=encrypt_example 
EXAMPLE_TARGET_D	:=decrypt_example

ifeq ($(DEBUG), y)
	CFLAGS := $(CPPFLAGS) -DDEBUG
endif

.PHONY: all

all: clean $(EXAMPLE_TARGET_E) $(EXAMPLE_TARGET_D)

$(EXAMPLE_TARGET_E):
	$(Q)$(CC) $(SRCS_E) -o $@ $(CPPFLAGS) $(LDFLAGS)

$(EXAMPLE_TARGET_D):
	$(Q)$(CC) $(SRCS_D) -o $@ $(CPPFLAGS) $(LDFLAGS)

clean:
	$(Q)$(RM) -f $(EXAMPLE_TARGET_E) $(EXAMPLE_TARGET_D)