/*
 * Copyright 2012-2014, SSRG VT
 * original version: Arijit Chattopardiyay
 * rewritten by: Antonio Barbalace
 */
// TODO rewrite the buffering algorithm

#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/string.h>
#include<linux/spinlock.h>
#include<linux/highmem.h>
#include<linux/mm.h>
#include<linux/sched.h>
#include<linux/tty.h>
#include<linux/tty_driver.h>
#include<linux/tty_flip.h>
#include<linux/device.h>
#include<linux/major.h>
#include<linux/timer.h>
#include<linux/spinlock.h>
#include<linux/types.h>

#include<asm/io.h>
#include<asm/bug.h>

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define FAILURE -1

/*
 * configurables
 */
#define NO_OF_DEV 64
#define SHM_SIZE 0x200000
#define BUF_SIZE ((SHM_SIZE/NO_OF_DEV) - PAGE_SIZE)
#define READING_INTERVAL 125

#if ( (SHM_SIZE & ~PAGE_MASK) )
# error "size must be a multiple of the architecture page size"
#endif
#if ( (NO_OF_DEV * (BUF_SIZE + PAGE_SIZE)) > SHM_SIZE )
# error "(NO_OF_DEV * (BUF_SIZE + PAGE_SIZE)) exceeds SHM_SIZE"
#endif

/*
 * constants
 */
#define MINOR_START_NUMBER 121
#define VTY_DEV_NAM "vty"

/*
 * System Variables
 */
static int order = 0;
static const char *tty_dev_name = VTY_DEV_NAM;
static const int reading_interval = READING_INTERVAL;
static const char tokenizer = '%';
static unsigned long long global_poff = 0l;

typedef struct vty_desc {
	struct tty_struct * tty;
	int id;
	struct list_head list;
} vty_desc_t;

struct list_head current_tty;

/*
 * _setup_vty_offset
 * 
 * fetching the command line argument for configuration.
 * if the configuration argument is missing the driver will not be loaded.
 */
static int __init _setup_vty_offset(char *str)
{
	global_poff = simple_strtoull(str, 0, 16);
	return 0;
}
early_param("vty_offset", _setup_vty_offset);

/*
 * allocate_shared_memory
 * 
 * it maps a physical shared memory area as a matrix communication buffer.
 * The matrix is NO_OF_DEV*NO_OF_DEV. There is a buffer for each communication
 * direction.
 */
struct ring_buffer {
	char buffer[BUF_SIZE];
	int current_pos;
	rwlock_t lock;
};
static struct ring_buffer *ring_buffer_address[NO_OF_DEV][NO_OF_DEV];

static int allocate_shared_memory (void)
{
	int i, j;
	void *poff = 0l;
	void *virtual_address[NO_OF_DEV];
	unsigned long pfn, pfn_end, node, nid;
	size_t size = SHM_SIZE;
	
	// fetching the pysical address (in shared memory)
	if (global_poff == 0)
	  return -ENOMEM;
	
	poff = (void*) global_poff;
	pfn = (unsigned long) poff >> PAGE_SHIFT;
	pfn_end = ((unsigned long) poff + ((unsigned long) size * NO_OF_DEV)) >> PAGE_SHIFT;
	
	//buffers do not have to overlap
	BUG_ON( (sizeof(struct ring_buffer) > (BUF_SIZE + PAGE_SIZE)) );
	
	// finding the current node id
	node = nid = -1;
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
	printk(KERN_INFO"virtualTTY: buffer %ld, rows %ld, columns(cpus) %d, node %ld[0x%lx-0x%lx]\n",
	       sizeof(struct ring_buffer), (long) size, (int)NO_OF_DEV, node,
	       (node!=-1)?node_start_pfn(node):0l, (node!=-1)?node_end_pfn(node):0l);
	
	// mapping in all the matrix of possible buffers (per rows)
	for (i = 0; i < NO_OF_DEV; i++) {
		if (node == -1) { // page never mapped
			virtual_address[i] = ioremap_cache(
					(resource_size_t)((void *) poff + (i * size)), size);
			if (! virtual_address[i])
				printk(KERN_ERR"%s: ioremap failed, virtual_address %d is 0x%p\n",
					__func__, i, virtual_address[i]);
		} else {
			struct page *shared_page;
			shared_page = pfn_to_page(pfn + (size >> PAGE_SHIFT)*i);
			virtual_address[i] = page_address(shared_page);
		}
	}

	// mapping each buffer and initialize it
	for (i = 0; i < NO_OF_DEV; i++) {
		void *p = virtual_address[i];
		for (j = 0; j < NO_OF_DEV; j++) {
			ring_buffer_address[i][j] = (struct ring_buffer *) ((void *) p
					+ ((BUF_SIZE + PAGE_SIZE) * j));
			
			ring_buffer_address[i][j]->current_pos = 0;
			rwlock_init(&(ring_buffer_address[i][j]->lock));
		}
	}

	return 0;
}

struct tty_driver *master_tty_driver;
struct timer_list read_function_timer;

static void tty_dev_read(long unsigned int time);

int tty_dev_open(struct tty_struct *tty, struct file *flip)
{

	vty_desc_t *tmp = (vty_desc_t *) kmalloc(sizeof(vty_desc_t), GFP_KERNEL);
	tmp->tty = tty;
	tmp->id = tty->index;
	INIT_LIST_HEAD(&(tmp->list));
	tty->driver_data = (void*)(long)tmp->id;
	list_add(&(tmp->list), &(current_tty));

//printk(KERN_ALERT"vty_dev_open current %s cpu %d (tty %p id %d) order %d\n",
//	current->comm, smp_processor_id(), tmp->tty, tmp->id, order);

	mod_timer(&read_function_timer,	jiffies + msecs_to_jiffies(reading_interval));
	return SUCCESS;
}

void tty_dev_close(struct tty_struct *tty, struct file *flip)
{

	struct list_head * cur, *n;
	vty_desc_t * curt;

//printk(KERN_ALERT"vty_dev_close current %s cpu %d (tty %p id %d) order %d\n",
  //      current->comm, smp_processor_id(), tty, tty->index, order);

	list_for_each_safe(cur, n, &current_tty)
	{
		curt = list_entry(cur, vty_desc_t, list);

		if (curt->tty == tty &&
		    curt->id == (int)(long)tty->driver_data) {
//printk(KERN_ALERT"VTY removing %p %d\n", curt->tty, curt->id);
			list_del(cur);
			kfree(curt);
			return;
		}
	}

//	del_timer(&read_function_timer);
}

int tty_dev_write(struct tty_struct * tty, const unsigned char *buf, int count)
{
	/**
	 * When 0 wants to write to 2 it will write in 2,0
	 * 2 wants to read what is written by 0, it will read 2,0
	 * */
	int xGrid = tty->index;
	int yGrid = order;
int pos;
	struct ring_buffer *current_buffer;

        struct list_head * cur, *n;
        vty_desc_t * curt;
	int exists =0;
        list_for_each_safe(cur, n, &current_tty)
        {
                curt = list_entry(cur, vty_desc_t, list);
		if (curt->tty == tty &&
		    curt->id == (int)(long)tty->driver_data) 
			exists++;
	}
	
	if ( (count > 0) &&
	    (ring_buffer_address[xGrid][yGrid] != NULL) ) {
	  
		write_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
		if (ring_buffer_address[xGrid][yGrid]->current_pos < 0 ||
				ring_buffer_address[xGrid][yGrid]->current_pos >= BUF_SIZE ||
				count > BUF_SIZE) {
		  
			ring_buffer_address[xGrid][yGrid]->current_pos = 0;
			printk(KERN_ALERT "Memory Overflow...........\n Resetting the value.....\n");
			if(count > BUF_SIZE) {
				count = BUF_SIZE;
			}
		}
		current_buffer = ring_buffer_address[xGrid][yGrid];
		memcpy(&(current_buffer->buffer[current_buffer->current_pos]), buf,
				count);
		current_buffer->current_pos += count;
pos = current_buffer->current_pos;
		current_buffer->buffer[current_buffer->current_pos] = '\0';
		
		write_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
	}

//printk(KERN_ALERT"vty_dev_write current %s cpu %d (tty %p id %d) order %d count %d pos %d exists %d\n",
//        current->comm, smp_processor_id(), tty, tty->index, order, count, pos, exists);

	return count;
}

void tty_dev_read(long unsigned int time)
{

	struct list_head * cur, *n;
	vty_desc_t * curt;

static int counter =0;
        counter++;

	list_for_each_safe(cur, n, &current_tty)	
	{
		int xGrid, yGrid, pos;
		struct ring_buffer *my_ring_buf;

		curt = list_entry(cur, vty_desc_t, list);
		xGrid = order;
		yGrid = curt->tty->index;
		my_ring_buf = ring_buffer_address[xGrid][yGrid];

/*if ((counter % 1024) == 1)
        printk(KERN_ALERT"%s: x%d y%d p%p pos%d\n",
		 __func__, xGrid, yGrid, my_ring_buf,
		my_ring_buf ? my_ring_buf->current_pos : -1);
*/
		if (my_ring_buf != NULL) {
			read_lock(&(my_ring_buf->lock));
			if (my_ring_buf->current_pos == 0) {
				read_unlock(&(my_ring_buf->lock));
				//mod_timer(&read_function_timer,
				//		jiffies + msecs_to_jiffies(reading_interval));
				continue;
			}

			tty_insert_flip_string(curt->tty, my_ring_buf->buffer,
					pos = my_ring_buf->current_pos);
			tty_flip_buffer_push(curt->tty);
			read_unlock(&(my_ring_buf->lock));

			write_lock(&(my_ring_buf->lock));
			memset(my_ring_buf->buffer, '\0', my_ring_buf->current_pos);
			my_ring_buf->current_pos = 0;
			write_unlock(&(my_ring_buf->lock));

//printk(KERN_ALERT"vty_dev_read current %s cpu %d (tty %p id %d) order %d pos %d\n",
//        current->comm, smp_processor_id(), curt->tty, curt->tty->index, order, pos);
		}
	}

	//unlock current_tty
	mod_timer(&read_function_timer,
			jiffies + msecs_to_jiffies(reading_interval));
	return;

}

static int tty_dev_write_room(struct tty_struct *tty)
{
	int xGrid = tty->index;
	int yGrid = order;
	int retVal = BUF_SIZE;
	if (ring_buffer_address[xGrid][yGrid] != NULL) {
		read_lock(&(ring_buffer_address[xGrid][yGrid]->lock));
		retVal = BUF_SIZE - ring_buffer_address[xGrid][yGrid]->current_pos;
		read_unlock(&(ring_buffer_address[xGrid][yGrid]->lock));
	}

//printk(KERN_ALERT"vty_dev_write_room current %s cpu %d (tty %p id %d) order %d ret %d\n",
//        current->comm, smp_processor_id(), tty, tty->index, order, retVal);

	return retVal;
}

static struct tty_operations tty_dev_operations = {
	.open = tty_dev_open,
	.close = tty_dev_close,
	.write = tty_dev_write,
	.write_room = tty_dev_write_room
};

static int __init vty_init(void)
{
	int ret;
	order = smp_processor_id();
	printk(KERN_INFO "virtualTTY: cpu %d, phys 0x%llx-0x%llx\n",
	       order, global_poff, global_poff + (SHM_SIZE * NO_OF_DEV));

	if ( (ret = allocate_shared_memory()) ) {
	  printk(KERN_ERR "%s: allocate_shared_memory error %d\n",
		 __func__, ret);
	  return FAILURE;
	}

	master_tty_driver = alloc_tty_driver(NO_OF_DEV);
	if( !master_tty_driver ) {
	  printk(KERN_ERR "%s: allocation of master tty failed %p\n",
		__func__, master_tty_driver);
	  return FAILURE;
	}

	master_tty_driver->owner = THIS_MODULE;
	master_tty_driver->driver_name="vtydriver";
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

	ret = tty_register_driver(master_tty_driver);
	if(ret != 0) {
		printk(KERN_ERR "%s: unable to register the tty device %d\n",
		  __func__, ret);
		return FAILURE;
	}
	setup_timer(&read_function_timer, tty_dev_read, 0);

	INIT_LIST_HEAD(&current_tty);

	return SUCCESS;
}

static void __exit vty_exit(void)
{
	printk(KERN_INFO "virtualTTY Driver: unloading\n");
	// TODO not implemented
}

module_init( vty_init);
module_exit( vty_exit);
