#!/bin/bash

INSTALL_MOD_PATH=../dss11_modules
TFTP_PATH=~/tftpboot/
IMPEXISTS="0"

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
	# Compile kernel
	line "Compiling kernel"
	make -j9 zImage
	# create dtb
	line "Compiling DTB"
	make dss11-1gb.dtb
	# Attach dtb at end of image
	cat arch/arm/boot/zImage arch/arm/boot/dts/dss11-1gb.dtb > zImage.tmp
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
	make -j9 modules
	finit
}

function i_modules {
	line "Install modules"
	export INSTALL_MOD_PATH
	if [ "$IMPEXISTS" = "0" ] ; then
		export INSTALL_MOD_PATH
		rm -rf $INSTALL_MOD_PATH
		mkdir -p $INSTALL_MOD_PATH
		IMPEXISTS="1"
	fi
	make modules_install
	finit
}

function i_firmware {
	# Install firmware blobs
	line "Install all firmware"
	if [ "$IMPEXISTS" = "0" ] ; then
		export INSTALL_MOD_PATH
		rm -rf $INSTALL_MOD_PATH
		mkdir -p $INSTALL_MOD_PATH
		IMPEXISTS="1"
	fi
	make firmware_install
	finit
}

function t_modules_firmware {
	#Install Modules to target
	line "Installing modules and firmware on target"
	find $INSTALL_MOD_PATH -type l -exec rm {} \;
	pscp -scp -r $INSTALL_MOD_PATH/ root@$1:/
	finit
}

while getopts ckmf OPT; do
	case $OPT in
		c)
			make clean
			rm -rf $INSTALL_MOD_PATH
			;;
		k)	
			m_kernel
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

shift `expr $OPTIND - 1`
if [ -z $1 ] ; then exit 0; fi

case $1 in
	setup)
		line "Setup"
		export ARCH=arm
		export CROSS_COMPILE=arm-linux-gnueabi-
		;;
	all)
		check_ip $2
		m_kernel
		i_kernel
		m_modules
		i_modules
		i_firmware
		t_modules_firmware $2
		;;
	install)	
		check_ip $2
		t_modules_firmware $2
		;;
	*)
		echo "mk.sh -[c|k|m|f]"
		echo "  -c	clean"
		echo "  -k	build kernel"
		echo "  -m	build modules"
		echo "  -f	build firmware"
		echo ""
		echo "mk.sh [setup|all]"
		echo "  setup	Call with source to setup toolchain exports."
		echo "  all	Complete build plus installation."
		echo ""
		echo "mk.sh install [target-ip]"
		echo "  install	Copy over modules and firmware to target."
		;;
esac


