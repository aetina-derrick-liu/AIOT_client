#!/bin/bash
YELLOW='\033[1;33m'
GREEN='\033[1;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}Installing dependence packages...${NC}"
apt-get update
apt-get install -y libjson-c-dev libssl-dev libcurl4-openssl-dev gawk lm-sensors dmidecode
ldconfig

#install jetson_stats tool
sudo apt-get install python3-pip
sudo pip3 install --upgrade pip jetson_stats
echo -e "${YELLOW}Install Jetson_status Tool${NC}"
sudo cp -rpf Jetson_status/ /opt/
cd /opt/Jetson_status
sudo chmod 777 /opt/Jetson_status/Jetson_status
