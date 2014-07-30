#!/bin/bash
#first, get the kernel version
VER=`cat ../mklinux-build/include/config/kernel.release`
THREADS=48
#`cat /proc/cpuinfo | grep processor | wc -l`

echo "Building kernel ${VER} for both xeon and xeonPhi on ${THREADS} threads"

if [ $(/usr/bin/id -u) -ne 0 ]; then
	echo "Please run this script as root."
	exit 2
fi

if [ ! -f ../mklinux-build/.config ]
then
	echo "No config for xeon found... running make menuconfig"
	exit 2
fi

if [ ! -f ../mklinux-build-phi/.config ]
then
        echo "No config for xeonPhi found... running make menuconfig"
	exit 2        
fi

check_errors() {
        # Function. Parameter 1 is the return code
        # Para. 2 is text to display on failure.
        if [ "${1}" -ne "0" ]; then
                echo "ERROR # ${1} : ${2}"
                # as a bonus, make our script exit with the right error code.
                exit ${1}
        fi
}

echo "Building for xeon"
KBUILD_OUTPUT=../mklinux-build/ make O=../mklinux-build/  bzImage -j $THREADS
check_errors $? "make bzImage failed"
KBUILD_OUTPUT=../mklinux-build/ make O=../mklinux-build/ -j $THREADS modules
check_errors $? "make modules failed"
KBUILD_OUTPUT=../mklinux-build/ make O=../mklinux-build/ -j $THREADS modules_install
check_errors $? "make modules_install failed"

#If all the above were successful, we doubt the rest will fail
echo "Copying vmlinuz, config, and .map to /boot..."
cp -f ../mklinux-build/arch/x86/boot/bzImage /boot/vmlinuz-"$VER"
cp -f ../mklinux-build/.config /boot/config-"$VER"
cp -f ../mklinux-build/System.map /boot/System.map-"$VER"

echo "creating initramfs"
dracut -f initramfs-"$VER".img $VER
check_errors $? "dracut failed"
cp -f initramfs-"$VER".img /boot/initramfs-"$VER".img
rm initramfs-"$VER".img

echo "Building for xeonPhi"
KBUILD_OUTPUT=../mklinux-build-phi/ make ARCH=k1om CROSS_COMPILE=k1om-mpss-linux- KBUILD_NOPEDANTIC=1 O=../mklinux-build-phi/ -j $THREADS
check_errors $? "make bzImage failed"

#If all the above were successful, we doubt the rest will fail
echo "running mpss_install_bzImage.sh"
./mpss_install_bzImage.sh



