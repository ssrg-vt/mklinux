#!/bin/sh

echo "exporting path.."
export PATH=/opt/mpss/3.2/sysroots/x86_64-mpsssdk-linux/usr/bin:/opt/mpss/3.2/sysroots/x86_64-mpsssdk-linux/usr/bin/k1om-mpss-linux:$PATH

echo "mounting debugfs.."
mount -t debugfs none /sys/kernel/debug

