# -*- coding: utf-8 -*-
################################################################################
# Raspberry Pi セットアップ用 fabric レシピ
#
# ◇準備
#   * Fabric と Cuisine のインストール
#     Ubuntu の場合，下記のコマンドを実行して，Fabric と Cuisine をインストール
#     します．
#
#     % sudo apt-get install fabric
#     % sudo apt-get install python-pip
#     % sudo pip install cuisine
#
#   * 無線 LAN の設定ファイルの作成 (必要な場合)
#     WiFi を使用する場合，接続先のアクセスポイントに関して，
#     「wpa_passphrase "SSID" "パスフレーズ"」を実行して得られた SSID および PSK 
#     の値を，wifi_config.py というファイルを作成して書き込んでおきます．
#
#     WIFI_SSID='SSIDの値'
#     WIFI_PSK='PSK の値'
#
# ◇使い方
#   * 通常セットアップ
#
#     % fab -a -H pi@raspberrypi
#
#     以下の条件が共に成立する場合，無線 LAN の設定も行われ，「ホスト名-wifi」で
#     アクセスできるようになります．
#     - 無線 LAN アダプタが接続されており，wlan0 が存在
#     - 上の方に記載した，wifi_config.py ファイルが存在
#
#   * ホスト名の変更を含むセットアップ
#     環境変数 RASP_HOSTNAME にホスト名を設定して実行すると，ホスト名の変更も行
#     われます．
#
#     % env RASP_HOSTNAME=新しいホスト名 fab -a -H pi@raspberrypi setup
#
# ◇備考
#    この Fabric レシピは冪等性がありますので，同じホストに対して複数回実行して
#    も問題ありません．
#
################################################################################

import imp
import os
import paramiko

from fabric.api import run, sudo, settings
from fabric.decorators import task
from fabric.context_managers import hide
from fabric.utils import puts

import fabric.colors
import cuisine

@task
def setup():
    with settings(
        # hide('warnings', 'stdout', 'stderr')
    ):
        # raspi-config の自動起動を無効化
        if cuisine.file_exists('/etc/profile.d/raspi-config.sh'):
            # NOTE: sudo() だと，raspi-config が起動してしまうので run()
            run('sudo rm -f /etc/profile.d/raspi-config.sh')

        if os.environ.has_key('RASP_HOSTNAME'):
            setup_hostname(os.environ['RASP_HOSTNAME'])

        setup_locale()
        setup_ssh()
        setup_package()
        setup_i2c()
        setup_sysctl()
        setup_wlan()

        # rootfs サイズを拡張
        sudo('raspi-config --expand-rootfs')

        # 再起動
        sudo('reboot')


# タイムゾーンとロケールの設定        
def setup_locale():
    puts(fabric.colors.green('[Setting Locale]', True))
    sudo('echo "Asia/Tokyo" > /etc/timezone')
    sudo('dpkg-reconfigure -f noninteractive tzdata')
    sudo('sed -i -e \'s/^en_GB.UTF-8 UTF-8/# en_GB.UTF-8 UTF-8/\' /etc/locale.gen')
    sudo('sed -i -e \'s/^# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/\' /etc/locale.gen')
    sudo('sed -i -e \'s/^# ja_JP.UTF-8 UTF-8/ja_JP.UTF-8 UTF-8/\' /etc/locale.gen')
    sudo('dpkg-reconfigure --frontend=noninteractive locales')

# タイムゾーンとロケールの設定        
def setup_hostname(hostname):
    puts(fabric.colors.green('[Setting Hostname]', True))
    sudo('sed -i -e \'s/^.*/%s/\' /etc/hostname' % (hostname))
    sudo('sed -i -e \'s/^127.0.1.1\t.*/127.0.1.1\t%s/\' /etc/hosts' % (hostname))

# 公開鍵のコピー
def setup_ssh():
    puts(fabric.colors.green('[Install SSH Publickey]', True))

    with cuisine.mode_sudo():
        cuisine.ssh_authorize("pi", cuisine.file_local_read('~/.ssh/id_rsa.pub'))

# パッケージのインストール
def setup_package():
    puts(fabric.colors.green('[Install Packages]', True))

    with cuisine.mode_sudo():
        cuisine.package_update()
        cuisine.package_upgrade()
        run('rpi-update')
        cuisine.package_ensure([
                'gcc',
                'make',
                'automake',
                'emacs23-nox',
                'tmux',
                'ruby',
                'git',
                'zsh',
                'i2c-tools',
                'lsb-release',
                'mlocate',
                ])

# I2C の有効化
def setup_i2c():
    if cuisine.file_exists('/etc/modprobe.d/raspi-blacklist.conf'):
        sudo('sed -i -e \'s/^blacklist i2c-bcm2708/# blacklist i2c-bcm2708/g\' ' +
             '/etc/modprobe.d/raspi-blacklist.conf')
    else:
        # RASPBIAN Release 2015-01-31 or later
        boot_conf = cuisine.text_ensure_line(
            cuisine.file_read('/boot/config.txt'),
            'dtparam=i2c_arm=on'
            )
        cuisine.file_write(
            location = '/boot/config.txt',
            content  = boot_conf,
            mode='755',
            sudo=True
            )
        modules_conf = cuisine.text_ensure_line(
            cuisine.file_read('/etc/modules'),
            'i2c-dev'
            )
        cuisine.file_write(
            location = '/etc/modules',
            content  = modules_conf,
            mode='644',
            sudo=True
            )

    # Repeated Start Condition
    cuisine.file_write(
        location = '/etc/modprobe.d/i2c.conf',
        content  = 'options i2c_bcm2708 combined=1\n',
        mode='644',
        sudo=True
        )

# WiFi の有効化
def setup_sysctl():
    puts(fabric.colors.green('[Setting sysctl.conf]', True))
    wifi_config = None

    cuisine.file_write(
        location = '/etc/sysctl.conf',
        content  = 'kernel.printk = 3 4 1 3\n'
                 + 'vm.swappiness=1\n'
                 + 'vm.min_free_kbytes = 8192\n'
                 + 'vm.dirty_expire_centisecs = 9000\n'
                 + 'vm.dirty_writeback_centisecs = 6000\n',
        mode='600',
        sudo=True
        )

# WiFi の有効化
def setup_wlan():
    if not cuisine.file_exists('/sys/class/net/wlan0'):
        return

    puts(fabric.colors.green('[Setting Wireless LAN]', True))
    wifi_config = None
    try:
        (file, pathname, description) = imp.find_module('wifi_config', ['.'])
        wifi_config = imp.load_module("wifi_config", file, pathname, description)
    except ImportError:
        puts(fabric.colors.red('SKIP WiFi Configuration. (FAILED to load wifi_config.py)', True))
        return 

    cuisine.file_write(
        location = '/etc/wpa_supplicant/wpa_supplicant.conf',
        content  = 'ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n'
                 + 'update_config=1\n'
                 + 'network={\n'
                 + ('    ssid=\"%s\"\n' % (wifi_config.WIFI_SSID))
                 + ('    psk=%s\n' % (wifi_config.WIFI_PSK))
                 + '    key_mgmt=WPA-PSK\n'
                 + '    proto=WPA2\n'
                 + '    pairwise=CCMP\n'
                 + '    group=CCMP\n'
                 + '}\n',
        mode='600',
        sudo=True
        )

    sudo('sed -i -e \'s/^wpa-roam \/etc\/wpa_supplicant\/wpa_supplicant.conf/'
         + 'wpa-conf \/etc\/wpa_supplicant\/wpa_supplicant.conf/\' /etc/network/interfaces') 
    sudo('sed -i -e \'s/^iface wlan0 inet manual/iface wlan0 inet dhcp/\' /etc/network/interfaces')

    # NOTE: Wifi の場合は「ホスト名-wifi」でアクセスできるようにする
    dhclient_conf = cuisine.text_ensure_line(
        cuisine.file_read('/etc/dhcp/dhclient.conf'),
        'interface "wlan0" { send host-name = concat (gethostname(), "-wifi"); }'
        )

    cuisine.file_write(
        location = '/etc/dhcp/dhclient.conf',
        content  = dhclient_conf,
        mode='644',
        sudo=True
        )

# Local Variables:
# coding: utf-8-unix
# mode: python
# c-basic-offset: 4
# tab-width: 4
# indent-tabs-mode: nil
# End:

