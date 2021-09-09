RUTOS_DIR="/home/magax/teltonika/Linux/GPL/openwrt-gpl-ipq40xx-generic.Linux-x86_64"
MENUCONFIG="false"

SCP="true"
SCP_DIR="/root"
IPK_DIR=$(echo "$RUTOS_DIR/bin/packages/arm"*"/base/")
USER="root"
IP="192.168.1.1"

#["<package name>"]="<build enabled>"
declare -A PACKAGES=(
    ["mqtt-subscriber"]="true"
    ["luci-app-mqtt-subscriber"]="true"
)
