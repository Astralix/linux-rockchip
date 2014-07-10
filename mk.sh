#!/bin/bash

#
# Basic path setup
#
INSTALL_MOD_PATH=./dss11_modules
TFTP_PATH=~/tftpboot/

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabi-

OUTDIR=../dss11

#
# Parallel build setup
#
NCPU=`grep -c ^processor /proc/cpuinfo`
let NPRC=$NCPU*2

#
# build functions
#

function line {
	echo "**************************************************"
	echo "** $1"
	echo "**************************************************"
}

function finit {
	echo "**** DONE ****************************************"
}

function check_ip {
	if [ -z "$1" ]; then
		echo "Error: give target device IP"
		exit 1
	else
		echo "Install Target $1"
	fi
}

function m_kernel {
	if [ -z "$1" ]; then
		echo "Error: give target type [1gb|sdc]"
		exit 1
	fi
	# Compile kernel
	line "Compiling kernel: $1"
	make -j$NPRC zImage || exit 1
	# create dtb
	dtsfile="dss11-$1.dtb"
	line "Compiling DTB: $dtsfile"
	make $dtsfile || exit 1
	# Attach dtb at end of image
	cat arch/arm/boot/zImage arch/arm/boot/dts/$dtsfile > zImage.tmp
	# make uboot image from above result
	mkimage -A arm -O linux -C none -T kernel -a 20008000 -e 20008000 -n linux-3.14 -d zImage.tmp uImage
	rm zImage.tmp
	finit
}

function i_kernel {
	line "Copy kernel -> tftp"
	cp uImage /home/uprinz/tftpboot/
	finit
}

function m_modules {
	# Compile Modules
	line "Compiling modules"
	make -j$NPRC modules || exit 1
	finit
}

function i_modules {
	line "Install modules"
	export INSTALL_MOD_PATH
	mkdir -p $INSTALL_MOD_PATH
	make modules_install
	finit
}

function i_firmware {
	# Install firmware blobs
	line "Install all firmware"
	mkdir -p $INSTALL_MOD_PATH
	make firmware_install
	finit
}

function t_modules_firmware {
	#Install Modules to target
	line "Installing modules and firmware on target at $INSTALL_MOD_PATH"
	find $INSTALL_MOD_PATH -type l -exec rm {} \;
	pscp -scp -r $INSTALL_MOD_PATH/ root@$1:/
	finit
}

function dl_wifi_drivers {
	#Download additional WiFi firmware blobs
	line "Downloading additional WiFi firmware"
	mkdir -p $INSTALL_MOD_PATH/lib/firmware/rtlwifi
	pushd $INSTALL_MOD_PATH/lib/firmware/rtlwifi
	wget -r -l1 --no-parent -A.bin -nd http://ftp2.halpanet.org/source/_dev/linux-firmware.git/rtlwifi/
	popd
	finit
}

while getopts ckmf OPT; do
	case $OPT in
		c)
			make clean
			rm -rf $INSTALL_MOD_PATH
			;;
		k)
			m_kernel $2
			i_kernel
			;;
		m)
			m_modules
			i_modules
			;;
		f)
			i_firmware
			;;
		\?)
			echo $USAGE >&2
			exit 1
			;;
	esac
done

# Remove the switches we parsed above.
shift `expr $OPTIND - 1`

# Check for left over parameters
if [ -z $1 ] ; then exit 0; fi

case $1 in
	sdc)
		;;
	1gb)
		;;
	setup)
		line "Setup"
		export ARCH
		export CROSS_COMPILE
		export INSTALL_MOD_PATH
		;;
	all)
		check_ip $3
		m_kernel $2
		i_kernel
		m_modules
		i_modules
		i_firmware
		t_modules_firmware $3
		;;
	install)	
		check_ip $2
		t_modules_firmware $2
		;;
	download)
		dl_wifi_drivers
		;;
	*)
		echo "mk.sh -[c|k|m|f] [sdc|1gb]"
		echo "  -c	clean"
		echo "  -k	build kernel"
		echo "  -m	build modules"
		echo "  -f	build firmware"
		echo "  sdc	build for sd-card model"
		echo "  1gb	build for 1GB model"
		echo ""
		echo "source mk.sh setup"
		echo "  setup	Call with source to setup toolchain exports."
		echo "  all	Complete build plus installation."
		echo ""
		echo "mk.sh install [target-ip]"
		echo "  install	Copy over modules and firmware to target."
		;;
esac
