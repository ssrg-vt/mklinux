#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/string.h>
#include<linux/spinlock.h>
#include<linux/highmem.h>
#include<linux/mm.h>
#include<asm/io.h>
#include<linux/sched.h>
#include<linux/tty.h>
#include<linux/tty_driver.h>
#include<linux/tty_flip.h>
#include<linux/device.h>
#include<linux/major.h>
#include<linux/timer.h>
#include<linux/spinlock.h>
#include<linux/types.h>

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define FAILURE -1
#define BUF_SIZE 19200

#define NO_OF_DEV 64

/**************************************************/
/*System Variables*/

#define MINOR_START_NUMBER 121
const char *tty_dev_name = "ttty";
int order = 0;
const int reading_interval = 200;
const char tokenizer = '%';

/***************************************************/

unsigned long long global_poff = 0l;

typedef struct vty_desc {
	struct tty_struct * tty;
	int id;
	struct list_head list;
} vty_desc_t;

int index = 0;
struct list_head current_tty;

static int __init _setup_vty_offset(char *str)
{
	global_poff = simple_strtoull(str,0,16);
	printk(KERN_ALERT "VTY offset %llx\n",global_poff);
	return 0;
}
early_param("vty_offset", _setup_vty_offset);

struct ring_buffer {
	char buffer[BUF_SIZE];
	int current_pos;
	rwlock_t lock;
};

struct ring_buffer *ring_buffer_address[NO_OF_DEV][NO_OF_DEV];

static int allocate_shared_memory() {

	unsigned long *poff = 0l;
	if (global_poff == 0) {
		poff = 0xc0000000;
	} else {
		poff = (unsigned long *) global_poff;
	}

	size_t size = 0x200000;
	void *virtual_address[64];

	printk(KERN_ALERT "Poff %llx and %ld\n",poff,size);
	unsigned long pfn = (long) poff >> PAGE_SHIFT;
	unsigned long node = -1, nid = -1;
	for_each_online_node(nid)
	{
		unsigned long start_pfn, end_pfn;
		start_pfn = node_start_pfn(nid);
		end_pfn = node_end_pfn(nid);
		if ((start_pfn <= pfn) && (pfn < end_pfn)) {
			node = nid;
			break; // node found continue
		}
	}

	int i;
	for (i = 0; i < 64; ++i) {
		if (node == -1) { // page never mapped (why?)
			virtual_address[i] = ioremap_cache(
					(resource_size_t)((void *) poff + (i * size)), size);
		} else {
			struct page *shared_page;
			shared_page = pfn_to_page(pfn);
			virtual_address[i] = page_address(shared_page);
			void * kmap_addr = kmap(shared_page);

		}
	}

	int j;
	for (i = 0; i < 64; ++i) {
		void *p = virtual_address[i];
		for (j = 0; j < 64; ++j) {
			ring_buffer_address[i][j] = (struct ring_buffer *) ((void *) p
					+ (sizeof(struct ring_buffer) * j));
				ring_buffer_address[i][j]->current_pos = 0;
				rwlock_init(&(ring_buffer_address[i][j]->lock));
		}
	}

	return 0;
}

struct tty_driver *master_tty_driver;
struct timer_list read_function_timer;

static void tty_dev_read(long unsigned int time);

int tty_dev_open(struct tty_struct *tty, struct file *flip) {

	vty_desc_t *tmp = (vty_desc_t *) kmalloc(sizeof(vty_desc_t), GFP_KERNEL);
	tmp->tty = tty;
	tmp->id = index++;
	INIT_LIST_HEAD(&(tmp->list));
	tty->driver_data = tmp->id;
	list_add(&(tmp->list), &(current_tty));

	mod_timer(&read_function_timer,	jiffies + msecs_to_jiffies(reading_interval));
	return SUCCESS;
}

void tty_dev_close(struct tty_struct *tty, struct file *flip) {

	struct list_head * cur, *n;
	vty_desc_t * curt;

	list_for_each_safe(cur, n, &current_tty)
	{
		curt = list_entry(cur, vty_desc_t, list);

		if (curt->tty == tty && curt->id == (int) tty->driver_data) {
			list_del(cur);
			kfree(curt);
			return 0;
		}
	}

	del_timer(&read_function_timer);
}

int tty_dev_write(struct tty_struct * tty, const unsigned char *buf, int count) {
	/**
	 * When 0 wants to write to 2 it will write in 2,0
	 * 2 wants to read what is written by 0, it will read 2,0
	 * */
	int xGrid = tty->index;
	int yGrid = order;
	if (count > 0 && (ring_buffer_address[xGrid][yGrid] != NULL)) {
		write_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
		if (ring_buffer_address[xGrid][yGrid]->current_pos
				< 0||
				ring_buffer_address[xGrid][yGrid]->current_pos >= BUF_SIZE || count > BUF_SIZE) {ring_buffer_address[xGrid][yGrid]->current_pos = 0;
		printk(KERN_ALERT "Memory Overflow...........\n Resetting the value.....\n");
		if(count > BUF_SIZE) {
			count = BUF_SIZE;
		}
	}
		struct ring_buffer *current_buffer = ring_buffer_address[xGrid][yGrid];
		memcpy(&(current_buffer->buffer[current_buffer->current_pos]), buf,
				count);
		current_buffer->current_pos += count;
		current_buffer->buffer[current_buffer->current_pos] = '\0';
		write_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
	}
	return count;
}

void tty_dev_read(long unsigned int time) {

	struct list_head * cur, *n;
	vty_desc_t * curt;

	list_for_each_safe(cur, n, &current_tty)	
	{
		curt = list_entry(cur, vty_desc_t, list);

		int xGrid = order;
		int yGrid = curt->tty->index;
		struct ring_buffer *my_ring_buf = ring_buffer_address[xGrid][yGrid];
		if (my_ring_buf != NULL) {
			read_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
			if (my_ring_buf->current_pos == 0) {
				read_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
				mod_timer(&read_function_timer,
						jiffies + msecs_to_jiffies(reading_interval));
				return;
			}

			tty_insert_flip_string(curt->tty, my_ring_buf->buffer,
					my_ring_buf->current_pos);
			tty_flip_buffer_push(curt->tty);
			read_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
			write_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
			memset(ring_buffer_address[xGrid][yGrid]->buffer, '\0',
					ring_buffer_address[xGrid][yGrid]->current_pos);
			ring_buffer_address[xGrid][yGrid]->current_pos = 0;
			write_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));

		}

	}
	//unlock current_tty
	mod_timer(&read_function_timer,
			jiffies + msecs_to_jiffies(reading_interval));
	return;

}

static int tty_dev_write_room(struct tty_struct *tty) {
	int xGrid = tty->index;
	int yGrid = order;
	int retVal = BUF_SIZE;
	if (ring_buffer_address[xGrid][yGrid] != NULL) {
		read_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
		retVal = BUF_SIZE - ring_buffer_address[xGrid][yGrid]->current_pos;
		read_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
	}
	return retVal;
}

static struct tty_operations tty_dev_operations = { .open = tty_dev_open,
		.close = tty_dev_close, .write = tty_dev_write, .write_room =
				tty_dev_write_room };

static int __init vty_init(void)
{
	printk(KERN_ALERT "Loading MyTTy Driver memory %s\n",__func__);
	order = smp_processor_id();
	printk(KERN_ALERT "The order is %d\n",order);

	allocate_shared_memory();
	printk(KERN_ALERT "Memory Allocation is successful\n");

	printk(KERN_ALERT "My TTY driver Module is loading\n");
	master_tty_driver = alloc_tty_driver(NO_OF_DEV);
	if(!master_tty_driver) {
		printk(KERN_ALERT "Allocation of master is failed\n");
		return FAILURE;
	}

	master_tty_driver->owner = THIS_MODULE;
	master_tty_driver->driver_name="Myttydriver";
	master_tty_driver->name = tty_dev_name;
	master_tty_driver->major = TTY_MAJOR;
	master_tty_driver->minor_start = MINOR_START_NUMBER;
	master_tty_driver->num = NO_OF_DEV;
	master_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	master_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	master_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS;
	master_tty_driver->init_termios = tty_std_termios;
	master_tty_driver->init_termios = tty_std_termios;

	master_tty_driver->init_termios.c_iflag = 0;
	master_tty_driver->init_termios.c_oflag = 0;
	master_tty_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	master_tty_driver->init_termios.c_lflag = 0;
	master_tty_driver->init_termios.c_ispeed = 38400;
	master_tty_driver->init_termios.c_ospeed = 38400;
	tty_set_operations(master_tty_driver, &tty_dev_operations);

	int retval = tty_register_driver(master_tty_driver);
	if(retval != 0)
	{
		printk(KERN_ALERT "Unable to register the device\n");
		return FAILURE;
	}
	setup_timer(&read_function_timer,tty_dev_read,0);

	INIT_LIST_HEAD(&current_tty);

	return SUCCESS;
}

static void __exit vty_exit(void)
{
	printk(KERN_ALERT "Unloading shared memory\n");
}

module_init( vty_init);
module_exit( vty_exit);
