Q	:=@
OUTPATH :=../../bin

FILES	:=Install.sh
FILES	+=iCAP_ClientServiceInit.sh

.PHONY: all

all:
	$(Q)echo "  Nothing needed to build in linux version"
#	$(Q)for i in $(FILES); do\
#		(cp $$i $(OUTPATH));\
#	done
