#!/bin/sh

cp /lib/modules/3.2.14popcornsock+/kernel/drivers/net/ethernet/intel/ixgbe/snull.ko /home/saif_local/from_akshai/latest_ramdisk/snull.ko


cd /home/saif_local/from_akshai/latest_ramdisk
echo `pwd`

find . | cpio --create --format='newc' > ../newinitrd

cd ..

gzip newinitrd

echo `pwd`
cp newinitrd.gz david_utils/ramdisk.img
