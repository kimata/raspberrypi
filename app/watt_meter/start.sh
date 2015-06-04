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
    sudo ip addr add 192.168.4.1/24 dev wlan0 > /dev/null 2>&1
    sleep 1
    sudo /usr/sbin/hostapd /etc/hostapd/hostapd.conf > /dev/null 2>&1 &
    sudo /usr/sbin/dhcpd -cf /etc/dhcp/dhcpd.conf -f > /dev/null 2>&1 &
    sudo ${LCD_SCRIPT} "Power Meter v ${VERSION}" "MODE: STAND ALONE" "SSID: ${SSID}" "http://${IP_ADDR}/"
    sleep 5
else
    # OFF
    sudo ifup wlan0
    sudo ${LCD_SCRIPT} "Power Meter v ${VERSION}" "MODE: WIFI CLIENT" "" "http://$(hostname -i)/"
fi

sudo ${CWD}/watt_meter

# Local Variables:
# coding: shift_jis-unix
# mode: sh
# tab-width: 4
# indent-tabs-mode: nil
# End:
