#!/bin/bash

function line {
	echo "**************************************************"
	echo "** Processing $1"
	echo "**************************************************"
}

case $1 in
	setup)
		line "Setup"
		export ARCH=arm
		export CROSS_COMPILE=arm-linux-gnueabi-
		;;
	k)
		# Compile kernel
		line "kernel"
		make -j9 zImage
		# create dtb
		line "DTB"
		make dss11-1gb.dtb
		# Attach dtb at end of image
		cat arch/arm/boot/zImage arch/arm/boot/dts/dss11-1gb.dtb > zImage.tmp
		# make uboot image from above result
		mkimage -A arm -O linux -C none -T kernel -a 20008000 -e 20008000 -n linux-3.14 -d zImage.tmp uImage
		rm zImage.tmp
		line "kernel -> tftp"
		sudo cp uImage /var/lib/tftpboot/
		line "DONE!"
		;;
	c)
		make clean
		;;
	*)
		echo "mk.sh [setup|k|c]"
		;;
esac




