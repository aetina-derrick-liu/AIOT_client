Q					:=@
CC 					:=g++ -std=c++11
SRCS_E    			:=encrypt_example.cpp
SRCS_D				:=decrypt_example.cpp
LIB_PATH			:=../lib
LIB_INCLUDE			:=../include
CPPFLAGS 			:=-Wall -I$(LIB_INCLUDE)
LIBS				:=-lcrypto_64 -lcrypto
LDFLAGS				:=-L$(LIB_PATH) $(LIBS)
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