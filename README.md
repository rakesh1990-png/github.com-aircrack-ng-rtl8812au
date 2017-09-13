# RTL8812AU/21AU and RTL8814AU drivers
# with monitor mode and frame injection
# currently ONLY supports RTL8812AU, NOT the RTL8814AU. SEE TODO

## TODO
* Support for RTL8814AU (adding HAL) should be added, a patch on the v5.1.5 branch could be modified and used
  see https://github.com/aircrack-ng/rtl8812au/commit/55e81afe4f9fa556cafe6e789b91043535d85b3a
  
  It needs some modifications in order to "fit", both in Makefile and other files. Some work is currently ongoing, but
  feel free to team up on it here with us. Check .rej files after patching and see what needs to be modified.


## DKMS
This driver can be installed using [DKMS]. This is a system which will automatically recompile and install a kernel module when a new kernel gets installed or updated. To make use of DKMS, install the `dkms` package, which on Debian (based) systems is done like this:
```
sudo apt install dkms
```

## Installation of Driver
In order to install the driver open a terminal in the directory with the source code and execute the following command:
```
sudo ./dkms-install.sh
```

## Removal of Driver
In order to remove the driver from your system open a terminal in the directory with the source code and execute the following command:
```
sudo ./dkms-remove.sh
```

## Make
For building & installing the RTL8812AU driver with 'make' use
```
make
make install
```

## Notes
For Ubuntu 17.04 add the following lines
```
[device]
wifi.scan-rand-mac-address=no
```
at the end of file /etc/NetworkManager/NetworkManager.conf and restart NetworkManager with the command:
```
sudo service NetworkManager restart
```

## Credits
```
astsam - for the main work + monitor/injection support - https://github.com/astsam
```

## Other Sources
```
astsam     - https://github.com/astsam/rtl8812au
gnab       - https://github.com/gnab/rtl8812au
zebulon2   - https://github.com/zebulon2/rtl8812au
paspro     - https://github.com/paspro/rtl8812au
ulli-kroll - https://github.com/ulli-kroll/rtl8821au
```
