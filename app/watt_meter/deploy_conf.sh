#!/bin/bash
# Copyright (C) 2015 Tetsuya Kimata <kimata@green-rabbit.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, version 2.
CWD=`dirname "${0}"`

sudo ln -sf "${CWD}/conf/interfaces"	/etc/network/interfaces
sudo ln -sf "${CWD}/conf/hostapd.conf"	/etc/hostapd/hostapd.conf
sudo ln -sf "${CWD}/conf/dhcpd.conf"	/etc/dhcp/dhcpd.conf

# Local Variables:
# coding: shift_jis-unix
# mode: sh
# tab-width: 4
# indent-tabs-mode: nil
# End:
