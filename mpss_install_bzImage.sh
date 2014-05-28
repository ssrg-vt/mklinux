#!/bin/sh 

# Antonio Barbalace, 2014

MIC="mic0"

KERNEL_PATH=""
KERNEL_BZIMAGE=$KERNEL_PATH"arch/x86/boot/bzImage"
KERNEL_SYSMAP=$KERNEL_PATH"System.map"

MPSS_PATH="/usr/share/mpss/"
MPSS_BOOT=$MPSS_PATH"boot/"
MY_BZIMAGE=$MPSS_BOOT"bzImage-3.2.14+mpss3.2-knightscorner"
MY_SYSMAP=$MPSS_BOOT"System.map-3.2.14+mpss3.2-knightscorner"

cp $KERNEL_BZIMAGE $MY_BZIMAGE
cp $KERNEL_SYSMAP $MY_SYSMAP

ELEM="\$1"
BUFF_ADDR_PAT="[\t ]__log_buf$"
cat $MY_SYSMAP | grep "$BUFF_ADDR_PAT"
DMESG_BUFF_ADDR=`cat $MY_SYSMAP | awk "/$BUFF_ADDR_PAT/ {print $ELEM}"`
BUFF_LEN_PAT="[\t ]log_buf_len$"
cat $MY_SYSMAP | grep "$BUFF_LEN_PAT"
DMESG_BUFF_LEN=`cat $MY_SYSMAP | awk "/$BUFF_LEN_PAT/ {print $ELEM}"`

echo "setting dmesg symbols"
DMESG_BUFF="/sys/class/mic/$MIC/log_buf_addr"
DMESG_LEN="/sys/class/mic/$MIC/log_buf_len"

echo $DMESG_BUFF_ADDR > $DMESG_BUFF
echo $DMESG_BUFF_LEN > $DMESG_LEN
cat $DMESG_BUFF
cat $DMESG_LEN

echo "installed in $MPSS_BOOT"
