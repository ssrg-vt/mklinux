#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/compat.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/power_sensor.h>
#include "apm_i2c_access.h"

#define DEV_NAME	"Power Sensor"
#define I2C_BUS		IIC_1
#define I2C_SA		0x46
#define I2C_RA		0x96
#define RD_LENGTH	2

static int major = 0;
static struct class *ps_class;
static struct completion start;
static struct completion stop;
static int sampling_interval = 200; // in ms
static atomic_t stop_sampling;
static atomic_t power_value;
static atomic_t exit_ps_read_thread;
static struct task_struct *ps_thread = NULL;
static dev_t ps_dev_num;
static struct device *ps_dev = NULL;

int read_inst_power(void) 
{
	u32 data;
	int ret = -1;
	int N, Y, pout;

	ret = i2c_sensor_read(I2C_BUS,
			I2C_SA, I2C_RA, RD_LENGTH, &data);

	if(ret < 0) {
		printk(KERN_ERR "%s: I2C read failed [%d]\n",
				__func__, ret);
		pout = -1;
	} else {
		/* Calculate power */
		N = 32 - ((data >> 11) & 0x1F);
		Y = data & 0x7ff;
		pout = (int)((2*Y >> N));
	}
	return pout;
}

int ps_read_thread(void *arg)
{	
	int ret = -1;
	int count = 0;
	int local_power = 0;
	int pout = 0;

	printk("%s: Starting...\n", __func__);
	do
	{
		//printk("%s: Waiting...\n", __func__);
		/* wait for the application to trigger start */
		wait_for_completion(&start);
		//printk("%s: Wokeup...\n", __func__);

		if(kthread_should_stop() || (atomic_read(&exit_ps_read_thread) == 1)) {
			break;
		}

		while(atomic_read(&stop_sampling) == 0)
		{
			pout = read_inst_power();
			if(pout <= 0) {
				printk(KERN_ERR "%s: I2C read failed [%d]\n",
						__func__, ret);
			} else {
				/* Accumulate the value */
				local_power += pout;
				count++;
			}
			msleep(sampling_interval);
		}

		if(count) {
			atomic_set(&power_value, local_power/count);
		}
		complete(&stop);
	} while(1);
	printk("%s: Stopping...\n", __func__);
	return 0;
}

static long ps_unlocked_ioctl(struct file   *file,
		unsigned int  cmd,
		unsigned long data)
{
	int ret = -1;
	int pwr = 0;
	void __user *arg = (void __user *) data;

	//printk("%s+\n", __func__);

	switch (cmd)
	{
		case PSCTL_START:
		{
			//printk("%s: PSCTL_START\n", __func__);
			/* clear the average power value */
			atomic_set(&stop_sampling, 0);
			atomic_set(&power_value, 0);
			complete(&start);
			ret = 0;
			break;
		}

		case PSCTL_STOP:
		{
			//printk("%s: PSCTL_STOP\n", __func__);
			atomic_set(&stop_sampling, 1);
			wait_for_completion(&stop);

			/* read and return the average */
			/* power value to user         */
			pwr = atomic_read(&power_value);

			if(pwr != 0) {
				ret = copy_to_user(arg, &pwr, sizeof(int));
			} else {
				ret = -1;
			}
			break;
		}

		case PSCTL_READ_PWR:
		{
			//printk("%s: PSCTL_READ_PWR:\n", __func__, cmd);
			pwr = read_inst_power();

			if(pwr != 0) {
				ret = copy_to_user(arg, &pwr, sizeof(int));
			} else {
				ret = -1;
			}
			break;
		}

		default:
		{
			//printk("%s: Err: Invalid command - %d\n", __func__, cmd);
			ret = -1;
			break;
		}
	}
	//printk("%s-\n", __func__);
	return ret;
}

static int ps_open(struct inode *inode, struct file *file)
{
	//printk("%s+\n", __func__);
	//printk("%s-\n", __func__);
	return 0;
}

static int ps_release(struct inode *inode, struct file *file)
{
	//printk("%s+\n", __func__);
	//printk("%s-\n", __func__);
	return 0;
}

static const struct file_operations ps_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = ps_unlocked_ioctl,
	.open           = ps_open,
	.release        = ps_release,
};

static int __init pwr_sensor_init(void)
{
	int ret = 0;

	printk(KERN_INFO "\nps_sensor: Power sensor driver - Initializing...\n");

	ps_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(ps_class)) {
		printk(KERN_ERR "pwr_sensor: can't register device class\n");
		ret = -1;
		goto cleanup;
	}

	major = register_chrdev(0, DEV_NAME, &ps_fops);

	if(major < 0) {
		printk(KERN_ERR "pwr_sensor: failed to register device\n");
		ret = -1;
		goto cleanup;
	}

	ps_dev_num = MKDEV(major, 0);

	ps_dev = device_create(ps_class, NULL, ps_dev_num, NULL, "power_sensor0");
	if(!ps_dev) {
		printk(KERN_ERR "pwr_sensor: failed to create dev entry\n");
		ret = -1;
		goto cleanup;
	}

	/* Create kernel thread for periodic reading */
	ps_thread = kthread_run(&ps_read_thread, 
			NULL, "Power Sensor Read Thread");

	if(!ps_thread) {
		printk(KERN_ERR "pwr_sensor: Failed to create kthread\n");
		ret = -1;
		goto cleanup;
	}

	/* Initialize completion variables */
	init_completion(&start);
	init_completion(&stop);
	atomic_set(&stop_sampling, 0);
	atomic_set(&power_value, 0);
	atomic_set(&exit_ps_read_thread, 0);
	printk(KERN_INFO "ps_sensor: Power sensor driver - Initializing...[Done]\n");
	return ret;

cleanup:
	printk(KERN_INFO "ps_sensor: Power sensor driver - Initializing...[Failed]\n");
	if(major > 0) {
		printk(KERN_INFO "ps_sensor: unregistering character device\n");
		unregister_chrdev(major, DEV_NAME);
	}

	if(!IS_ERR(ps_class)) {
		if(ps_dev) {
			printk(KERN_INFO "ps_sensor: destroying device created\n");
			device_destroy(ps_class, ps_dev_num);
		}
		printk(KERN_INFO "ps_sensor: destroying class\n");
		class_destroy(ps_class);
	}

	return ret;
}
module_init(pwr_sensor_init);

static void __exit pwr_sensor_exit(void)
{
	printk(KERN_INFO "\nps_sensor: Power sensor driver - Exiting...\n");
	if(major > 0) {
		printk(KERN_INFO "ps_sensor: unregistering character device\n");
		unregister_chrdev(major, DEV_NAME);
	}

	if(!IS_ERR(ps_class)) {
		if(ps_dev) {
			printk(KERN_INFO "ps_sensor: destroying device created\n");
			device_destroy(ps_class, ps_dev_num);
		}
		printk(KERN_INFO "ps_sensor: destroying class\n");
		class_destroy(ps_class);
	}

	printk(KERN_INFO "ps_sensor: signaling the read thread to exit\n");
	atomic_set(&exit_ps_read_thread, 1);
	complete(&start);
	kthread_stop(ps_thread);
	printk(KERN_INFO "ps_sensor: Power sensor driver - Exiting...[Done]\n");
}
module_exit(pwr_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sharath Kumar Bhat");
MODULE_DESCRIPTION("Power sensor driver");

