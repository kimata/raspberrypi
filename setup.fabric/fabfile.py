# -*- coding: utf-8-unix -*-

# Raspberry Pi セットアップ用 fabric レシピ

from fabric.api import settings, sudo 
from fabric.decorators import task
from cuisine import *

@task
def setup_package():
    # raspi-config は使わないので，自動起動を無効化
    if file_exists('/etc/profile.d/raspi-config.sh'):
        run('sudo rm -f /etc/profile.d/raspi-config.sh')

    with settings(mode_sudo()):
        package_update()
        package_upgrade()

        # パッケージのインストール
        packages = '''
        emacs23-nox tmux ruby git zsh mlocate
        '''.split()
        for pkg in packages:
            package_ensure(pkg)

        # タイムゾーンとロケールの設定
        run('echo "Asia/Tokyo" > /etc/timezone')
        run('dpkg-reconfigure -f noninteractive tzdata')
        run('sed -i -e \'s/^en_GB.UTF-8 UTF-8/# en_GB.UTF-8 UTF-8/\' /etc/locale.gen')
        run('sed -i -e \'s/^# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/\' /etc/locale.gen')
        run('sed -i -e \'s/^# ja_JP.UTF-8 UTF-8/ja_JP.UTF-8 UTF-8/\' /etc/locale.gen')
        run('dpkg-reconfigure --frontend=noninteractive locales')
        run('updatedb')

        # 公開鍵のコピー
        with open(os.path.expanduser('~/.ssh/id_rsa.pub')) as f:
            pubkey = f.read()
        with mode_sudo():
            ssh_authorize("pi", pubkey)

        # rootfs サイズを拡張
        run('raspi-config --expand-rootfs')

        # I2C の有効化
        run('sed -i -e \'s/^blacklist i2c-bcm2708/# blacklist i2c-bcm2708/g\' ' +
            '/etc/modprobe.d/raspi-blacklist.conf')

        # 再起動
        run('reboot')
        
