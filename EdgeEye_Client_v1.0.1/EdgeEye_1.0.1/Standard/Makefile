include	./current.mk

Q								:=@
OUTPUT_PATH						:=bin
CLIENT_VERSION_MAJOR			:=1
CLIENT_VERSION_MINOR			:=0
CLIENT_VERSION_BUILD			:=1
RELEASE_CLIENT_TAR				:=EdgeEye_ClientService_linux_V_$(CLIENT_VERSION_MAJOR)_$(CLIENT_VERSION_MINOR)_$(CLIENT_VERSION_BUILD).tar.gz
RELEASE_CLIENT_EXE				:=iCAP_ClientSerivce_win32_V_$(CLIENT_VERSION_MAJOR)_$(CLIENT_VERSION_MINOR)_$(CLIENT_VERSION_BUILD).exe

PATH_CLIENTSERVICE				:=common/Client/ClientService
PATH_RECOVERY					:=common/Client/Recovery
PATH_CLIENTSERVICE_SETTING		:=common/Client/SetupAgent
PATH_CLIENTSERVICE_INSTALLER	:=common/Client/Installer
PATH_CLIENTSERVICE_DEPENDENCIES	:=common/Client/Installer/Dependencies.sh
PATH_CLIENTSERVICE_API			:=common/Client/libiCAPClient
PATH_CLIENTSERVICE_JETSONSTATS	:=common/Client/Installer/Jetson_status

CLIENTWEB_FILES					:=$(shell ls $(PATH_CLIENTSERVICE)/wwwroot/dist/)

.PHONY: all
ifeq ($(OS),Windows_NT)
all: init client
else
all: clean client
endif

.PHONY: client
client:
ifeq ($(OS),Windows_NT)
	$(Q)echo "  Building iCAP Client windows 32-bit version..."
	$(Q)echo "  Building iCAP Client Service..."
	$(Q)cd $(PATH_CLIENTSERVICE) && make MACROS=$(CLIENT_MACRO)
	$(Q)cd $(PATH_RECOVERY) && make MACROS=$(CLIENT_MACRO)
	$(Q)echo "  Building iCAP Client Service installer..."
	$(Q)cd $(PATH_CLIENTSERVICE_INSTALLER) && make
	
	$(Q)echo "  Copy iCAP Client Service require files..."
	$(Q)mv $(PATH_CLIENTSERVICE_INSTALLER)/iCAP_ClientService_win32* $(OUTPUT_PATH)/iCAP_ClientService/windows/bin/
	
	$(Q)cd $(PATH_CLIENTSERVICE) && make clean
	$(Q)cd $(PATH_RECOVERY) && make clean
	$(Q)cd $(PATH_CLIENTSERVICE_SETTING) && make clean
else
	$(Q)mkdir -p $(OUTPUT_PATH)/iCAP_ClientService/linux/bin
	$(Q)mkdir -p $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux
	$(Q)mkdir -p $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/wwwroot
	$(Q)mkdir -p $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/wwwroot/dist
	$(Q)mkdir -p $(OUTPUT_PATH)/iCAP_ClientService/linux/lib
	$(Q)echo "  Building iCAP Client linux 64-bit version..."
	$(Q)echo "  Building iCAP Client Service..."
	$(Q)cd $(PATH_CLIENTSERVICE) && make MACROS=$(CLIENT_MACRO)
	$(Q)echo "  Building iCAP Client Service installer..."
	$(Q)cd $(PATH_CLIENTSERVICE_INSTALLER) && make
#	$(Q)echo "  Building iCAP Client Service API..."
#	$(Q)cd $(PATH_CLIENTSERVICE_API)/src && make
	$(Q)echo "  Copy iCAP Client Service require files..."
	$(Q)cp -r $(PATH_CLIENTSERVICE_JETSONSTATS) $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)mv $(PATH_CLIENTSERVICE)/iCAP_ClientService_64 $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)cp ./ServiceSetting.json $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)cp $(PATH_CLIENTSERVICE_DEPENDENCIES) $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)chmod a+x $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/Dependencies.sh
	$(Q)cp $(PATH_CLIENTSERVICE_INSTALLER)/Install.sh $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)chmod a+x $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/Install.sh
	$(Q)cp $(PATH_CLIENTSERVICE_INSTALLER)/Uninstall.sh $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)chmod a+x $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/Uninstall.sh
	$(Q)cp $(PATH_CLIENTSERVICE_INSTALLER)/iCAP_Client.service $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)cp $(PATH_CLIENTSERVICE_INSTALLER)/eula.md $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/
	$(Q)for i in $(CLIENTWEB_FILES); do\
		(cp $(PATH_CLIENTSERVICE)/wwwroot/dist/$$i $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/iCAP_ClientService_linux/wwwroot/dist);\
	done
	$(Q)cd $(OUTPUT_PATH)/iCAP_ClientService/linux/bin/ && tar zcf $(RELEASE_CLIENT_TAR) iCAP_ClientService_linux/ --remove-files
#	$(Q)mv $(PATH_CLIENTSERVICE_API)/bin/* $(OUTPUT_PATH)/iCAP_ClientService/linux/lib/
endif

.PHONY: clean
clean:
	$(Q)echo "  Clean previous build files..."
	$(Q)$(RM) -r $(OUTPUT_PATH)
