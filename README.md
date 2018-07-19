# Software for CO2 Monitor

## Installation

### Arch Linux
[There is PKGBUILD in AUR](https://aur.archlinux.org/packages/co2mon-git/). The simplest way to [install](https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages) is using yaourt:

`yaourt -S co2mon-git`

### Fedora GNU/Linux and RHEL/CentOS/Scientific Linux
co2mon packages can be installed from russianfedora-free repo or directly from [koji](http://koji.russianfedora.pro/koji/packageinfo?packageID=174)

Install repo:

`su -c 'dnf install --nogpgcheck http://mirror.yandex.ru/fedora/russianfedora/russianfedora/free/fedora/russianfedora-free-release-stable.noarch.rpm http://mirror.yandex.ru/fedora/russianfedora/russianfedora/nonfree/fedora/russianfedora-nonfree-release-stable.noarch.rpm'`

Install co2mon:

`dnf install co2mon`

### From sources

    # macOS
    brew install pkg-config hidapi libmicrohttpd10

    # Ubuntu
    apt-get install g++ pkg-config libhidapi-dev libmicrohttpd-dev

    make
    ./build/co2mond    

## Usage

/dev/hidapi# must have permissions set to be accessable by the running user. If you get an error, try to run the server under root:
    
    sudo ./build/co2mond
    
If that helped, you should set the rules with a udev rule
    
    # set the rule
    sudo cp ./udevrules/99-co2mon.rules /etc/udev/rules.d/
    # Manually trigger udev (not required usually)
    sudo udevadm control --reload-rules
    sudo udevadm trigger

### Webserver

    #To run a webserver on a portnumber
    ./build/co2mond -P portnumber
    
Connect ro the webserver with http://localhost:portnumber

To run on a singleboard computer like NanoPi NEO PLUS you should:
    
    # Set the timezone
    sudo dpkg-reconfigure tzdata
    # Add ntp support to synchronize with internet time
    sudo apt-get install ntp
    # Set the rules for automatic startup with systemd
    sudo cp ./systemd/ntp-wait.service /etc/systemd/system/
    sudo systemctl enable ntp-wait
    sudo cp ./systemd/co2mon.service /etc/systemd/system/
    sudo systemctl enable co2mon
    
You can reboot now and have co2mon service start up automatically and be available on port 15137 (http://localhost:15137)
If you are behind a NAT server like a router then you most probably can't access your device from the internet. You will have either use port forwarding on router, or use a tunnel to another server under your control. SSH can be used:

    # If you own a server with DNS name domain.com and want your device be accessable with port 17137 (GatewayPorts must be enabled in sshd config file on the your-domain.com)
    ssh -N -R 17137:localhost:15137 user@your-domain.com

To avoid asking for a password you need to use generate public-private keys pair and upload your public key on the your-domain.com with this manual http://www.linuxproblem.org/art_9.html. You can make ssh tunnel start automatically on boot. You would have to create key pair using the same manual but with sudo, because systemd runs under root and the key pair must belong to root. Check that ssh do not ask a password on connect with

    # Run that to see that no password is asked and no dialog is initiated
    sudo ssh -N -R 17137:localhost:15137 user@your-domain.com

and your device is accessible with http://your-domain.com:17137. Then edit ./systemd/ssh-tunnel.service to set it on your domain etc. Then:

    # Set the rules for automatic ssh tunnel startup with systemd
    sudo cp ./systemd/ssh-tunnel.service /etc/systemd/system/
    sudo systemctl enable ssh-tunnel
    # Check that it is working with either reboot or
    sudo systemctl start ssh-tunnel

## See also

  * [ZyAura ZG01C Module Manual](http://www.zyaura.com/support/manual/pdf/ZyAura_CO2_Monitor_ZG01C_Module_ApplicationNote_141120.pdf)
  * [RevSpace CO2 Meter Hacking](https://revspace.nl/CO2MeterHacking)
  * [Photos of the device and the circuit board](http://habrahabr.ru/company/masterkit/blog/248403/)
