#!/bin/bash

# create dtb
make dss11-1gb.dtb

# Attach dtb at end of image
cat arch/arm/boot/zImage arch/arm/boot/dts/dss-1gb.dtb > zImage.tmp

# make uboot image from above result
mkimage -A arm -O linux -C none -T kernel -a 20008000 -e 20008000 -n linux-3.14 -d zImage.tmp uImage
rm zImage.tmp
sudo cp uImage /var/lib/tftpboot/

