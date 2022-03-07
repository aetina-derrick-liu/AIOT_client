#!/bin/bash
YELLOW='\033[1;33m'
GREEN='\033[1;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}Start to install iCAP Client Service${NC}"

echo -e "${GREEN}  Create /var/iCAP_Client folder...${NC}";
sudo mkdir -p /var/iCAP_Client
sudo mkdir -p /var/iCAP_Client/wwwroot
sudo mkdir -p /var/iCAP_Client/wwwroot/dist

echo -e "${GREEN}  Copy iCAP_ClientService into /usr/sbin${NC}";
cp ./iCAP_ClientService_64 /usr/sbin/iCAP_ClientService

echo -e "${GREEN}  Copy require files...${NC}"
cp ./wwwroot/dist/* /var/iCAP_Client/wwwroot/dist

echo -e "${GREEN}  Run iCAP_ClientService to a service${NC}";
cp iCAP_Client.service /etc/systemd/system

echo -e "${YELLOW}  Enable iCAP_ClientService${NC}";
systemctl enable iCAP_Client.service
systemctl daemon-reload
systemctl start iCAP_Client.service
