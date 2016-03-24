/*
 */

#include <linux/kernel.h>
#include <linux/getcpu.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/vsyscall.h>
#include <asm/vgtod.h>

notrace long
__vdso_popcorn_migrate(void)
{
	return __get_popcorn_migrate();
}

long popcorn_migrate(void)
	__attribute__((weak, alias("__vdso_popcorn_migrate")));
