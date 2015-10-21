How run an application in ft-popcorn:

PRIMARY:

-- boot primary kernel

NOTE: if you want to debug the secondary kernel with an extra NIC, you need to blacklist that NIC on the boot parameter of the primary kernel e.g.: pci_dev_flags=0x8086:0x10c9:0.1:b.

As example my boot parameters in /etc/grub.d/20_popcorn_linux are:

menuentry "FT_Popcorn_Linux_marina" {
	recordfail
        gfxmode $linux_gfx_mode
        insmod gzio
        insmod part_msdos
        insmod ext2
        set root='(hd0,msdos1)'
        search --no-floppy --fs-uuid --set=root 6bd7ec5c-96ec-4c5d-9def-576127193147
        linux   /boot/vmlinuz-3.2.14-ft-popcorn+ root=UUID=6bd7ec5c-96ec-4c5d-9def-576127193147 ro   console=tty0 console=ttyS0,115200 earlyprintk=ttyS0,115200 nmi_watchdog=0 present_mask=0 vty_offset=0x1fac000000 mem=1920M norandmaps pci_dev_flags=0x8086:0x10c9:0.1:b
        initrd  /boot/initrd.img-3.2.14-ft-popcorn+

}

-- boot secondary kernel using mklinux-utils (branch ft-replication)
	>./mklinux_boot.sh 1

NOTE: if you want to debug the secondary kernel with an extra NIC blacklist only the primary NIC from the boot parameters.

As example my boot parameters of kernel 1 are (cat of boot_args_1.args):

earlyprintk=ttyS0,115200 console=ttyS0,115200 acpi_irq_nobalance no_ipi_broadcast=1 lapic_timer=1000000 pci_dev_flags=0x1002:0x439c:b,0x1002:0x4390:b,0x8086:0x10c9:0.0:b,0x102b:0x0532:b,0x1002:0x439d:b,0x1002:0x4396:b,0x1002:0x4397:b,0x1002:0x4398:b,0x1002:0x4399:b,0x1002:0x4385:b mklinux debug vty_offset=0x1fac000000 present_mask=1 memmap=1616M@1920M memmap=512M@4096M memmap=1919M$1M mem=4608M memmap=512k$64k norandmaps
 
-- run tunnelize.sh
	>./tunnelize.sh

-- connect to secondary kernel using ssh 10.1.2.2
	> ssh 10.1.2.2

SECONDARY:

-- set the dummy_device_driver ip as the one used by the NIC of the primary kernel

	>ifconfig ft_dummy_driver IP

NOTE: if you use a second NIC to debug, be sure that the NIC is up and connected to the network and that it will not expose the IP address of the ft_dummy_driver during ARP.

	echo 1 > /proc/sys/net/ipv4/conf/eth0/arp_ignore
	echo 2 > /proc/sys/net/ipv4/conf/eth0/arp_announce
	ifconfig ft_dummy_driver 10.1.1.48
	dhcpcd eth0
	echo 1 > /proc/sys/net/ipv4/conf/eth0/arp_ignore
	echo 2 > /proc/sys/net/ipv4/conf/eth0/arp_announce 

PRIMARY:

-- enter in popcorn namespace using mklinux-utils (branch ft-replication). When running popcorn.sh the parameter requested is the number of replicas desired (if using 2 kernels=> 2 replicas).

	> cd  mklinux-utils/ns/
	> ./launch_ns.sh
	> ./popcorn.sh 2

-- run the application. NOTE the absolute path of the application needs to be present on both kernels to work, otherwise you will have segfault.

NOTE: if you create files without absolute path, they will be on the / directory on secondary replicas.

DEBUG:

-> to see if the application started on the secondary replica check dmesg.
   if you see something like:
	[  290.453592] maybe_create_replicas: Replica list of TCPEchoOnlyRead pid 2507
	[  290.453595] primary: {pid: 2507, kernel 0}, secondary: {pid: 563, kernel 1} 
	[  290.453600] comm: TCPEchoOnlyRead pid 2507

   on all replicas, it was correctly started.

-> to use tcpdump to capture packets from a client:
   tcpdump -ni eth0 -s0 -w /var/tmp/capture.pcap

-> to disable ARP discovering and add a static entry in arp table:
   sudo arp
   sudo arp -s 10.1.1.48  00:25:90:5a:cd:a6

-> to disable NIC checksum computation on both tx and rx:
   sudo ethtool --show-offload  eth0
   sudo ethtool --offload  eth0  rx off  tx off
