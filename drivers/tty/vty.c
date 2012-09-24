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




MODULE_LICENSE("GPL");

#define SUCCESS 0
#define FAILURE -1
#define BUF_SIZE 38400

#define NO_OF_DEV 4
	

/**************************************************/
/*System Variables*/

#define MINOR_START_NUMBER 121
const char *tty_dev_name  = "ttty";
int order = 0;
const int reading_interval = 200;
const char tokenizer = '%';

/***************************************************/

struct tty *current_tty;



struct ring_buffer
{
	char buffer[BUF_SIZE];
	int current_pos;
};

struct ring_buffer *ring_buffer_address[NO_OF_DEV];



static int allocate_shared_memory()
{
	void * poff= (void *)0xc0000000; // 3GGB
        size_t size = 0x00600000; // 2MB
        void * virtual_address;

        unsigned long pfn = (long)poff >> PAGE_SHIFT;
        
        unsigned long node= -1, nid= -1;
        for_each_online_node(nid) {
                unsigned long start_pfn, end_pfn;
                start_pfn = node_start_pfn(nid);
                end_pfn = node_end_pfn(nid);
                if ((start_pfn <= pfn) && (pfn < end_pfn)) {
                        node = nid;
                        break; // node found continue
                }
        }

        if (node == -1) { // page never mapped (why?)
                virtual_address = ioremap_cache((resource_size_t)poff,size);
        }else { 
                struct page *shared_page;
                shared_page =  pfn_to_page(pfn);
                virtual_address = page_address(shared_page);
                void * kmap_addr = kmap(shared_page);                       

        }

		ring_buffer_address[0] = (struct ring_buffer *)virtual_address;
		ring_buffer_address[0]->current_pos = 0;
		
		int i;
		
		for(i=1;i<NO_OF_DEV;i++)
		{
			ring_buffer_address[i] = (struct ring_buffer *)(((void *)ring_buffer_address[i-1])+sizeof(struct ring_buffer));
			ring_buffer_address[i]->current_pos = 0;

		}
/*
		ring_buffer_address[1] = (struct ring_buffer *)(((void *)ring_buffer_address[0])+sizeof(struct ring_buffer));
		ring_buffer_address[1]->current_pos = 0;

		ring_buffer_address[2] = (struct ring_buffer *)(((void *)ring_buffer_address[1])+sizeof(struct ring_buffer));
		ring_buffer_address[2]->current_pos = 0;

		ring_buffer_address[3] = (struct ring_buffer *)(((void *)ring_buffer_address[2])+sizeof(struct ring_buffer));
		ring_buffer_address[3]->current_pos = 0;
*/
		return 0;
}





struct tty_driver *master_tty_driver;
struct timer_list read_function_timer;


static int tty_dev_read(void);

int tty_dev_open(struct tty_struct *tty, struct file *flip)
{
	current_tty = tty;
	mod_timer(&read_function_timer, jiffies + msecs_to_jiffies(reading_interval));
	return SUCCESS;
}


void tty_dev_close(struct tty_struct *tty, struct file *flip)
{
	del_timer(&read_function_timer);
}

static int  tty_dev_write(struct tty_struct * tty,const unsigned char *buf, int count)
{
	int index = tty->index;
	struct ring_buffer *my_ring_buf = ring_buffer_address[index];
	if (count > 0)
	{
		if (ring_buffer_address[index]->current_pos < 0 ||
				ring_buffer_address[index]->current_pos > BUF_SIZE ) {
			ring_buffer_address[index]->current_pos = 0 ;
		}
		struct ring_buffer *current_buffer =  ring_buffer_address[index];
		current_buffer->buffer[current_buffer->current_pos] = *buf;
		memcpy(&(current_buffer->buffer[current_buffer->current_pos]),
				buf, count);
		current_buffer->current_pos +=count;
		current_buffer->buffer[current_buffer->current_pos] = '\0';
	}
	return count;
}

static int tty_dev_read(void)
{
	struct ring_buffer *my_ring_buf =  ring_buffer_address[order];
	if(my_ring_buf->current_pos == 0){
		mod_timer(&read_function_timer, jiffies + msecs_to_jiffies(reading_interval));
		return 0;
	}
	struct tty_struct *tty = current_tty;
	if(tty==NULL)
	{
		printk(KERN_ALERT "TTY is null \n");
		return 0;
	}

	tty_buffer_flush(tty);
	tty_insert_flip_string(tty,my_ring_buf->buffer,my_ring_buf->current_pos);
	tty_flip_buffer_push(tty);
	memset(ring_buffer_address[order]->buffer,'\0',strlen(ring_buffer_address[order]->buffer));
	ring_buffer_address[order]->current_pos = 0;
	mod_timer(&read_function_timer, jiffies + msecs_to_jiffies(reading_interval));
	return 1;

}


static int tty_dev_write_room(struct tty_struct *tty){

	return 255;
}

static struct tty_operations tty_dev_operations = {

	.open	=	tty_dev_open,
	.close	=	tty_dev_close,
	.write	=	tty_dev_write,
	.write_room = tty_dev_write_room
};


static int __init vty_init(void)
{
	printk(KERN_ALERT "Loading shared memory %s",__func__);
	order = smp_processor_id();
	printk(KERN_ALERT "The order is %d\n",order);
	
	allocate_shared_memory();
	printk(KERN_ALERT "Memory Allocation is successful\n");



	printk(KERN_ALERT "My TTY driver Module is loading\n");	
	master_tty_driver = alloc_tty_driver(NO_OF_DEV);
	if(!master_tty_driver){
		printk(KERN_ALERT "Allocation of master is failed");
		return FAILURE;
	}
	
	master_tty_driver->owner = THIS_MODULE;
	master_tty_driver->driver_name="Myttydriver";
	master_tty_driver->name = tty_dev_name;	
	master_tty_driver->major = TTY_MAJOR;
	master_tty_driver->minor_start =  MINOR_START_NUMBER;
	master_tty_driver->num =  NO_OF_DEV;
	master_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	master_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	master_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS ;
	master_tty_driver->init_termios =  tty_std_termios;
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

	int i;
	for(i = 0 ;i < NO_OF_DEV ; ++i){
		tty_register_device(master_tty_driver,i+ MINOR_START_NUMBER,NULL);
	}
    setup_timer(&read_function_timer,tty_dev_read,0);
	return SUCCESS;
}

static void __exit vty_exit(void)
{
	printk(KERN_ALERT "Unloading shared memory\n");
}

module_init(vty_init);
module_exit(vty_exit);
