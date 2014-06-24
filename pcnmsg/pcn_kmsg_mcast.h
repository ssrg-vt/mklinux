
/*
 * MULTICAST support routine
 */

/* Same thing, but for mcast windows */
struct pcn_kmsg_mcast_local mcastlocal[POPCORN_MAX_MCAST_CHANNELS];


#define MCASTWIN(_id_) (mcastlocal[(_id_)].mcastvirt)
#define LOCAL_TAIL(_id_) (mcastlocal[(_id_)].local_tail)

/* MULTICAST RING BUFFER */
static inline unsigned long mcastwin_inuse(pcn_kmsg_mcast_id id)
{
	return MCASTWIN(id)->head - MCASTWIN(id)->tail;
}

static inline int mcastwin_put(pcn_kmsg_mcast_id id,
			       struct pcn_kmsg_message *msg)
{
	unsigned long ticket;
	unsigned long time_limit = jiffies + 2;


	MCAST_PRINTK("called for id %lu, msg 0x%p\n", id, msg);

	/* if the queue is already really long, return EAGAIN */
	if (mcastwin_inuse(id) >= RB_SIZE) {
		MCAST_PRINTK("window full, caller should try again...\n");
		return -EAGAIN;
	}

	/* grab ticket */
	ticket = fetch_and_add(&MCASTWIN(id)->head, 1);
	MCAST_PRINTK("ticket = %lu, head = %lu, tail = %lu\n",
		     ticket, MCASTWIN(id)->head, MCASTWIN(id)->tail);

	/* spin until there's a spot free for me */
	while (mcastwin_inuse(id) >= RB_SIZE) {
		if (unlikely(time_after(jiffies, time_limit))) {
			MCAST_PRINTK("spinning too long to wait for window to be free; this is bad!\n");
			return -1;
		}
	}

	/* insert item */
	memcpy(&MCASTWIN(id)->buffer[ticket & RB_MASK].payload, 
	       &msg->payload, PCN_KMSG_PAYLOAD_SIZE);

	memcpy((void*)&(MCASTWIN(id)->buffer[ticket & RB_MASK].hdr), 
	       (void*)&(msg->hdr), sizeof(struct pcn_kmsg_hdr));

	/* set counter to (# in group - self) */

	/*
	int x;

	if ((x = atomic_read(&MCASTWIN(id)->read_counter[ticket & RB_MASK]))) {
		KMSG_ERR("read counter is not zero (it's %d)\n", x);
		return -1;
	}
	*/

	atomic_set(&MCASTWIN(id)->read_counter[ticket & RB_MASK],
		rkinfo->mcast_wininfo[id].num_members - 1);

	MCAST_PRINTK("set counter to %d\n", 
		     rkinfo->mcast_wininfo[id].num_members - 1);

	pcn_barrier();

	/* set completed flag */
	MCASTWIN(id)->buffer[ticket & RB_MASK].ready = 1;

	return 0;
}

static inline int mcastwin_get(pcn_kmsg_mcast_id id,
			       struct pcn_kmsg_reverse_message **msg)
{
	struct pcn_kmsg_reverse_message *rcvd;

	MCAST_PRINTK("called for id %lu, head %lu, tail %lu, local_tail %lu\n", 
		     id, MCASTWIN(id)->head, MCASTWIN(id)->tail, 
		     LOCAL_TAIL(id));

retry:

	/* if we sent a bunch of messages, it's possible our local_tail
	   has gotten behind the global tail and we need to update it */
	/* TODO -- atomicity concerns here? */
	if (LOCAL_TAIL(id) < MCASTWIN(id)->tail) {
		LOCAL_TAIL(id) = MCASTWIN(id)->tail;
	}

	if (MCASTWIN(id)->head == LOCAL_TAIL(id)) {
		MCAST_PRINTK("nothing in buffer, returning...\n");
		return -1;
	}

	/* spin until entry.ready at end of cache line is set */
	rcvd = &(MCASTWIN(id)->buffer[LOCAL_TAIL(id) & RB_MASK]);
	while (!rcvd->ready) {
		pcn_cpu_relax();
	}

	// barrier here?
	pcn_barrier();

	/* we can't step on our own messages! */
	if (rcvd->hdr.from_cpu == my_cpu) {
		LOCAL_TAIL(id)++;
		goto retry;
	}

	*msg = rcvd;

	return 0;
}

static inline void mcastwin_advance_tail(pcn_kmsg_mcast_id id)
{
	unsigned long slot = LOCAL_TAIL(id) & RB_MASK;
	//int val_ret, i;
	//char printstr[256];
	//char intstr[16];

	MCAST_PRINTK("local tail currently on slot %lu, read counter %d\n", 
		     LOCAL_TAIL(id), atomic_read(&MCASTWIN(id)->read_counter[slot]));

	/*
	memset(printstr, 0, 256);
	memset(intstr, 0, 16);

	for (i = 0; i < 64; i++) {
		sprintf(intstr, "%d ", atomic_read(&MCASTWIN(id)->read_counter[i]));
		strcat(printstr, intstr);
	}

	MCAST_PRINTK("read_counter: %s\n", printstr);

	val_ret = atomic_add_return_sync(-1, &MCASTWIN(id)->read_counter[slot]);

	MCAST_PRINTK("read counter after: %d\n", val_ret);
	*/

	if (atomic_dec_and_test_sync(&MCASTWIN(id)->read_counter[slot])) {
		MCAST_PRINTK("we're the last reader to go; ++ global tail\n");
		MCASTWIN(id)->buffer[slot].ready = 0;
		atomic64_inc((atomic64_t *) &MCASTWIN(id)->tail);
	}

	LOCAL_TAIL(id)++;
}

static void map_mcast_win(pcn_kmsg_work_t *w)
{
	pcn_kmsg_mcast_id id = w->id_to_join;

	/* map window */
	if (id < 0 || id > POPCORN_MAX_MCAST_CHANNELS) {
		KMSG_ERR("%s: invalid mcast channel id %lu specified!\n",
			 __func__, id);
		return;
	}

	MCASTWIN(id) = ioremap_cache(rkinfo->mcast_wininfo[id].phys_addr,
				     sizeof(struct pcn_kmsg_mcast_window));
	if (MCASTWIN(id)) {
		MCAST_PRINTK("ioremapped mcast window, virt addr 0x%p\n",
			     MCASTWIN(id));
	} else {
		KMSG_ERR("Failed to map mcast window %lu at phys addr 0x%lx\n",
			 id, rkinfo->mcast_wininfo[id].phys_addr);
	}
}

static inline int pcn_kmsg_mcast_window_init(struct pcn_kmsg_mcast_window *win)
{
	int i;

	win->head = 0;
	win->tail = 0;

	for (i = 0; i < PCN_KMSG_RBUF_SIZE; i++) {
		atomic_set(&win->read_counter[i], 0);
	}

	//memset(&win->read_counter, 0, 
	//       PCN_KMSG_RBUF_SIZE * sizeof(int));
	memset(&win->buffer, 0,
	       PCN_KMSG_RBUF_SIZE * sizeof(struct pcn_kmsg_message));
	return 0;
}

static void process_mcast_queue(pcn_kmsg_mcast_id id)
{
	struct pcn_kmsg_reverse_message *msg;
	while (!mcastwin_get(id, &msg)) {
		MCAST_PRINTK("Got an mcast message, type %d!\n",
			     msg->hdr.type);

		/* Special processing for large messages */
                if (msg->hdr.is_lg_msg) {
                        MCAST_PRINTK("message is a large message!\n");
                        process_large_message(msg);
                } else {
                        MCAST_PRINTK("message is a small message!\n");
                        process_small_message(msg);
                }

		mcastwin_advance_tail(id);
	}

}

inline void lock_chan(pcn_kmsg_mcast_id id)
{
}

inline void unlock_chan(pcn_kmsg_mcast_id id)
{
}

inline int count_members(unsigned long mask)
{
	int i, count = 0;

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (mask & (1ULL << i)) {
			count++;
		}
	}

	return count;
}

void print_mcast_map(void)
{
#if MCAST_VERBOSE
	int i;

	printk("ACTIVE MCAST GROUPS:\n");

	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (rkinfo->mcast_wininfo[i].mask) {
			printk("group %d, mask 0x%lx, num_members %d\n", 
			       i, rkinfo->mcast_wininfo[i].mask, 
			       rkinfo->mcast_wininfo[i].num_members);
		}
	}
	return;
#endif
}

/* Open a multicast group containing the CPUs specified in the mask. */
int pcn_kmsg_mcast_open(pcn_kmsg_mcast_id *id, unsigned long mask)
{
	int rc, i, found_id;
	struct pcn_kmsg_mcast_message msg;
	struct pcn_kmsg_mcast_wininfo *slot;
	struct pcn_kmsg_mcast_window * new_win;

	MCAST_PRINTK("Reached pcn_kmsg_mcast_open, mask 0x%lx\n", mask);

	if (!(mask & (1 << my_cpu))) {
		KMSG_ERR("This CPU is not a member of the mcast group to be created, cpu %d, mask 0x%lx\n",
			 my_cpu, mask);
		return -1;
	}

	/* find first unused channel */
retry:
	found_id = -1;

	for (i = 0; i < POPCORN_MAX_MCAST_CHANNELS; i++) {
		if (!rkinfo->mcast_wininfo[i].num_members) {
			found_id = i;
			break;
		}
	}

	MCAST_PRINTK("Found channel ID %d\n", found_id);

	if (found_id == -1) {
		KMSG_ERR("No free multicast channels!\n");
		return -1;
	}

	/* lock and check if channel is still unused; 
	   otherwise, try again */
	lock_chan(found_id);

	if (rkinfo->mcast_wininfo[i].num_members) {
		unlock_chan(found_id);
		MCAST_PRINTK("Got scooped; trying again...\n");
		goto retry;
	}

	/* set slot info */
	slot = &rkinfo->mcast_wininfo[found_id];
	slot->mask = mask;
	slot->num_members = count_members(mask);
	slot->owner_cpu = my_cpu;

	MCAST_PRINTK("Found %d members\n", slot->num_members);

	/* kmalloc window for slot */
	new_win = kmalloc(sizeof(struct pcn_kmsg_mcast_window), GFP_ATOMIC);

	if (!new_win) {
		KMSG_ERR("Failed to kmalloc mcast buffer!\n");
		goto out;
	}

	/* zero out window */
	memset(new_win, 0x0, sizeof(struct pcn_kmsg_mcast_window));

	MCASTWIN(found_id) = new_win;
	slot->phys_addr = virt_to_phys(new_win);
	MCAST_PRINTK("Malloced mcast receive window %d at phys addr 0x%lx\n",
		     found_id, slot->phys_addr);

	/* send message to each member except self.  Can't use mcast yet because
	   group is not yet established, so unicast to each CPU in mask. */
	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_OPEN;
	msg.id = found_id;
	msg.mask = mask;
	msg.num_members = slot->num_members;

	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if ((slot->mask & (1ULL << i)) && 
		    (my_cpu != i)) {
			MCAST_PRINTK("Sending message to CPU %d\n", i);

			rc = pcn_kmsg_send(i, (struct pcn_kmsg_message *) &msg);

			if (rc) {
				KMSG_ERR("Message send failed!\n");
			}
		}
	}

	*id = found_id;

out:
	unlock_chan(found_id);

	return 0;
}

/* Add new members to a multicast group. */
int pcn_kmsg_mcast_add_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	lock_chan(id);

	KMSG_ERR("Operation not yet supported!\n");

	//rkinfo->mcast_wininfo[id].mask |= mask; 

	/* TODO -- notify new members */

	unlock_chan(id);
	return 0;
}

/* Remove existing members from a multicast group. */
int pcn_kmsg_mcast_delete_members(pcn_kmsg_mcast_id id, unsigned long mask)
{
	lock_chan(id);

	KMSG_ERR("Operation not yet supported!\n");

	//rkinfo->mcast_wininfo[id].mask &= !mask;

	/* TODO -- notify new members */

	unlock_chan(id);

	return 0;
}

inline int pcn_kmsg_mcast_close_notowner(pcn_kmsg_mcast_id id)
{
	MCAST_PRINTK("Closing multicast channel %lu on CPU %d\n", id, my_cpu);

	/* process remaining messages in queue (should there be any?) */

	/* remove queue from list of queues being polled */
	iounmap(MCASTWIN(id));

	MCASTWIN(id) = NULL;

	return 0;
}

/* Close a multicast group. */
int pcn_kmsg_mcast_close(pcn_kmsg_mcast_id id)
{
	int rc;
	struct pcn_kmsg_mcast_message msg;
	struct pcn_kmsg_mcast_wininfo *wi = &rkinfo->mcast_wininfo[id];

	if (wi->owner_cpu != my_cpu) {
		KMSG_ERR("Only creator (cpu %d) can close mcast group %lu!\n",
			 wi->owner_cpu, id);
		return -1;
	}

	lock_chan(id);

	/* set window to close */
	wi->is_closing = 1;

	/* broadcast message to close window globally */
	msg.hdr.type = PCN_KMSG_TYPE_MCAST;
	msg.hdr.prio = PCN_KMSG_PRIO_HIGH;
	msg.type = PCN_KMSG_MCAST_CLOSE;
	msg.id = id;

	rc = pcn_kmsg_mcast_send(id, (struct pcn_kmsg_message *) &msg);
	if (rc) {
		KMSG_ERR("failed to send mcast close message!\n");
		return -1;
	}

	/* wait until global_tail == global_head */
	while (MCASTWIN(id)->tail != MCASTWIN(id)->head) {}

	/* free window and set channel as unused */
	kfree(MCASTWIN(id));
	MCASTWIN(id) = NULL;

	wi->mask = 0;
	wi->num_members = 0;
	wi->is_closing = 0;

	unlock_chan(id);

	return 0;
}

unsigned long mcast_ipi_ts;

static int __pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, 
				 struct pcn_kmsg_message *msg)
{
	int i, rc;

	if (!msg) {
		KMSG_ERR("Passed in a null pointer to msg!\n");
		return -1;
	}

	/* set source CPU */
	msg->hdr.from_cpu = my_cpu;

	/* place message in rbuf */
	rc = mcastwin_put(id, msg);

	if (rc) {
		KMSG_ERR("failed to place message in mcast window -- maybe it's full?\n");
		return -1;
	}

	rdtscll(mcast_ipi_ts);

	/* send IPI to all in mask but me */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[id].mask & (1ULL << i)) {
			if (i != my_cpu) {
				MCAST_PRINTK("sending IPI to CPU %d\n", i);
				apic->send_IPI_single(i, POPCORN_KMSG_VECTOR);
			}
		}
	}

	return 0;
}

#define MCAST_HACK 0

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send(pcn_kmsg_mcast_id id, struct pcn_kmsg_message *msg)
{
#if MCAST_HACK

	int i, rc;

	MCAST_PRINTK("Sending mcast message, id %lu\n", id);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send(i, msg);

			if (rc) {
				KMSG_ERR("Batch send failed to CPU %d\n", i);
				return -1;
			}
		}
	}

	return 0;
#else
	int rc;

	MCAST_PRINTK("sending mcast message to group id %lu\n", id);

	msg->hdr.is_lg_msg = 0;
	msg->hdr.lg_start = 0;
	msg->hdr.lg_end = 0;
	msg->hdr.lg_seqnum = 0;

	rc = __pcn_kmsg_mcast_send(id, msg);

	return rc;
#endif
}

/* Send a message to the specified multicast group. */
int pcn_kmsg_mcast_send_long(pcn_kmsg_mcast_id id, 
			     struct pcn_kmsg_long_message *msg, 
			     unsigned int payload_size)
{
#if MCAST_HACK
	int i, rc;

	MCAST_PRINTK("Sending long mcast message, id %lu, size %u\n", 
		     id, payload_size);

	/* quick hack for testing for now; 
	   loop through mask and send individual messages */
	for (i = 0; i < POPCORN_MAX_CPUS; i++) {
		if (rkinfo->mcast_wininfo[id].mask & (0x1 << i)) {
			rc = pcn_kmsg_send_long(i, msg, payload_size);

			if (rc) {
				KMSG_ERR("Batch send failed to CPU %d\n", i);
				return -1;
			}
		}
	}

	return 0;
#else

	KMSG_ERR("long messages not yet supported in mcast!\n");

	return 0;
#endif
}


static int pcn_kmsg_mcast_callback(struct pcn_kmsg_message *message) 
{
	int rc = 0;
	struct pcn_kmsg_mcast_message *msg = 
		(struct pcn_kmsg_mcast_message *) message;
	pcn_kmsg_work_t *kmsg_work;

	MCAST_PRINTK("Received mcast message, type %d\n", msg->type);

	switch (msg->type) {
		case PCN_KMSG_MCAST_OPEN:
			MCAST_PRINTK("Processing mcast open message...\n");

			/* Need to queue work to remap the window in a kernel
			   thread; it can't happen here */
			kmsg_work = kmalloc(sizeof(pcn_kmsg_work_t), GFP_ATOMIC);
			if (kmsg_work) {
				INIT_WORK((struct work_struct *) kmsg_work,
					  process_kmsg_wq_item);
				kmsg_work->op = PCN_KMSG_WQ_OP_MAP_MCAST_WIN;
				kmsg_work->from_cpu = msg->hdr.from_cpu;
				kmsg_work->id_to_join = msg->id;
				queue_work(kmsg_wq, 
					   (struct work_struct *) kmsg_work);
			} else {
				KMSG_ERR("Failed to kmalloc work structure!\n");
			}

			break;

		case PCN_KMSG_MCAST_ADD_MEMBERS:
			KMSG_ERR("Mcast add not yet implemented...\n");
			break;

		case PCN_KMSG_MCAST_DEL_MEMBERS:
			KMSG_ERR("Mcast delete not yet implemented...\n");
			break;

		case PCN_KMSG_MCAST_CLOSE:
			MCAST_PRINTK("Processing mcast close message...\n");
			pcn_kmsg_mcast_close_notowner(msg->id);
			break;

		default:
			KMSG_ERR("Invalid multicast message type %d\n", 
				 msg->type);
			rc = -1;
			goto out;
	}

	print_mcast_map();

out:
	pcn_kmsg_free_msg(message);
	return rc;
}
