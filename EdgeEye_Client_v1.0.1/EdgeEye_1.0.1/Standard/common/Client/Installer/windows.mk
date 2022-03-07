Q			:= @
SRCS		:= iCAP_ClientServiceInstaller.nsi
OUTPATH		:= ../../bin

all:
	$(Q)echo "  Generating install shield ..."
	$(Q)makensis $(SRCS)

clean:
	$(Q)echo "  Cleaning '$(TARGET)' ..."
	$(Q)rm -f $(TARGET)
