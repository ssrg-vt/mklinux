#!/bin/sh

N_JOBS=8

arm64_mkimage() {
echo "mkimage"
mkimage -A arm -O linux -C none -T kernel -a 0x00080000 -e 0x00080000 -n "Linux-LE" -d ./arch/arm64/boot/Image ./arch/arm64/boot/uImage

echo "cp"
cp arch/arm64/boot/dts/apm-mustang.dtb /boot/
cp arch/arm64/boot/uImage /boot/
}

amd64_modules() {
echo "modules"
make -j${N_JOBS} modules 
make -j${N_JOBS} modules_install
}

ARCH=`uname -a`
ARCH_ARM64="aarch64"
ARCH_AMD64="x86_64"

echo $ARCH

echo "make"
make -j${N_JOBS} 
case $ARCH in *$ARCH_AMD64*): ; amd64_modules ;; esac

echo "install"
make -j${N_JOBS} install
case $ARCH in *$ARCH_ARM64*): ; arm64_mkimage ;; esac

echo "DONE"
             
