#!/bin/bash
#first, get the kernel version
make include/config/kernel.release
VER=`cat include/config/kernel.release`
CC=gcc
THREADS=`cat /proc/cpuinfo | grep processor | wc -l`

echo "Building kernel ${VER} with ${CC} on ${THREADS} threads"

if [ $(/usr/bin/id -u) -ne 0 ]; then
	echo "Please run this script as root."
	exit 2
fi

# Determine OS platform (and, if applicable, Linux distribution)
# Courtesy of http://legroom.net/2010/05/05/generic-method-determine-linux-or-unix-distribution-name
detect_os() {
	UNAME=$(uname | tr "[:upper:]" "[:lower:]")
	# If Linux, try to determine specific distribution
	if [ "$UNAME" == "linux" ]; then
	    # If available, use LSB to identify distribution
	    if [ -f /etc/lsb-release -o -d /etc/lsb-release.d ]; then
		DISTRO=$(lsb_release -i | cut -d: -f2 | sed s/'^\t'//)
	    # Otherwise, use release info file
	    else
		DISTRO=$(ls -d /etc/[A-Za-z]*[_-][rv]e[lr]* | grep -v "lsb" | cut -d'/' -f3 | cut -d'-' -f1 | cut -d'_' -f1)
	    fi
	fi
	# For everything else (or if above failed), just use generic identifier
	[ "$DISTRO" == "" ] && DISTRO=$UNAME

	# Convert to lowercase before echoing result
	echo $DISTRO | tr "[:upper:]" "[:lower:]"
}

check_errors() {
	# Function. Parameter 1 is the return code
	# Para. 2 is text to display on failure.
	if [ "${1}" -ne "0" ]; then
		echo "ERROR # ${1} : ${2}"
		# as a bonus, make our script exit with the right error code.
		exit ${1}
	fi
}

if [ ! -f .config ]
then
	echo "No config found... running make menuconfig"
	make menuconfig
fi

make -j$THREADS bzImage CC=$CC
check_errors $? "make bzImage failed"
make -j$THREADS modules CC=$CC
check_errors $? "make modules failed"
sudo make modules_install
check_errors $? "make modules_install failed"
sudo make headers_install INSTALL_HDR_PATH=/usr/src/linux-headers-"$VER"
check_errors $? "make headers_install failed"

#If all the above were successful, we doubt the rest will fail
echo "Copying vmlinuz, config, and .map to /boot..."
sudo cp arch/x86/boot/bzImage /boot/vmlinuz-"$VER"
sudo cp .config /boot/config-"$VER"
sudo cp System.map /boot/System.map-"$VER"

# Determine Linux distribution
DISTRO=`detect_os`

# Generate ramfs and update bootloader as needed by different distributions
if [ "$DISTRO" == "ubuntu" ]; then
	echo "Running update-initramfs and update-grub for Ubuntu..."
	sudo update-initramfs -c -k "$VER"
	sudo update-grub

elif [ "$DISTRO" == "arch" ]; then
	echo "Running mkinitcpio for Arch Linux..."
	mkinitcpio -k $VER -g /boot/kernel-$VER.img

else
	echo "Error: Could not figure out how to generate ramfs or update bootloader on your distribution: $DISTRO"  
	echo "Please do this manually"

fi
