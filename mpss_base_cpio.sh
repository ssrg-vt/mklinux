#!/bin/sh

# Antonio Barbalace, 2014

MIC="mic0"

KERNEL_PATH=""
WORKING_DIR="/var/mpss/base/"

MPSS_PATH="/usr/share/mpss/"
MPSS_BOOT=$MPSS_PATH"boot/"

MPSS_CPIO_GZ="initramfs-knightscorner.cpio.gz"
MPSS_INITRAMFS=$MPSS_BOOT$MPSS_CPIO_GZ

MY_CPIO_GZ="initramfs-3.2.14+mpss3.2.cpio.gz"
MY_INITRAMFS=$MPSS_BOOT$MY_CPIO_GZ

case $1 in
  extract)
    echo "unpacking $MPSS_INITRAMFS in $WORKING_DIR"
    mkdir $WORKING_DIR
    cd $WORKING_DIR
    gunzip <$MPSS_INITRAMFS | cpio -i
    ;;
  create)
    echo "packing $WORKING_DIR in $MY_INITRAMFS"
    cd $WORKING_DIR
    find . | cpio -o -c | gzip >../$MY_CPIO_GZ
    rm -f $MY_INITRAMFS
    mv ../$MY_CPIO_GZ $MY_INITRAMFS
    ;;
  install)
    echo "installing $MY_INITRAMFS as $MIC ramfs"
    micctrl --base=CPIO --new=$MY_INITRAMFS $MIC
    micctrl --updateramfs $MIC
    ;;
  default)
    echo "reset $MPSS_INITRAMFS as $MIC ramfs"
    micctrl --base=CPIO --new=$MPSS_INITRAMFS $MIC
    micctrl --updateramfs $MIC
    ;;
  *)
    echo "Usage: $0 [extract|create]"
    exit 2
esac

