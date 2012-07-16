/*
 * Boot parameters and other support stuff for MKLinux
 *
 * (C) Ben Shelton <beshelto@vt.edu> 2012
 */

#include <linux/rcupdate.h>
#include <linux/rculist.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/smp.h>
#include <linux/cpu.h>

int mklinux_boot __initdata;
EXPORT_SYMBOL(mklinux_boot);

static int __init setup_mklinux(char *arg)
{
        mklinux_boot = 1;
        return 0;
}
early_param("mklinux", setup_mklinux);
