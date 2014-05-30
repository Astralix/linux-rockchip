#!/bin/bash

function line {
	echo "**************************************************"
	echo "** Processing $1"
	echo "**************************************************"
}

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabi-
OUTDIR=../out
INSTALL_MOD_PATH=modules
NCPU=12

if [ $USE_CCACHE = 1 ]
then
	echo "*** USING CCACHE ***"
	CC="ccache arm-linux-gnueabi-gcc"
fi

while getopts ckmi OPT; do
	case $OPT in
	c)
		make clean
		rm -rf $INSTALL_MOD_PATH
		;;
	k)
		# Compile kernel
		line "kernel"
		make -j$NCPU zImage || exit 1
		# create dtb
		line "DTB"
		make dss11-1gb.dtb || exit 1
		# Attach dtb at end of image
		cat arch/arm/boot/zImage arch/arm/boot/dts/dss11-1gb.dtb > zImage.tmp
		# make uboot image from above result
		mkimage -A arm -O linux -C none -T kernel -a 20008000 -e 20008000 -n linux-3.14 -d zImage.tmp uImage
		rm zImage.tmp
		line "kernel -> tftp"
		sudo cp uImage /var/lib/tftpboot/
		line "DONE!"
		;;
	m)
		# Compile modules
		line "modules"
		export INSTALL_MOD_PATH
		make -j$NCPU modules || exit 1
		mkdir -p $INSTALL_MOD_PATH
		make modules_install
		make firmware_install
		find $INSTALL_MOD_PATH -type l -exec rm -f {} \;
		;;
	i)	
		# Install
		line "Installing kernel to tftp"
		sudo cp uImage /var/lib/tftpboot/ || exit 1
		line "DONE"
		;;
	\?)
		echo $USAGE >&2
		exit 1
		;;
	setup)
		line "Setup"
		export ARCH
		export CROSS_COMPILE
		export INSTALL_MOD_PATH
		;;
	esac
done

# Remove the switches we parsed above.
shift `expr $OPTIND - 1`
case $1 in
	setup)
		export ARCH
		export CROSS_COMPILE
		export INSTALL_MOD_PATH
		;;
	help)
		echo $USAGE >&2
		;;
esac

