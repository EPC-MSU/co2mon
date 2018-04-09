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
    
    # make file accessible only by root
    sudo chown root ./udevrules/99-co2mon.rules
    sudo chmod 600 ./udevrules/99-co2mon.rules
    # set the rule
    sudo cp ./udevrules/99-co2mon.rules /etc/udev/rules.d/
    # Manually trigger udev (not required usually)
    udevadm control --reload-rules
    udevadm trigger

### Webserver

    #To run a webserver on a portnumber
    ./build/co2mond -P portnumber
    
Connect ro the webserver with http://localhost:portnumber

To run on a singleboard computer like NanoPi NEO PLUS you should:
    
    # Set the timezone
    sudo dpkg-reconfigure tzdata
    # Make the server run at startup
    ...

## See also

  * [ZyAura ZG01C Module Manual](http://www.zyaura.com/support/manual/pdf/ZyAura_CO2_Monitor_ZG01C_Module_ApplicationNote_141120.pdf)
  * [RevSpace CO2 Meter Hacking](https://revspace.nl/CO2MeterHacking)
  * [Photos of the device and the circuit board](http://habrahabr.ru/company/masterkit/blog/248403/)
