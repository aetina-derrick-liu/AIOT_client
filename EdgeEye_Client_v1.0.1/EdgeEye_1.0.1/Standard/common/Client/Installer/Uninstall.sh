#!/bin/bash
YELLOW='\033[1;33m'
GREEN='\033[1;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}Start to uninstall iCAP Client Service${NC}"
echo -e "${RED}It will remove the iCAP Client Service, sure?(y/n)${NC}"
read input
if [ "${input}" == "y" ]; then
	echo -e "${GREEN}  Remove iCAP_ClientService";
	systemctl stop iCAP_Client.service
	systemctl disable iCAP_Client.service
	rm /etc/systemd/system/iCAP_Client.service
	rm /usr/sbin/iCAP_ClientService
	systemctl daemon-reload
	systemctl reset-failed
	rm -r /var/iCAP_Client
fi