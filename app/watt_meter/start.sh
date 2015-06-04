#!/bin/bash
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.
CWD=`dirname "${0}"`

PIN_NO_SWITCH=13
LCD_SCRIPT=${CWD}/../../driver/dev/sc2004cs/sc2004cs.test

echo ${PIN_NO_SWITCH} | sudo tee /sys/class/gpio/export > /dev/null 2>&1
echo in | sudo tee /sys/class/gpio/gpio${PIN_NO_SWITCH}/direction > /dev/null 2>&1
SWITCH=`cat /sys/class/gpio/gpio${PIN_NO_SWITCH}/value`
VERSION="1.0"
IP_ADDR="192.168.4.1"
SSID="rasp-meter"

if [ ${SWITCH} = "0" ]; then
    # ON
    echo "MODE: STAND ALONE"
    sudo /usr/sbin/hostapd /etc/hostapd/hostapd.conf &
    sleep 1
    sudo ip addr add 192.168.4.1/24 dev wlan0
    sudo /usr/sbin/dhcpd -cf /etc/dhcp/dhcpd.conf -f wlan0 &
    sudo ${LCD_SCRIPT} "Power Meter v ${VERSION}" "MODE: STAND ALONE" "SSID: ${SSID}" "IP: ${IP_ADDR}"
else
    # OFF
    echo "MODE: WIFI CLIENT"
    sudo wpa_supplicant -B -iwlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf -Dnl80211
    sleep 1
    sudo dhclient wlan0
    sudo ${LCD_SCRIPT} "Power Meter v ${VERSION}" "MODE: WIFI CLIENT" "" "IP: $(hostname -I | cut -d ' ' -f1)"
fi
sleep 5
sudo ${CWD}/watt_meter

# Local Variables:
# coding: shift_jis-unix
# mode: sh
# tab-width: 4
# indent-tabs-mode: nil
# End:
