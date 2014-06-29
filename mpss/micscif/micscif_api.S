
micscif_api.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <vma_pvt_release>:
				 */
	struct kref ref;
};

static void vma_pvt_release(struct kref *ref)
{
       0:	55                   	push   %rbp
       1:	48 89 e5             	mov    %rsp,%rbp
       4:	e8 00 00 00 00       	callq  9 <vma_pvt_release+0x9>
	struct vma_pvt *vmapvt = container_of(ref, struct vma_pvt, ref);
       9:	48 83 ef 14          	sub    $0x14,%rdi
	kfree(vmapvt);
       d:	e8 00 00 00 00       	callq  12 <vma_pvt_release+0x12>
}
      12:	5d                   	pop    %rbp
      13:	c3                   	retq   
      14:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
      1b:	00 00 00 00 00 

0000000000000020 <scif_event_register>:
	return ret;
}
EXPORT_SYMBOL(scif_put_pages);

int scif_event_register(scif_callback_t handler)
{
      20:	55                   	push   %rbp
      21:	48 89 e5             	mov    %rsp,%rbp
      24:	41 54                	push   %r12
      26:	53                   	push   %rbx
      27:	e8 00 00 00 00       	callq  2c <scif_event_register+0xc>
      2c:	49 89 fc             	mov    %rdi,%r12
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
      2f:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 36 <scif_event_register+0x16>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
      36:	48 85 ff             	test   %rdi,%rdi
      39:	74 5d                	je     98 <scif_event_register+0x78>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
      3b:	ba 18 00 00 00       	mov    $0x18,%edx
      40:	be d0 00 00 00       	mov    $0xd0,%esi
      45:	e8 00 00 00 00       	callq  4a <scif_event_register+0x2a>
	/* Add to the list of event handlers */
	struct scif_callback *cb = kmalloc(sizeof(*cb), GFP_KERNEL);
	if (!cb)
      4a:	48 85 c0             	test   %rax,%rax
      4d:	48 89 c3             	mov    %rax,%rbx
      50:	74 4e                	je     a0 <scif_event_register+0x80>
		return -ENOMEM;
	mutex_lock(&ms_info.mi_event_cblock);
      52:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
      59:	e8 00 00 00 00       	callq  5e <scif_event_register+0x3e>
	cb->callback_handler = handler;
      5e:	4c 89 63 10          	mov    %r12,0x10(%rbx)
	list_add_tail(&cb->list_member, &ms_info.mi_event_cb);
	mutex_unlock(&ms_info.mi_event_cblock);
      62:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
      69:	48 8b 05 00 00 00 00 	mov    0x0(%rip),%rax        # 70 <scif_event_register+0x50>
#ifndef CONFIG_DEBUG_LIST
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
      70:	48 89 1d 00 00 00 00 	mov    %rbx,0x0(%rip)        # 77 <scif_event_register+0x57>
	new->next = next;
      77:	48 c7 03 00 00 00 00 	movq   $0x0,(%rbx)
	new->prev = prev;
      7e:	48 89 43 08          	mov    %rax,0x8(%rbx)
	prev->next = new;
      82:	48 89 18             	mov    %rbx,(%rax)
      85:	e8 00 00 00 00       	callq  8a <scif_event_register+0x6a>
	return 0;
      8a:	31 c0                	xor    %eax,%eax
}
      8c:	5b                   	pop    %rbx
      8d:	41 5c                	pop    %r12
      8f:	5d                   	pop    %rbp
      90:	c3                   	retq   
      91:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
      98:	bb 10 00 00 00       	mov    $0x10,%ebx
      9d:	eb b3                	jmp    52 <scif_event_register+0x32>
      9f:	90                   	nop
int scif_event_register(scif_callback_t handler)
{
	/* Add to the list of event handlers */
	struct scif_callback *cb = kmalloc(sizeof(*cb), GFP_KERNEL);
	if (!cb)
		return -ENOMEM;
      a0:	b8 f4 ff ff ff       	mov    $0xfffffff4,%eax
      a5:	eb e5                	jmp    8c <scif_event_register+0x6c>
      a7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
      ae:	00 00 

00000000000000b0 <scif_event_unregister>:
	return 0;
}
EXPORT_SYMBOL(scif_event_register);

int scif_event_unregister(scif_callback_t handler)
{
      b0:	55                   	push   %rbp
      b1:	48 89 e5             	mov    %rsp,%rbp
      b4:	41 54                	push   %r12
      b6:	53                   	push   %rbx
      b7:	e8 00 00 00 00       	callq  bc <scif_event_unregister+0xc>
	struct list_head *pos, *unused;
	struct scif_callback *temp;
	int err = -EINVAL;
      bc:	41 bc ea ff ff ff    	mov    $0xffffffea,%r12d
	return 0;
}
EXPORT_SYMBOL(scif_event_register);

int scif_event_unregister(scif_callback_t handler)
{
      c2:	48 89 fb             	mov    %rdi,%rbx
	struct list_head *pos, *unused;
	struct scif_callback *temp;
	int err = -EINVAL;

	mutex_lock(&ms_info.mi_event_cblock);
      c5:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
      cc:	e8 00 00 00 00       	callq  d1 <scif_event_unregister+0x21>
	list_for_each_safe(pos, unused, &ms_info.mi_event_cb) {
      d1:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # d8 <scif_event_unregister+0x28>
      d8:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
      df:	48 8b 39             	mov    (%rcx),%rdi
      e2:	74 27                	je     10b <scif_event_unregister+0x5b>
		temp = list_entry(pos, struct scif_callback, list_member);
		if (temp->callback_handler == handler) {
      e4:	48 39 59 10          	cmp    %rbx,0x10(%rcx)
      e8:	75 0f                	jne    f9 <scif_event_unregister+0x49>
      ea:	eb 33                	jmp    11f <scif_event_unregister+0x6f>
      ec:	0f 1f 40 00          	nopl   0x0(%rax)
      f0:	48 39 5f 10          	cmp    %rbx,0x10(%rdi)
      f4:	74 32                	je     128 <scif_event_unregister+0x78>
	struct list_head *pos, *unused;
	struct scif_callback *temp;
	int err = -EINVAL;

	mutex_lock(&ms_info.mi_event_cblock);
	list_for_each_safe(pos, unused, &ms_info.mi_event_cb) {
      f6:	48 89 c7             	mov    %rax,%rdi
      f9:	48 81 ff 00 00 00 00 	cmp    $0x0,%rdi
     100:	48 8b 07             	mov    (%rdi),%rax
     103:	75 eb                	jne    f0 <scif_event_unregister+0x40>

int scif_event_unregister(scif_callback_t handler)
{
	struct list_head *pos, *unused;
	struct scif_callback *temp;
	int err = -EINVAL;
     105:	41 bc ea ff ff ff    	mov    $0xffffffea,%r12d
			kfree(temp);
			break;
		}
	}

	mutex_unlock(&ms_info.mi_event_cblock);
     10b:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     112:	e8 00 00 00 00       	callq  117 <scif_event_unregister+0x67>
	return err;
}
     117:	44 89 e0             	mov    %r12d,%eax
     11a:	5b                   	pop    %rbx
     11b:	41 5c                	pop    %r12
     11d:	5d                   	pop    %rbp
     11e:	c3                   	retq   
	int err = -EINVAL;

	mutex_lock(&ms_info.mi_event_cblock);
	list_for_each_safe(pos, unused, &ms_info.mi_event_cb) {
		temp = list_entry(pos, struct scif_callback, list_member);
		if (temp->callback_handler == handler) {
     11f:	48 89 f8             	mov    %rdi,%rax
     122:	48 89 cf             	mov    %rcx,%rdi
     125:	0f 1f 00             	nopl   (%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
     128:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
			err = 0;
     12c:	45 31 e4             	xor    %r12d,%r12d
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
     12f:	48 89 48 08          	mov    %rcx,0x8(%rax)
	prev->next = next;
     133:	48 89 01             	mov    %rax,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     136:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
     13d:	00 ad de 
     140:	48 89 07             	mov    %rax,(%rdi)
	entry->prev = LIST_POISON2;
     143:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
     14a:	00 ad de 
     14d:	48 89 47 08          	mov    %rax,0x8(%rdi)
			list_del(pos);
			kfree(temp);
     151:	e8 00 00 00 00       	callq  156 <scif_event_unregister+0xa6>
			break;
     156:	eb b3                	jmp    10b <scif_event_unregister+0x5b>
     158:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
     15f:	00 

0000000000000160 <scif_get_nodeIDs>:
 *	scif_get_nodeIDs() returns the total number of scif nodes
 *	(including host) in the system
 */
int
scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self)
{
     160:	55                   	push   %rbp
     161:	48 89 e5             	mov    %rsp,%rbp
     164:	41 54                	push   %r12
     166:	53                   	push   %rbx
     167:	e8 00 00 00 00       	callq  16c <scif_get_nodeIDs+0xc>
	int node;
#ifdef _MIC_SCIF_
	micscif_get_node_info();
#endif

	*self = ms_info.mi_nodeid;
     16c:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 172 <scif_get_nodeIDs+0x12>
 *	scif_get_nodeIDs() returns the total number of scif nodes
 *	(including host) in the system
 */
int
scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self)
{
     172:	49 89 fc             	mov    %rdi,%r12
	int node;
#ifdef _MIC_SCIF_
	micscif_get_node_info();
#endif

	*self = ms_info.mi_nodeid;
     175:	66 89 02             	mov    %ax,(%rdx)
	mutex_lock(&ms_info.mi_conflock);
     178:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 *	scif_get_nodeIDs() returns the total number of scif nodes
 *	(including host) in the system
 */
int
scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self)
{
     17f:	89 f3                	mov    %esi,%ebx
#ifdef _MIC_SCIF_
	micscif_get_node_info();
#endif

	*self = ms_info.mi_nodeid;
	mutex_lock(&ms_info.mi_conflock);
     181:	e8 00 00 00 00       	callq  186 <scif_get_nodeIDs+0x26>
	len = SCIF_MIN(len, (int32_t)ms_info.mi_total);
     186:	8b 35 00 00 00 00    	mov    0x0(%rip),%esi        # 18c <scif_get_nodeIDs+0x2c>
	for (node = 0; node <=(int32_t)ms_info.mi_maxid; node++) {
     18c:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 192 <scif_get_nodeIDs+0x32>
	micscif_get_node_info();
#endif

	*self = ms_info.mi_nodeid;
	mutex_lock(&ms_info.mi_conflock);
	len = SCIF_MIN(len, (int32_t)ms_info.mi_total);
     192:	39 f3                	cmp    %esi,%ebx
     194:	0f 4e f3             	cmovle %ebx,%esi
	for (node = 0; node <=(int32_t)ms_info.mi_maxid; node++) {
     197:	85 c0                	test   %eax,%eax
     199:	78 4c                	js     1e7 <scif_get_nodeIDs+0x87>
     19b:	31 d2                	xor    %edx,%edx
     19d:	45 31 c0             	xor    %r8d,%r8d
     1a0:	31 db                	xor    %ebx,%ebx
     1a2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		if (ms_info.mi_mask & (1UL << node)) {
     1a8:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # 1af <scif_get_nodeIDs+0x4f>
     1af:	48 0f a3 d1          	bt     %rdx,%rcx
     1b3:	73 14                	jae    1c9 <scif_get_nodeIDs+0x69>
			online++;
     1b5:	83 c3 01             	add    $0x1,%ebx
			if (offset < len)
     1b8:	44 39 c6             	cmp    %r8d,%esi
     1bb:	7e 0c                	jle    1c9 <scif_get_nodeIDs+0x69>
				nodes[offset++] = node;
     1bd:	49 63 c0             	movslq %r8d,%rax
     1c0:	41 83 c0 01          	add    $0x1,%r8d
     1c4:	66 41 89 14 44       	mov    %dx,(%r12,%rax,2)
#endif

	*self = ms_info.mi_nodeid;
	mutex_lock(&ms_info.mi_conflock);
	len = SCIF_MIN(len, (int32_t)ms_info.mi_total);
	for (node = 0; node <=(int32_t)ms_info.mi_maxid; node++) {
     1c9:	83 c2 01             	add    $0x1,%edx
     1cc:	39 15 00 00 00 00    	cmp    %edx,0x0(%rip)        # 1d2 <scif_get_nodeIDs+0x72>
     1d2:	7d d4                	jge    1a8 <scif_get_nodeIDs+0x48>
				nodes[offset++] = node;
		}
	}
	pr_debug("SCIFAPI get_nodeIDs total %d online %d filled in %d nodes\n", 
		ms_info.mi_total, online, len);
	mutex_unlock(&ms_info.mi_conflock);
     1d4:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     1db:	e8 00 00 00 00       	callq  1e0 <scif_get_nodeIDs+0x80>

	return online;
}
     1e0:	89 d8                	mov    %ebx,%eax
     1e2:	5b                   	pop    %rbx
     1e3:	41 5c                	pop    %r12
     1e5:	5d                   	pop    %rbp
     1e6:	c3                   	retq   
 *	(including host) in the system
 */
int
scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self)
{
	int online = 0;
     1e7:	31 db                	xor    %ebx,%ebx
     1e9:	eb e9                	jmp    1d4 <scif_get_nodeIDs+0x74>
     1eb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000000001f0 <scif_vma_open>:
 * The kernel invokes this function only on one of the two VMAs.
 *
 * Return Values: None.
 */
static void scif_vma_open(struct vm_area_struct *vma)
{
     1f0:	55                   	push   %rbp
     1f1:	48 89 e5             	mov    %rsp,%rbp
     1f4:	e8 00 00 00 00       	callq  1f9 <scif_vma_open+0x9>
	struct vma_pvt *vmapvt = ((vma)->vm_private_data);
	pr_debug("SCIFAPI vma open: vma_start 0x%lx vma_end 0x%lx\n", 
			((vma)->vm_start), ((vma)->vm_end));
	kref_get(&vmapvt->ref);
     1f9:	48 8b bf a0 00 00 00 	mov    0xa0(%rdi),%rdi
     200:	48 83 c7 14          	add    $0x14,%rdi
     204:	e8 00 00 00 00       	callq  209 <scif_vma_open+0x19>
}
     209:	5d                   	pop    %rbp
     20a:	c3                   	retq   
     20b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000000000210 <scif_readfrom>:
 *	else an apt error is returned as documented in scif.h.
 */
int
scif_readfrom(scif_epd_t epd, off_t loffset, size_t len,
				off_t roffset, int flags)
{
     210:	55                   	push   %rbp
     211:	48 89 e5             	mov    %rsp,%rbp
     214:	41 57                	push   %r15
     216:	41 56                	push   %r14
     218:	41 55                	push   %r13
     21a:	41 54                	push   %r12
     21c:	53                   	push   %rbx
     21d:	48 83 ec 28          	sub    $0x28,%rsp
     221:	e8 00 00 00 00       	callq  226 <scif_readfrom+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
     226:	48 8d 87 70 01 00 00 	lea    0x170(%rdi),%rax
     22d:	48 89 f3             	mov    %rsi,%rbx
     230:	49 89 cc             	mov    %rcx,%r12
     233:	49 89 fd             	mov    %rdi,%r13
     236:	48 89 c7             	mov    %rax,%rdi
     239:	49 89 d7             	mov    %rdx,%r15
     23c:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
     240:	45 89 c6             	mov    %r8d,%r14d
     243:	e8 00 00 00 00       	callq  248 <scif_readfrom+0x38>
#endif

static __always_inline
bool is_unaligned(off_t src_offset, off_t dst_offset)
{
        src_offset = src_offset & (L1_CACHE_BYTES - 1);
     248:	48 89 da             	mov    %rbx,%rdx
        dst_offset = dst_offset & (L1_CACHE_BYTES - 1);
     24b:	4c 89 e0             	mov    %r12,%rax
#endif

static __always_inline
bool is_unaligned(off_t src_offset, off_t dst_offset)
{
        src_offset = src_offset & (L1_CACHE_BYTES - 1);
     24e:	83 e2 3f             	and    $0x3f,%edx
        dst_offset = dst_offset & (L1_CACHE_BYTES - 1);
     251:	83 e0 3f             	and    $0x3f,%eax
        if (src_offset == dst_offset)
     254:	48 39 c2             	cmp    %rax,%rdx
     257:	75 49                	jne    2a2 <scif_readfrom+0x92>
     259:	eb 50                	jmp    2ab <scif_readfrom+0x9b>
     25b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		" offset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
			err = micscif_rma_copy(epd, loffset, NULL,
     260:	31 d2                	xor    %edx,%edx
     262:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%rsp)
     269:	00 
     26a:	45 89 f1             	mov    %r14d,%r9d
     26d:	4d 89 e0             	mov    %r12,%r8
     270:	c7 04 24 01 00 00 00 	movl   $0x1,(%rsp)
     277:	b9 00 00 10 00       	mov    $0x100000,%ecx
     27c:	48 89 de             	mov    %rbx,%rsi
     27f:	4c 89 ef             	mov    %r13,%rdi
     282:	e8 00 00 00 00       	callq  287 <scif_readfrom+0x77>
				MAX_UNALIGNED_BUF_SIZE,
				roffset, flags, REMOTE_TO_LOCAL, false);
			if (err)
     287:	85 c0                	test   %eax,%eax
		" offset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
			err = micscif_rma_copy(epd, loffset, NULL,
     289:	89 c2                	mov    %eax,%edx
				MAX_UNALIGNED_BUF_SIZE,
				roffset, flags, REMOTE_TO_LOCAL, false);
			if (err)
     28b:	75 45                	jne    2d2 <scif_readfrom+0xc2>
     28d:	48 81 c3 00 00 10 00 	add    $0x100000,%rbx
     294:	49 81 c4 00 00 10 00 	add    $0x100000,%r12
				goto readfrom_err;
			loffset += MAX_UNALIGNED_BUF_SIZE;
			roffset += MAX_UNALIGNED_BUF_SIZE;
			len -=MAX_UNALIGNED_BUF_SIZE;
     29b:	49 81 ef 00 00 10 00 	sub    $0x100000,%r15
	pr_debug("SCIFAPI readfrom: ep %p loffset 0x%lx len 0x%lx"
		" offset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
     2a2:	49 81 ff 00 00 10 00 	cmp    $0x100000,%r15
     2a9:	77 b5                	ja     260 <scif_readfrom+0x50>
			loffset += MAX_UNALIGNED_BUF_SIZE;
			roffset += MAX_UNALIGNED_BUF_SIZE;
			len -=MAX_UNALIGNED_BUF_SIZE;
		}
	}
	err = micscif_rma_copy(epd, loffset, NULL, len,
     2ab:	31 d2                	xor    %edx,%edx
     2ad:	c7 44 24 08 01 00 00 	movl   $0x1,0x8(%rsp)
     2b4:	00 
     2b5:	45 89 f1             	mov    %r14d,%r9d
     2b8:	4d 89 e0             	mov    %r12,%r8
     2bb:	c7 04 24 01 00 00 00 	movl   $0x1,(%rsp)
     2c2:	4c 89 f9             	mov    %r15,%rcx
     2c5:	48 89 de             	mov    %rbx,%rsi
     2c8:	4c 89 ef             	mov    %r13,%rdi
     2cb:	e8 00 00 00 00       	callq  2d0 <scif_readfrom+0xc0>
     2d0:	89 c2                	mov    %eax,%edx
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
     2d2:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
     2d6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     2dd:	89 55 c4             	mov    %edx,-0x3c(%rbp)
     2e0:	e8 00 00 00 00       	callq  2e5 <scif_readfrom+0xd5>
	int ret;
	get_kref_count(epd);
	ret = __scif_readfrom(epd, loffset, len, roffset, flags);
	put_kref_count(epd);
	return ret;
}
     2e5:	8b 55 c4             	mov    -0x3c(%rbp),%edx
     2e8:	48 83 c4 28          	add    $0x28,%rsp
     2ec:	5b                   	pop    %rbx
     2ed:	41 5c                	pop    %r12
     2ef:	41 5d                	pop    %r13
     2f1:	89 d0                	mov    %edx,%eax
     2f3:	41 5e                	pop    %r14
     2f5:	41 5f                	pop    %r15
     2f7:	5d                   	pop    %rbp
     2f8:	c3                   	retq   
     2f9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000000300 <scif_writeto>:
 *	else an apt error is returned as documented in scif.h.
 *
 */
int scif_writeto(scif_epd_t epd, off_t loffset, size_t len,
				off_t roffset, int flags)
{
     300:	55                   	push   %rbp
     301:	48 89 e5             	mov    %rsp,%rbp
     304:	41 57                	push   %r15
     306:	41 56                	push   %r14
     308:	41 55                	push   %r13
     30a:	41 54                	push   %r12
     30c:	53                   	push   %rbx
     30d:	48 83 ec 28          	sub    $0x28,%rsp
     311:	e8 00 00 00 00       	callq  316 <scif_writeto+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
     316:	48 8d 87 70 01 00 00 	lea    0x170(%rdi),%rax
     31d:	48 89 f3             	mov    %rsi,%rbx
     320:	49 89 cc             	mov    %rcx,%r12
     323:	49 89 fd             	mov    %rdi,%r13
     326:	48 89 c7             	mov    %rax,%rdi
     329:	49 89 d7             	mov    %rdx,%r15
     32c:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
     330:	45 89 c6             	mov    %r8d,%r14d
     333:	e8 00 00 00 00       	callq  338 <scif_writeto+0x38>
#endif

static __always_inline
bool is_unaligned(off_t src_offset, off_t dst_offset)
{
        src_offset = src_offset & (L1_CACHE_BYTES - 1);
     338:	48 89 da             	mov    %rbx,%rdx
        dst_offset = dst_offset & (L1_CACHE_BYTES - 1);
     33b:	4c 89 e0             	mov    %r12,%rax
#endif

static __always_inline
bool is_unaligned(off_t src_offset, off_t dst_offset)
{
        src_offset = src_offset & (L1_CACHE_BYTES - 1);
     33e:	83 e2 3f             	and    $0x3f,%edx
        dst_offset = dst_offset & (L1_CACHE_BYTES - 1);
     341:	83 e0 3f             	and    $0x3f,%eax
        if (src_offset == dst_offset)
     344:	48 39 c2             	cmp    %rax,%rdx
     347:	75 49                	jne    392 <scif_writeto+0x92>
     349:	eb 50                	jmp    39b <scif_writeto+0x9b>
     34b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		" roffset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
			err = micscif_rma_copy(epd, loffset, NULL,
     350:	31 d2                	xor    %edx,%edx
     352:	c7 44 24 08 00 00 00 	movl   $0x0,0x8(%rsp)
     359:	00 
     35a:	45 89 f1             	mov    %r14d,%r9d
     35d:	4d 89 e0             	mov    %r12,%r8
     360:	c7 04 24 00 00 00 00 	movl   $0x0,(%rsp)
     367:	b9 00 00 10 00       	mov    $0x100000,%ecx
     36c:	48 89 de             	mov    %rbx,%rsi
     36f:	4c 89 ef             	mov    %r13,%rdi
     372:	e8 00 00 00 00       	callq  377 <scif_writeto+0x77>
				MAX_UNALIGNED_BUF_SIZE,
				roffset, flags, LOCAL_TO_REMOTE, false);
			if (err)
     377:	85 c0                	test   %eax,%eax
		" roffset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
			err = micscif_rma_copy(epd, loffset, NULL,
     379:	89 c2                	mov    %eax,%edx
				MAX_UNALIGNED_BUF_SIZE,
				roffset, flags, LOCAL_TO_REMOTE, false);
			if (err)
     37b:	75 45                	jne    3c2 <scif_writeto+0xc2>
     37d:	48 81 c3 00 00 10 00 	add    $0x100000,%rbx
     384:	49 81 c4 00 00 10 00 	add    $0x100000,%r12
				goto writeto_err;
			loffset += MAX_UNALIGNED_BUF_SIZE;
			roffset += MAX_UNALIGNED_BUF_SIZE;
			len -= MAX_UNALIGNED_BUF_SIZE;
     38b:	49 81 ef 00 00 10 00 	sub    $0x100000,%r15
	pr_debug("SCIFAPI writeto: ep %p loffset 0x%lx len 0x%lx"
		" roffset 0x%lx flags 0x%x\n", 
		epd, loffset, len, roffset, flags);

	if (is_unaligned(loffset, roffset)) {
		while(len > MAX_UNALIGNED_BUF_SIZE) {
     392:	49 81 ff 00 00 10 00 	cmp    $0x100000,%r15
     399:	77 b5                	ja     350 <scif_writeto+0x50>
			loffset += MAX_UNALIGNED_BUF_SIZE;
			roffset += MAX_UNALIGNED_BUF_SIZE;
			len -= MAX_UNALIGNED_BUF_SIZE;
		}
	}
	err = micscif_rma_copy(epd, loffset, NULL, len,
     39b:	31 d2                	xor    %edx,%edx
     39d:	c7 44 24 08 01 00 00 	movl   $0x1,0x8(%rsp)
     3a4:	00 
     3a5:	45 89 f1             	mov    %r14d,%r9d
     3a8:	4d 89 e0             	mov    %r12,%r8
     3ab:	c7 04 24 00 00 00 00 	movl   $0x0,(%rsp)
     3b2:	4c 89 f9             	mov    %r15,%rcx
     3b5:	48 89 de             	mov    %rbx,%rsi
     3b8:	4c 89 ef             	mov    %r13,%rdi
     3bb:	e8 00 00 00 00       	callq  3c0 <scif_writeto+0xc0>
     3c0:	89 c2                	mov    %eax,%edx
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
     3c2:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
     3c6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     3cd:	89 55 c4             	mov    %edx,-0x3c(%rbp)
     3d0:	e8 00 00 00 00       	callq  3d5 <scif_writeto+0xd5>
	int ret;
	get_kref_count(epd);
	ret = __scif_writeto(epd, loffset, len, roffset, flags);
	put_kref_count(epd);
	return ret;
}
     3d5:	8b 55 c4             	mov    -0x3c(%rbp),%edx
     3d8:	48 83 c4 28          	add    $0x28,%rsp
     3dc:	5b                   	pop    %rbx
     3dd:	41 5c                	pop    %r12
     3df:	41 5d                	pop    %r13
     3e1:	89 d0                	mov    %edx,%eax
     3e3:	41 5e                	pop    %r14
     3e5:	41 5f                	pop    %r15
     3e7:	5d                   	pop    %rbp
     3e8:	c3                   	retq   
     3e9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000003f0 <scif_unpin_pages>:
 * Upon successful completion, scif_unpin_pages() returns 0;
 * else an apt error is returned as documented in scif.h
 */
int
scif_unpin_pages(scif_pinned_pages_t pinned_pages)
{
     3f0:	55                   	push   %rbp
     3f1:	48 89 e5             	mov    %rsp,%rbp
     3f4:	e8 00 00 00 00       	callq  3f9 <scif_unpin_pages+0x9>
	int err = 0, ret;

	if (!pinned_pages || SCIFEP_MAGIC != pinned_pages->magic)
     3f9:	48 85 ff             	test   %rdi,%rdi
     3fc:	74 42                	je     440 <scif_unpin_pages+0x50>
     3fe:	48 b8 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rax
     405:	00 1f 5c 
     408:	48 39 47 18          	cmp    %rax,0x18(%rdi)
     40c:	75 32                	jne    440 <scif_unpin_pages+0x50>
		return -EINVAL;

	ret = atomic_sub_return((int32_t)pinned_pages->nr_pages, 
     40e:	48 8b 17             	mov    (%rdi),%rdx
 *
 * Atomically subtracts @i from @v and returns @v - @i
 */
static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
     411:	89 d0                	mov    %edx,%eax
     413:	f7 d8                	neg    %eax
	unsigned long flags;
	if (unlikely(boot_cpu_data.x86 <= 3))
		goto no_xadd;
#endif
	/* Modern 486+ processor */
	return i + xadd(&v->counter, i);
     415:	f0 0f c1 47 10       	lock xadd %eax,0x10(%rdi)
			&pinned_pages->ref_count);
	BUG_ON(ret < 0);
     41a:	39 d0                	cmp    %edx,%eax
     41c:	78 32                	js     450 <scif_unpin_pages+0x60>
 * else an apt error is returned as documented in scif.h
 */
int
scif_unpin_pages(scif_pinned_pages_t pinned_pages)
{
	int err = 0, ret;
     41e:	b8 00 00 00 00       	mov    $0x0,%eax
	 * Destroy the window if the ref count for this set of pinned
	 * pages has dropped to zero. If it is positive then there is
	 * a valid registered window which is backed by these pages and
	 * it will be destroyed once all such windows are unregistered.
	 */
	if (!ret)
     423:	74 0b                	je     430 <scif_unpin_pages+0x40>
		err = micscif_destroy_pinned_pages(pinned_pages);

	return err;
}
     425:	5d                   	pop    %rbp
     426:	c3                   	retq   
     427:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
     42e:	00 00 
	 * pages has dropped to zero. If it is positive then there is
	 * a valid registered window which is backed by these pages and
	 * it will be destroyed once all such windows are unregistered.
	 */
	if (!ret)
		err = micscif_destroy_pinned_pages(pinned_pages);
     430:	e8 00 00 00 00       	callq  435 <scif_unpin_pages+0x45>

	return err;
}
     435:	5d                   	pop    %rbp
     436:	c3                   	retq   
     437:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
     43e:	00 00 
scif_unpin_pages(scif_pinned_pages_t pinned_pages)
{
	int err = 0, ret;

	if (!pinned_pages || SCIFEP_MAGIC != pinned_pages->magic)
		return -EINVAL;
     440:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
	 */
	if (!ret)
		err = micscif_destroy_pinned_pages(pinned_pages);

	return err;
}
     445:	5d                   	pop    %rbp
     446:	c3                   	retq   
     447:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
     44e:	00 00 
	if (!pinned_pages || SCIFEP_MAGIC != pinned_pages->magic)
		return -EINVAL;

	ret = atomic_sub_return((int32_t)pinned_pages->nr_pages, 
			&pinned_pages->ref_count);
	BUG_ON(ret < 0);
     450:	0f 0b                	ud2    
     452:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
     459:	1f 84 00 00 00 00 00 

0000000000000460 <atomic_long_add_unless.constprop.15>:
	atomic64_t *v = (atomic64_t *)l;

	return (long)atomic64_dec_return(v);
}

static inline long atomic_long_add_unless(atomic_long_t *l, long a, long u)
     460:	55                   	push   %rbp
     461:	48 89 e5             	mov    %rsp,%rbp
     464:	e8 00 00 00 00       	callq  469 <atomic_long_add_unless.constprop.15+0x9>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     469:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
     470:	00 00 80 
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
     473:	48 8b 0f             	mov    (%rdi),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     476:	48 39 f1             	cmp    %rsi,%rcx
     479:	74 45                	je     4c0 <atomic_long_add_unless.constprop.15+0x60>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     47b:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     47f:	48 89 c8             	mov    %rcx,%rax
     482:	f0 48 0f b1 17       	lock cmpxchg %rdx,(%rdi)
     487:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     48a:	b8 01 00 00 00       	mov    $0x1,%eax
     48f:	48 39 d1             	cmp    %rdx,%rcx
     492:	75 0c                	jne    4a0 <atomic_long_add_unless.constprop.15+0x40>
{
	atomic64_t *v = (atomic64_t *)l;

	return (long)atomic64_add_unless(v, a, u);
}
     494:	5d                   	pop    %rbp
     495:	c3                   	retq   
     496:	48 89 c2             	mov    %rax,%rdx
     499:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     4a0:	48 39 f2             	cmp    %rsi,%rdx
     4a3:	74 1b                	je     4c0 <atomic_long_add_unless.constprop.15+0x60>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     4a5:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     4a9:	48 89 d0             	mov    %rdx,%rax
     4ac:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     4b1:	48 39 d0             	cmp    %rdx,%rax
     4b4:	75 e0                	jne    496 <atomic_long_add_unless.constprop.15+0x36>
     4b6:	b8 01 00 00 00       	mov    $0x1,%eax
     4bb:	5d                   	pop    %rbp
     4bc:	c3                   	retq   
     4bd:	0f 1f 00             	nopl   (%rax)
     4c0:	31 c0                	xor    %eax,%eax
     4c2:	5d                   	pop    %rbp
     4c3:	c3                   	retq   
     4c4:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
     4cb:	00 00 00 00 00 

00000000000004d0 <atomic_long_sub_return.constprop.16>:
	atomic64_t *v = (atomic64_t *)l;

	return (long)atomic64_add_return(i, v);
}

static inline long atomic_long_sub_return(long i, atomic_long_t *l)
     4d0:	55                   	push   %rbp
     4d1:	48 89 e5             	mov    %rsp,%rbp
     4d4:	e8 00 00 00 00       	callq  4d9 <atomic_long_sub_return.constprop.16+0x9>
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
     4d9:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
     4e0:	f0 48 0f c1 07       	lock xadd %rax,(%rdi)
     4e5:	48 83 e8 01          	sub    $0x1,%rax
{
	atomic64_t *v = (atomic64_t *)l;

	return (long)atomic64_sub_return(i, v);
}
     4e9:	5d                   	pop    %rbp
     4ea:	c3                   	retq   
     4eb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000000004f0 <atomic_long_add.constprop.17>:
	atomic64_t *v = (atomic64_t *)l;

	atomic64_dec(v);
}

static inline void atomic_long_add(long i, atomic_long_t *l)
     4f0:	55                   	push   %rbp
     4f1:	48 89 e5             	mov    %rsp,%rbp
     4f4:	e8 00 00 00 00       	callq  4f9 <atomic_long_add.constprop.17+0x9>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
     4f9:	f0 48 83 07 01       	lock addq $0x1,(%rdi)
{
	atomic64_t *v = (atomic64_t *)l;

	atomic64_add(i, v);
}
     4fe:	5d                   	pop    %rbp
     4ff:	c3                   	retq   

0000000000000500 <scif_munmap>:
 * is opened and closed exactly once by each process that uses it.
 *
 * Return Values: None.
 */
void scif_munmap(struct vm_area_struct *vma)
{
     500:	55                   	push   %rbp
     501:	48 89 e5             	mov    %rsp,%rbp
     504:	41 57                	push   %r15
     506:	41 56                	push   %r14
     508:	41 55                	push   %r13
     50a:	41 54                	push   %r12
     50c:	53                   	push   %rbx
     50d:	48 81 ec c8 00 00 00 	sub    $0xc8,%rsp
     514:	e8 00 00 00 00       	callq  519 <scif_munmap+0x19>
	struct endpt *ep;
	struct vma_pvt *vmapvt = ((vma)->vm_private_data);
     519:	4c 8b af a0 00 00 00 	mov    0xa0(%rdi),%r13
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT );
     520:	48 8b 5f 10          	mov    0x10(%rdi),%rbx
     524:	48 2b 5f 08          	sub    0x8(%rdi),%rbx
	uint64_t offset;
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
     528:	48 c7 85 40 ff ff ff 	movq   $0x0,-0xc0(%rbp)
     52f:	00 00 00 00 
 * is opened and closed exactly once by each process that uses it.
 *
 * Return Values: None.
 */
void scif_munmap(struct vm_area_struct *vma)
{
     533:	49 89 ff             	mov    %rdi,%r15
	uint64_t offset;
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
	int err;

	might_sleep();
     536:	e8 00 00 00 00       	callq  53b <scif_munmap+0x3b>
 */
void scif_munmap(struct vm_area_struct *vma)
{
	struct endpt *ep;
	struct vma_pvt *vmapvt = ((vma)->vm_private_data);
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT );
     53b:	48 c1 eb 0c          	shr    $0xc,%rbx

	might_sleep();
	pr_debug("SCIFAPI munmap: vma_start 0x%lx vma_end 0x%lx\n", 
			((vma)->vm_start), ((vma)->vm_end));
	/* used to be a BUG_ON(), prefer keeping the kernel alive */
	if (!vmapvt) {
     53f:	4d 85 ed             	test   %r13,%r13
     542:	0f 84 2c 03 00 00    	je     874 <scif_munmap+0x374>
			((vma)->vm_start), ((vma)->vm_end));
		return;
	}

	ep = vmapvt->ep;
	offset = vmapvt->valid_offset ? vmapvt->offset :
     548:	41 80 7d 10 00       	cmpb   $0x0,0x10(%r13)
		printk(KERN_ERR "SCIFAPI munmap: vma_start 0x%lx vma_end 0x%lx\n", 
			((vma)->vm_start), ((vma)->vm_end));
		return;
	}

	ep = vmapvt->ep;
     54d:	4d 8b 65 00          	mov    0x0(%r13),%r12
	offset = vmapvt->valid_offset ? vmapvt->offset :
     551:	0f 85 29 01 00 00    	jne    680 <scif_munmap+0x180>
     557:	4d 8b 87 90 00 00 00 	mov    0x90(%r15),%r8
     55e:	49 c1 e0 0c          	shl    $0xc,%r8
		((vma)->vm_pgoff) << PAGE_SHIFT;
	pr_debug("SCIFAPI munmap: ep %p %s  nr_pages 0x%x offset 0x%llx\n", 
     562:	41 8b 04 24          	mov    (%r12),%eax
		ep, scif_ep_states[ep->state], nr_pages, offset);

	req.out_window = &window;
     566:	48 8d 85 40 ff ff ff 	lea    -0xc0(%rbp),%rax
	req.offset = offset;
     56d:	4c 89 45 a8          	mov    %r8,-0x58(%rbp)
	offset = vmapvt->valid_offset ? vmapvt->offset :
		((vma)->vm_pgoff) << PAGE_SHIFT;
	pr_debug("SCIFAPI munmap: ep %p %s  nr_pages 0x%x offset 0x%llx\n", 
		ep, scif_ep_states[ep->state], nr_pages, offset);

	req.out_window = &window;
     571:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
	req.offset = offset;
	req.nr_bytes = ((vma)->vm_end) - ((vma)->vm_start);
     575:	49 8b 47 10          	mov    0x10(%r15),%rax
     579:	49 2b 47 08          	sub    0x8(%r15),%rax
	req.prot = ((vma)->vm_flags) & (VM_READ | VM_WRITE);
	req.type = WINDOW_PARTIAL;
     57d:	c7 45 bc 00 00 00 00 	movl   $0x0,-0x44(%rbp)
	req.head = &ep->rma_info.remote_reg_list;

	micscif_inc_node_refcnt(ep->remote_dev, 1);
     584:	4d 8b b4 24 48 01 00 	mov    0x148(%r12),%r14
     58b:	00 
	pr_debug("SCIFAPI munmap: ep %p %s  nr_pages 0x%x offset 0x%llx\n", 
		ep, scif_ep_states[ep->state], nr_pages, offset);

	req.out_window = &window;
	req.offset = offset;
	req.nr_bytes = ((vma)->vm_end) - ((vma)->vm_start);
     58c:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
	req.prot = ((vma)->vm_flags) & (VM_READ | VM_WRITE);
     590:	41 8b 47 30          	mov    0x30(%r15),%eax
     594:	83 e0 03             	and    $0x3,%eax
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
     597:	4d 85 f6             	test   %r14,%r14
     59a:	89 45 b8             	mov    %eax,-0x48(%rbp)
	req.type = WINDOW_PARTIAL;
	req.head = &ep->rma_info.remote_reg_list;
     59d:	49 8d 44 24 38       	lea    0x38(%r12),%rax
     5a2:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
     5a6:	0f 85 e4 00 00 00    	jne    690 <scif_munmap+0x190>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);
     5ac:	4d 8d b4 24 80 00 00 	lea    0x80(%r12),%r14
     5b3:	00 
     5b4:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
     5bb:	4c 89 f7             	mov    %r14,%rdi
     5be:	e8 00 00 00 00       	callq  5c3 <scif_munmap+0xc3>

	if ((err = micscif_query_window(&req)))
     5c3:	48 8d 7d a0          	lea    -0x60(%rbp),%rdi
     5c7:	e8 00 00 00 00       	callq  5cc <scif_munmap+0xcc>
     5cc:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     5d3:	85 c0                	test   %eax,%eax
     5d5:	0f 85 c5 02 00 00    	jne    8a0 <scif_munmap+0x3a0>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	else
		micscif_rma_list_munmap(window, offset, nr_pages);
     5db:	48 8b bd 40 ff ff ff 	mov    -0xc0(%rbp),%rdi
     5e2:	89 da                	mov    %ebx,%edx
     5e4:	4c 89 c6             	mov    %r8,%rsi
     5e7:	e8 00 00 00 00       	callq  5ec <scif_munmap+0xec>

	mutex_unlock(&ep->rma_info.rma_lock);
     5ec:	4c 89 f7             	mov    %r14,%rdi
     5ef:	e8 00 00 00 00       	callq  5f4 <scif_munmap+0xf4>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
     5f4:	4d 8b b4 24 48 01 00 	mov    0x148(%r12),%r14
     5fb:	00 
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
     5fc:	4d 85 f6             	test   %r14,%r14
     5ff:	74 29                	je     62a <scif_munmap+0x12a>
		if (unlikely((atomic_long_sub_return(cnt, 
     601:	4d 8d 96 88 01 00 00 	lea    0x188(%r14),%r10
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
     608:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
     60f:	f0 49 0f c1 86 88 01 	lock xadd %rax,0x188(%r14)
     616:	00 00 
     618:	48 83 e8 01          	sub    $0x1,%rax
     61c:	0f 88 ce 00 00 00    	js     6f0 <scif_munmap+0x1f0>
     622:	4d 8b b4 24 48 01 00 	mov    0x148(%r12),%r14
     629:	00 

	micscif_destroy_node_dep(ep->remote_dev, nr_pages);
     62a:	89 de                	mov    %ebx,%esi
     62c:	4c 89 f7             	mov    %r14,%rdi
     62f:	e8 00 00 00 00       	callq  634 <scif_munmap+0x134>
	 * The kernel probably zeroes these out but we still want
	 * to clean up our own mess just in case.
	 */
	vma->vm_ops = NULL;
	((vma)->vm_private_data) = NULL;
	kref_put(&vmapvt->ref, vma_pvt_release);
     634:	49 8d 7d 14          	lea    0x14(%r13),%rdi
     638:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi

	/*
	 * The kernel probably zeroes these out but we still want
	 * to clean up our own mess just in case.
	 */
	vma->vm_ops = NULL;
     63f:	49 c7 87 88 00 00 00 	movq   $0x0,0x88(%r15)
     646:	00 00 00 00 
	((vma)->vm_private_data) = NULL;
     64a:	49 c7 87 a0 00 00 00 	movq   $0x0,0xa0(%r15)
     651:	00 00 00 00 
	kref_put(&vmapvt->ref, vma_pvt_release);
     655:	e8 00 00 00 00       	callq  65a <scif_munmap+0x15a>
	micscif_rma_put_task(ep, nr_pages);
     65a:	89 de                	mov    %ebx,%esi
     65c:	4c 89 e7             	mov    %r12,%rdi
     65f:	e8 00 00 00 00       	callq  664 <scif_munmap+0x164>
}
     664:	48 81 c4 c8 00 00 00 	add    $0xc8,%rsp
     66b:	5b                   	pop    %rbx
     66c:	41 5c                	pop    %r12
     66e:	41 5d                	pop    %r13
     670:	41 5e                	pop    %r14
     672:	41 5f                	pop    %r15
     674:	5d                   	pop    %rbp
     675:	c3                   	retq   
     676:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
     67d:	00 00 00 
			((vma)->vm_start), ((vma)->vm_end));
		return;
	}

	ep = vmapvt->ep;
	offset = vmapvt->valid_offset ? vmapvt->offset :
     680:	4d 8b 45 08          	mov    0x8(%r13),%r8
     684:	e9 d9 fe ff ff       	jmpq   562 <scif_munmap+0x62>
     689:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
     690:	49 8b 8e 88 01 00 00 	mov    0x188(%r14),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
     697:	49 8d be 88 01 00 00 	lea    0x188(%r14),%rdi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     69e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
     6a5:	00 00 80 
     6a8:	48 39 f1             	cmp    %rsi,%rcx
     6ab:	0f 84 d7 00 00 00    	je     788 <scif_munmap+0x288>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     6b1:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     6b5:	48 89 c8             	mov    %rcx,%rax
     6b8:	f0 49 0f b1 96 88 01 	lock cmpxchg %rdx,0x188(%r14)
     6bf:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     6c1:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     6c4:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     6c7:	0f 84 df fe ff ff    	je     5ac <scif_munmap+0xac>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     6cd:	48 39 f2             	cmp    %rsi,%rdx
     6d0:	0f 84 b2 00 00 00    	je     788 <scif_munmap+0x288>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     6d6:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     6da:	48 89 d0             	mov    %rdx,%rax
     6dd:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     6e2:	48 39 c2             	cmp    %rax,%rdx
     6e5:	0f 84 c1 fe ff ff    	je     5ac <scif_munmap+0xac>
     6eb:	48 89 c2             	mov    %rax,%rdx
     6ee:	eb dd                	jmp    6cd <scif_munmap+0x1cd>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
     6f0:	48 8b 45 08          	mov    0x8(%rbp),%rax
     6f4:	4c 89 f1             	mov    %r14,%rcx
     6f7:	ba a7 00 00 00       	mov    $0xa7,%edx
     6fc:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
     703:	4d 8b 8e 88 01 00 00 	mov    0x188(%r14),%r9
     70a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     711:	4c 89 95 38 ff ff ff 	mov    %r10,-0xc8(%rbp)
     718:	45 0f b7 06          	movzwl (%r14),%r8d
     71c:	48 89 04 24          	mov    %rax,(%rsp)
     720:	31 c0                	xor    %eax,%eax
     722:	e8 00 00 00 00       	callq  727 <scif_munmap+0x227>
     727:	49 8b 8e 88 01 00 00 	mov    0x188(%r14),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     72e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
     735:	00 00 80 
     738:	4c 8b 95 38 ff ff ff 	mov    -0xc8(%rbp),%r10
     73f:	48 39 f1             	cmp    %rsi,%rcx
     742:	0f 84 da fe ff ff    	je     622 <scif_munmap+0x122>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     748:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     74c:	48 89 c8             	mov    %rcx,%rax
     74f:	f0 49 0f b1 12       	lock cmpxchg %rdx,(%r10)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     754:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     757:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     75a:	0f 84 c2 fe ff ff    	je     622 <scif_munmap+0x122>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
     760:	48 39 f2             	cmp    %rsi,%rdx
     763:	0f 84 b9 fe ff ff    	je     622 <scif_munmap+0x122>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
     769:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
     76d:	48 89 d0             	mov    %rdx,%rax
     770:	f0 49 0f b1 0a       	lock cmpxchg %rcx,(%r10)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
     775:	48 39 c2             	cmp    %rax,%rdx
     778:	0f 84 a4 fe ff ff    	je     622 <scif_munmap+0x122>
     77e:	48 89 c2             	mov    %rax,%rdx
     781:	eb dd                	jmp    760 <scif_munmap+0x260>
     783:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
     788:	49 8d 86 68 01 00 00 	lea    0x168(%r14),%rax
     78f:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
     796:	48 89 c7             	mov    %rax,%rdi
     799:	48 89 85 30 ff ff ff 	mov    %rax,-0xd0(%rbp)
     7a0:	e8 00 00 00 00       	callq  7a5 <scif_munmap+0x2a5>
		if (SCIFDEV_STOPPED == dev->sd_state ||
     7a5:	41 8b 46 04          	mov    0x4(%r14),%eax
     7a9:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
			SCIFDEV_STOPPING == dev->sd_state ||
     7b0:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
     7b3:	83 fa 01             	cmp    $0x1,%edx
     7b6:	76 1a                	jbe    7d2 <scif_munmap+0x2d2>
     7b8:	83 f8 01             	cmp    $0x1,%eax
     7bb:	74 15                	je     7d2 <scif_munmap+0x2d2>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
     7bd:	49 8b 86 88 01 00 00 	mov    0x188(%r14),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
     7c4:	48 85 c0             	test   %rax,%rax
     7c7:	78 28                	js     7f1 <scif_munmap+0x2f1>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
     7c9:	f0 49 83 86 88 01 00 	lock addq $0x1,0x188(%r14)
     7d0:	00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
     7d2:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
     7d9:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
     7e0:	e8 00 00 00 00       	callq  7e5 <scif_munmap+0x2e5>
     7e5:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     7ec:	e9 bb fd ff ff       	jmpq   5ac <scif_munmap+0xac>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
     7f1:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 7f7 <scif_munmap+0x2f7>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
     7f7:	31 d2                	xor    %edx,%edx
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
     7f9:	c7 85 7c ff ff ff 2e 	movl   $0x2e,-0x84(%rbp)
     800:	00 00 00 
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
     803:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     80a:	48 8d b5 74 ff ff ff 	lea    -0x8c(%rbp),%rsi
     811:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
     818:	49 c7 86 b0 01 00 00 	movq   $0x2,0x1b0(%r14)
     81f:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
     823:	66 89 85 74 ff ff ff 	mov    %ax,-0x8c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
     82a:	31 c0                	xor    %eax,%eax
     82c:	66 89 85 78 ff ff ff 	mov    %ax,-0x88(%rbp)
			notif_msg.payload[0] = dev->sd_node;
     833:	41 0f b7 06          	movzwl (%r14),%eax
     837:	48 89 45 80          	mov    %rax,-0x80(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
     83b:	e8 00 00 00 00       	callq  840 <scif_munmap+0x340>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
     840:	49 8b 86 b0 01 00 00 	mov    0x1b0(%r14),%rax
     847:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     84e:	48 83 f8 02          	cmp    $0x2,%rax
     852:	74 6d                	je     8c1 <scif_munmap+0x3c1>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
     854:	48 83 f8 03          	cmp    $0x3,%rax
     858:	0f 85 6b ff ff ff    	jne    7c9 <scif_munmap+0x2c9>
				dev->sd_state = SCIFDEV_RUNNING;
     85e:	41 c7 46 04 02 00 00 	movl   $0x2,0x4(%r14)
     865:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
     866:	f0 41 80 a6 8f 01 00 	lock andb $0x7f,0x18f(%r14)
     86d:	00 7f 
     86f:	e9 55 ff ff ff       	jmpq   7c9 <scif_munmap+0x2c9>
	might_sleep();
	pr_debug("SCIFAPI munmap: vma_start 0x%lx vma_end 0x%lx\n", 
			((vma)->vm_start), ((vma)->vm_end));
	/* used to be a BUG_ON(), prefer keeping the kernel alive */
	if (!vmapvt) {
		WARN_ON(1);
     874:	be c6 0a 00 00       	mov    $0xac6,%esi
     879:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     880:	e8 00 00 00 00       	callq  885 <scif_munmap+0x385>
		printk(KERN_ERR "SCIFAPI munmap: vma_start 0x%lx vma_end 0x%lx\n", 
     885:	49 8b 57 10          	mov    0x10(%r15),%rdx
     889:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     890:	31 c0                	xor    %eax,%eax
     892:	49 8b 77 08          	mov    0x8(%r15),%rsi
     896:	e8 00 00 00 00       	callq  89b <scif_munmap+0x39b>
     89b:	e9 c4 fd ff ff       	jmpq   664 <scif_munmap+0x164>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);

	if ((err = micscif_query_window(&req)))
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
     8a0:	89 c1                	mov    %eax,%ecx
     8a2:	ba dd 0a 00 00       	mov    $0xadd,%edx
     8a7:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     8ae:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     8b5:	31 c0                	xor    %eax,%eax
     8b7:	e8 00 00 00 00       	callq  8bc <scif_munmap+0x3bc>
     8bc:	e9 2b fd ff ff       	jmpq   5ec <scif_munmap+0xec>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
     8c1:	48 8d bd 48 ff ff ff 	lea    -0xb8(%rbp),%rdi
     8c8:	30 c0                	xor    %al,%al
     8ca:	b9 0a 00 00 00       	mov    $0xa,%ecx
     8cf:	f3 ab                	rep stos %eax,%es:(%rdi)
     8d1:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
     8d8:	00 00 00 00 

DECLARE_PER_CPU(struct task_struct *, current_task);

static __always_inline struct task_struct *get_current(void)
{
	return percpu_read_stable(current_task);
     8dc:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
     8e3:	00 00 
     8e5:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
     8ec:	48 8d 85 48 ff ff ff 	lea    -0xb8(%rbp),%rax
     8f3:	48 83 c0 18          	add    $0x18,%rax
     8f7:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
     8fe:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
     905:	49 8d 86 98 01 00 00 	lea    0x198(%r14),%rax
     90c:	48 89 85 28 ff ff ff 	mov    %rax,-0xd8(%rbp)
     913:	eb 0f                	jmp    924 <scif_munmap+0x424>
     915:	0f 1f 00             	nopl   (%rax)
     918:	e8 00 00 00 00       	callq  91d <scif_munmap+0x41d>
     91d:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     924:	48 8b bd 28 ff ff ff 	mov    -0xd8(%rbp),%rdi
     92b:	ba 02 00 00 00       	mov    $0x2,%edx
     930:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
     937:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
     93e:	e8 00 00 00 00       	callq  943 <scif_munmap+0x443>
     943:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
     94a:	02 
     94b:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     952:	74 c4                	je     918 <scif_munmap+0x418>
     954:	48 8b bd 28 ff ff ff 	mov    -0xd8(%rbp),%rdi
     95b:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
     962:	4c 89 85 38 ff ff ff 	mov    %r8,-0xc8(%rbp)
     969:	e8 00 00 00 00       	callq  96e <scif_munmap+0x46e>
     96e:	49 8b 86 b0 01 00 00 	mov    0x1b0(%r14),%rax
     975:	4c 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%r8
     97c:	e9 d3 fe ff ff       	jmpq   854 <scif_munmap+0x354>
     981:	66 66 66 66 66 66 2e 	data32 data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
     988:	0f 1f 84 00 00 00 00 
     98f:	00 

0000000000000990 <__scif_open>:
 * Create a SCIF end point and set the state to UNBOUND.  This function
 * returns the address of the end point data structure.
 */
scif_epd_t
__scif_open(void)
{
     990:	55                   	push   %rbp
     991:	48 89 e5             	mov    %rsp,%rbp
     994:	53                   	push   %rbx
     995:	48 83 ec 08          	sub    $0x8,%rsp
     999:	e8 00 00 00 00       	callq  99e <__scif_open+0xe>
	struct endpt *ep;

	might_sleep();
     99e:	e8 00 00 00 00       	callq  9a3 <__scif_open+0x13>
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
     9a3:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 9aa <__scif_open+0x1a>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
     9aa:	48 85 ff             	test   %rdi,%rdi
     9ad:	0f 84 9d 00 00 00    	je     a50 <__scif_open+0xc0>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
     9b3:	ba 90 02 00 00       	mov    $0x290,%edx
     9b8:	be d0 80 00 00       	mov    $0x80d0,%esi
     9bd:	e8 00 00 00 00       	callq  9c2 <__scif_open+0x32>
	if ((ep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL)) == NULL) {
     9c2:	48 85 c0             	test   %rax,%rax
     9c5:	48 89 c3             	mov    %rax,%rbx
     9c8:	0f 84 b9 00 00 00    	je     a87 <__scif_open+0xf7>
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
     9ce:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 9d5 <__scif_open+0x45>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
     9d5:	48 85 ff             	test   %rdi,%rdi
     9d8:	0f 84 87 00 00 00    	je     a65 <__scif_open+0xd5>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
     9de:	ba b0 00 00 00       	mov    $0xb0,%edx
     9e3:	be d0 80 00 00       	mov    $0x80d0,%esi
     9e8:	e8 00 00 00 00       	callq  9ed <__scif_open+0x5d>
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point descriptor\n");
		goto err_ep_alloc;
	}

	if ((ep->qp_info.qp = (struct micscif_qp *)
     9ed:	48 85 c0             	test   %rax,%rax
     9f0:	48 89 43 10          	mov    %rax,0x10(%rbx)
     9f4:	74 79                	je     a6f <__scif_open+0xdf>
			kzalloc(sizeof(struct micscif_qp), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point queue pointer\n");
		goto err_qp_alloc;
	}

	spin_lock_init(&ep->lock);
     9f6:	31 c0                	xor    %eax,%eax
	mutex_init (&ep->sendlock);
     9f8:	48 c7 c2 00 00 00 00 	mov    $0x0,%rdx
     9ff:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
			kzalloc(sizeof(struct micscif_qp), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point queue pointer\n");
		goto err_qp_alloc;
	}

	spin_lock_init(&ep->lock);
     a06:	66 89 43 04          	mov    %ax,0x4(%rbx)
	mutex_init (&ep->sendlock);
     a0a:	48 8d bb 00 02 00 00 	lea    0x200(%rbx),%rdi
     a11:	e8 00 00 00 00       	callq  a16 <__scif_open+0x86>
	mutex_init (&ep->recvlock);
     a16:	48 8d bb 20 02 00 00 	lea    0x220(%rbx),%rdi
     a1d:	48 c7 c2 00 00 00 00 	mov    $0x0,%rdx
     a24:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     a2b:	e8 00 00 00 00       	callq  a30 <__scif_open+0xa0>

	if (micscif_rma_ep_init(ep) < 0) {
     a30:	48 89 df             	mov    %rbx,%rdi
     a33:	e8 00 00 00 00       	callq  a38 <__scif_open+0xa8>
     a38:	85 c0                	test   %eax,%eax
     a3a:	78 5d                	js     a99 <__scif_open+0x109>
		printk(KERN_ERR "SCIFAPI _open: RMA EP Init failed\n");
		goto err_rma_init;
	}

	ep->state = SCIFEP_UNBOUND;
     a3c:	c7 03 01 00 00 00    	movl   $0x1,(%rbx)
	pr_debug("SCIFAPI open: ep %p success\n", ep);
	return (scif_epd_t)ep;
     a42:	48 89 d8             	mov    %rbx,%rax
	kfree(ep->qp_info.qp);
err_qp_alloc:
	kfree(ep);
err_ep_alloc:
	return NULL;
}
     a45:	48 83 c4 08          	add    $0x8,%rsp
     a49:	5b                   	pop    %rbx
     a4a:	5d                   	pop    %rbp
     a4b:	c3                   	retq   
     a4c:	0f 1f 40 00          	nopl   0x0(%rax)
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
     a50:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # a57 <__scif_open+0xc7>

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
     a57:	bb 10 00 00 00       	mov    $0x10,%ebx
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
     a5c:	48 85 ff             	test   %rdi,%rdi
     a5f:	0f 85 79 ff ff ff    	jne    9de <__scif_open+0x4e>
	if ((ep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point descriptor\n");
		goto err_ep_alloc;
	}

	if ((ep->qp_info.qp = (struct micscif_qp *)
     a65:	48 c7 43 10 10 00 00 	movq   $0x10,0x10(%rbx)
     a6c:	00 
     a6d:	eb 87                	jmp    9f6 <__scif_open+0x66>
			kzalloc(sizeof(struct micscif_qp), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point queue pointer\n");
     a6f:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     a76:	e8 00 00 00 00       	callq  a7b <__scif_open+0xeb>
	return (scif_epd_t)ep;

err_rma_init:
	kfree(ep->qp_info.qp);
err_qp_alloc:
	kfree(ep);
     a7b:	48 89 df             	mov    %rbx,%rdi
     a7e:	e8 00 00 00 00       	callq  a83 <__scif_open+0xf3>
err_ep_alloc:
	return NULL;
     a83:	31 c0                	xor    %eax,%eax
     a85:	eb be                	jmp    a45 <__scif_open+0xb5>
{
	struct endpt *ep;

	might_sleep();
	if ((ep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point descriptor\n");
     a87:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     a8e:	31 c0                	xor    %eax,%eax
     a90:	e8 00 00 00 00       	callq  a95 <__scif_open+0x105>
err_rma_init:
	kfree(ep->qp_info.qp);
err_qp_alloc:
	kfree(ep);
err_ep_alloc:
	return NULL;
     a95:	31 c0                	xor    %eax,%eax
	struct endpt *ep;

	might_sleep();
	if ((ep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "SCIFAPI open: kzalloc fail on scif end point descriptor\n");
		goto err_ep_alloc;
     a97:	eb ac                	jmp    a45 <__scif_open+0xb5>
	spin_lock_init(&ep->lock);
	mutex_init (&ep->sendlock);
	mutex_init (&ep->recvlock);

	if (micscif_rma_ep_init(ep) < 0) {
		printk(KERN_ERR "SCIFAPI _open: RMA EP Init failed\n");
     a99:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     aa0:	31 c0                	xor    %eax,%eax
     aa2:	e8 00 00 00 00       	callq  aa7 <__scif_open+0x117>
	ep->state = SCIFEP_UNBOUND;
	pr_debug("SCIFAPI open: ep %p success\n", ep);
	return (scif_epd_t)ep;

err_rma_init:
	kfree(ep->qp_info.qp);
     aa7:	48 8b 7b 10          	mov    0x10(%rbx),%rdi
     aab:	e8 00 00 00 00       	callq  ab0 <__scif_open+0x120>
     ab0:	eb c9                	jmp    a7b <__scif_open+0xeb>
     ab2:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
     ab9:	1f 84 00 00 00 00 00 

0000000000000ac0 <scif_open>:
	return NULL;
}

scif_epd_t
scif_open(void)
{
     ac0:	55                   	push   %rbp
     ac1:	48 89 e5             	mov    %rsp,%rbp
     ac4:	53                   	push   %rbx
     ac5:	48 83 ec 08          	sub    $0x8,%rsp
     ac9:	e8 00 00 00 00       	callq  ace <scif_open+0xe>
	struct endpt *ep;
	ep = (struct endpt *)__scif_open();
     ace:	e8 00 00 00 00       	callq  ad3 <scif_open+0x13>
	if (ep)
     ad3:	48 85 c0             	test   %rax,%rax

scif_epd_t
scif_open(void)
{
	struct endpt *ep;
	ep = (struct endpt *)__scif_open();
     ad6:	48 89 c3             	mov    %rax,%rbx
	if (ep)
     ad9:	74 0c                	je     ae7 <scif_open+0x27>
		kref_init(&(ep->ref_count));
     adb:	48 8d b8 70 01 00 00 	lea    0x170(%rax),%rdi
     ae2:	e8 00 00 00 00       	callq  ae7 <scif_open+0x27>
	return (scif_epd_t)ep;
}
     ae7:	48 83 c4 08          	add    $0x8,%rsp
     aeb:	48 89 d8             	mov    %rbx,%rax
     aee:	5b                   	pop    %rbx
     aef:	5d                   	pop    %rbp
     af0:	c3                   	retq   
     af1:	66 66 66 66 66 66 2e 	data32 data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
     af8:	0f 1f 84 00 00 00 00 
     aff:	00 

0000000000000b00 <__scif_close>:
 * DMA and message traffic has ended the end point must be placed in a zombie
 * state and wait for the other side to also release it's memory references.
 */
int
__scif_close(scif_epd_t epd)
{
     b00:	55                   	push   %rbp
     b01:	48 89 e5             	mov    %rsp,%rbp
     b04:	41 57                	push   %r15
     b06:	41 56                	push   %r14
     b08:	41 55                	push   %r13
     b0a:	41 54                	push   %r12
     b0c:	53                   	push   %rbx
     b0d:	48 83 ec 78          	sub    $0x78,%rsp
     b11:	e8 00 00 00 00       	callq  b16 <__scif_close+0x16>
	struct list_head *pos, *tmpq;
	unsigned long sflags;
	enum endptstate oldstate;
	int err;

	pr_debug("SCIFAPI close: ep %p %s\n", ep, scif_ep_states[ep->state]);
     b16:	8b 07                	mov    (%rdi),%eax
 * DMA and message traffic has ended the end point must be placed in a zombie
 * state and wait for the other side to also release it's memory references.
 */
int
__scif_close(scif_epd_t epd)
{
     b18:	49 89 fd             	mov    %rdi,%r13
	enum endptstate oldstate;
	int err;

	pr_debug("SCIFAPI close: ep %p %s\n", ep, scif_ep_states[ep->state]);

	might_sleep();
     b1b:	e8 00 00 00 00       	callq  b20 <__scif_close+0x20>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
     b20:	49 8b 9d 48 01 00 00 	mov    0x148(%r13),%rbx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
     b27:	48 85 db             	test   %rbx,%rbx
     b2a:	0f 85 10 07 00 00    	jne    1240 <__scif_close+0x740>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
     b30:	4d 8d 65 04          	lea    0x4(%r13),%r12

	spin_lock_irqsave(&ep->lock, sflags);
     b34:	4c 89 e7             	mov    %r12,%rdi
     b37:	e8 00 00 00 00       	callq  b3c <__scif_close+0x3c>
	oldstate = ep->state;
     b3c:	41 8b 55 00          	mov    0x0(%r13),%edx

	ep->state = SCIFEP_CLOSING;
     b40:	41 c7 45 00 07 00 00 	movl   $0x7,0x0(%r13)
     b47:	00 

	might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);

	spin_lock_irqsave(&ep->lock, sflags);
     b48:	48 89 c3             	mov    %rax,%rbx
	oldstate = ep->state;

	ep->state = SCIFEP_CLOSING;

	switch (oldstate) {
     b4b:	83 fa 0a             	cmp    $0xa,%edx
     b4e:	0f 87 14 01 00 00    	ja     c68 <__scif_close+0x168>
     b54:	ff 24 d5 00 00 00 00 	jmpq   *0x0(,%rdx,8)
		// and does a standard teardown.
		ts = jiffies;
		while (ep->state == SCIFEP_MAPPING) {
			cpu_relax();
			if (time_after((unsigned long)jiffies,ts + NODE_ALIVE_TIMEOUT)) {
				printk(KERN_ERR "%s %d ep->state %d\n", __func__, __LINE__, ep->state);
     b5b:	41 8b 4d 00          	mov    0x0(%r13),%ecx
     b5f:	ba c4 00 00 00       	mov    $0xc4,%edx
     b64:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     b6b:	31 c0                	xor    %eax,%eax
     b6d:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     b74:	e8 00 00 00 00       	callq  b79 <__scif_close+0x79>
				ep->state = SCIFEP_BOUND;
     b79:	41 c7 45 00 02 00 00 	movl   $0x2,0x0(%r13)
     b80:	00 
     b81:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
				break;
			}
		}

		init_waitqueue_head(&ep->disconwq);	// Wait for connection queue
     b88:	4d 8d b5 a0 01 00 00 	lea    0x1a0(%r13),%r14
     b8f:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
     b96:	4c 89 f7             	mov    %r14,%rdi
     b99:	e8 00 00 00 00       	callq  b9e <__scif_close+0x9e>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
     b9e:	48 89 de             	mov    %rbx,%rsi
     ba1:	4c 89 e7             	mov    %r12,%rdi
     ba4:	e8 00 00 00 00       	callq  ba9 <__scif_close+0xa9>
		spin_unlock_irqrestore(&ep->lock, sflags);

		micscif_unregister_all_windows(epd);
     ba9:	4c 89 ef             	mov    %r13,%rdi
     bac:	e8 00 00 00 00       	callq  bb1 <__scif_close+0xb1>

		// Remove from the connected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
     bb1:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     bb8:	e8 00 00 00 00       	callq  bbd <__scif_close+0xbd>
		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
     bbd:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # bc4 <__scif_close+0xc4>
		spin_unlock_irqrestore(&ep->lock, sflags);

		micscif_unregister_all_windows(epd);

		// Remove from the connected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
     bc4:	48 89 c3             	mov    %rax,%rbx
		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
     bc7:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
     bce:	48 8b 11             	mov    (%rcx),%rdx
     bd1:	74 34                	je     c07 <__scif_close+0x107>
			tmpep = list_entry(pos, struct endpt, list);
     bd3:	4c 8d b9 c0 fd ff ff 	lea    -0x240(%rcx),%r15
			if (tmpep == ep) {
     bda:	4d 39 fd             	cmp    %r15,%r13
     bdd:	75 1c                	jne    bfb <__scif_close+0xfb>
     bdf:	e9 fe 06 00 00       	jmpq   12e2 <__scif_close+0x7e2>
     be4:	0f 1f 40 00          	nopl   0x0(%rax)
		micscif_unregister_all_windows(epd);

		// Remove from the connected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
			tmpep = list_entry(pos, struct endpt, list);
     be8:	4c 8d ba c0 fd ff ff 	lea    -0x240(%rdx),%r15
			if (tmpep == ep) {
     bef:	4d 39 fd             	cmp    %r15,%r13
     bf2:	0f 84 f0 06 00 00    	je     12e8 <__scif_close+0x7e8>

		micscif_unregister_all_windows(epd);

		// Remove from the connected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
     bf8:	48 89 c2             	mov    %rax,%rdx
     bfb:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
     c02:	48 8b 02             	mov    (%rdx),%rax
     c05:	75 e1                	jne    be8 <__scif_close+0xe8>
			// The other side has completed the disconnect before
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock and
			// proceed to teardown the end point below.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
     c07:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # c0e <__scif_close+0x10e>
     c0e:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
     c15:	48 8b 01             	mov    (%rcx),%rax
     c18:	74 3c                	je     c56 <__scif_close+0x156>
				tmpep = list_entry(pos, struct endpt, list);
     c1a:	48 8d 91 c0 fd ff ff 	lea    -0x240(%rcx),%rdx
				if (tmpep == ep) {
     c21:	49 39 d5             	cmp    %rdx,%r13
     c24:	0f 85 e9 04 00 00    	jne    1113 <__scif_close+0x613>
     c2a:	48 89 c2             	mov    %rax,%rdx
     c2d:	48 89 c8             	mov    %rcx,%rax
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     c30:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
     c37:	00 ad de 
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
     c3a:	48 8b 48 08          	mov    0x8(%rax),%rcx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
     c3e:	48 89 4a 08          	mov    %rcx,0x8(%rdx)
	prev->next = next;
     c42:	48 89 11             	mov    %rdx,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     c45:	48 89 30             	mov    %rsi,(%rax)
	entry->prev = LIST_POISON2;
     c48:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
     c4f:	00 ad de 
     c52:	48 89 70 08          	mov    %rsi,0x8(%rax)
     c56:	48 89 de             	mov    %rbx,%rsi
     c59:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     c60:	e8 00 00 00 00       	callq  c65 <__scif_close+0x165>
     c65:	0f 1f 00             	nopl   (%rax)
		wake_up_interruptible(&ep->conwq);
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	}
	if (ep->port.port && !ep->accepted_ep)
     c68:	41 0f b7 45 08       	movzwl 0x8(%r13),%eax
     c6d:	66 85 c0             	test   %ax,%ax
     c70:	75 5e                	jne    cd0 <__scif_close+0x1d0>
     c72:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		put_scif_port(ep->port.port);
	micscif_dec_node_refcnt(ep->remote_dev, 1);
     c78:	49 8b 9d 48 01 00 00 	mov    0x148(%r13),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
     c7f:	48 85 db             	test   %rbx,%rbx
     c82:	74 21                	je     ca5 <__scif_close+0x1a5>
		if (unlikely((atomic_long_sub_return(cnt, 
     c84:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
     c8b:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
     c92:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
     c99:	00 00 
     c9b:	48 83 e8 01          	sub    $0x1,%rax
     c9f:	0f 88 8d 07 00 00    	js     1432 <__scif_close+0x932>
	micscif_teardown_ep(ep);
     ca5:	4c 89 ef             	mov    %r13,%rdi
     ca8:	e8 00 00 00 00       	callq  cad <__scif_close+0x1ad>
	micscif_add_epd_to_zombie_list(ep, !MI_EPLOCK_HELD);
     cad:	4c 89 ef             	mov    %r13,%rdi
     cb0:	31 f6                	xor    %esi,%esi
     cb2:	e8 00 00 00 00       	callq  cb7 <__scif_close+0x1b7>
	return 0;
}
     cb7:	48 83 c4 78          	add    $0x78,%rsp
     cbb:	31 c0                	xor    %eax,%eax
     cbd:	5b                   	pop    %rbx
     cbe:	41 5c                	pop    %r12
     cc0:	41 5d                	pop    %r13
     cc2:	41 5e                	pop    %r14
     cc4:	41 5f                	pop    %r15
     cc6:	5d                   	pop    %rbp
     cc7:	c3                   	retq   
     cc8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
     ccf:	00 
		wake_up_interruptible(&ep->conwq);
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	}
	if (ep->port.port && !ep->accepted_ep)
     cd0:	41 80 bd 60 01 00 00 	cmpb   $0x0,0x160(%r13)
     cd7:	00 
     cd8:	75 9e                	jne    c78 <__scif_close+0x178>
		put_scif_port(ep->port.port);
     cda:	0f b7 f8             	movzwl %ax,%edi
     cdd:	e8 00 00 00 00       	callq  ce2 <__scif_close+0x1e2>
     ce2:	eb 94                	jmp    c78 <__scif_close+0x178>
     ce4:	0f 1f 40 00          	nopl   0x0(%rax)
     ce8:	48 89 c6             	mov    %rax,%rsi
     ceb:	4c 89 e7             	mov    %r12,%rdi
     cee:	e8 00 00 00 00       	callq  cf3 <__scif_close+0x1f3>
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == oldstate);
	case SCIFEP_CLOSED:
	case SCIFEP_DISCONNECTED:
		spin_unlock_irqrestore(&ep->lock, sflags);
		micscif_unregister_all_windows(epd);
     cf3:	4c 89 ef             	mov    %r13,%rdi
     cf6:	e8 00 00 00 00       	callq  cfb <__scif_close+0x1fb>
		// Remove from the disconnected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
     cfb:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     d02:	e8 00 00 00 00       	callq  d07 <__scif_close+0x207>
		list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
     d07:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # d0e <__scif_close+0x20e>
     d0e:	48 81 ff 00 00 00 00 	cmp    $0x0,%rdi
     d15:	48 8b 17             	mov    (%rdi),%rdx
     d18:	74 35                	je     d4f <__scif_close+0x24f>
			tmpep = list_entry(pos, struct endpt, list);
     d1a:	48 8d 8f c0 fd ff ff 	lea    -0x240(%rdi),%rcx
			if (tmpep == ep) {
     d21:	49 39 cd             	cmp    %rcx,%r13
     d24:	75 1d                	jne    d43 <__scif_close+0x243>
     d26:	e9 b8 04 00 00       	jmpq   11e3 <__scif_close+0x6e3>
     d2b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		spin_unlock_irqrestore(&ep->lock, sflags);
		micscif_unregister_all_windows(epd);
		// Remove from the disconnected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
			tmpep = list_entry(pos, struct endpt, list);
     d30:	48 8d ba c0 fd ff ff 	lea    -0x240(%rdx),%rdi
			if (tmpep == ep) {
     d37:	49 39 fd             	cmp    %rdi,%r13
     d3a:	0f 84 b0 04 00 00    	je     11f0 <__scif_close+0x6f0>
	case SCIFEP_DISCONNECTED:
		spin_unlock_irqrestore(&ep->lock, sflags);
		micscif_unregister_all_windows(epd);
		// Remove from the disconnected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
     d40:	48 89 ca             	mov    %rcx,%rdx
     d43:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
     d4a:	48 8b 0a             	mov    (%rdx),%rcx
     d4d:	75 e1                	jne    d30 <__scif_close+0x230>
     d4f:	48 89 c6             	mov    %rax,%rsi
     d52:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     d59:	e8 00 00 00 00       	callq  d5e <__scif_close+0x25e>
		wake_up_interruptible(&ep->conwq);
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	}
	if (ep->port.port && !ep->accepted_ep)
     d5e:	41 0f b7 45 08       	movzwl 0x8(%r13),%eax
     d63:	66 85 c0             	test   %ax,%ax
     d66:	0f 84 0c ff ff ff    	je     c78 <__scif_close+0x178>
     d6c:	e9 5f ff ff ff       	jmpq   cd0 <__scif_close+0x1d0>
     d71:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
     d78:	48 89 c6             	mov    %rax,%rsi
     d7b:	4c 89 e7             	mov    %r12,%rdi
     d7e:	e8 00 00 00 00       	callq  d83 <__scif_close+0x283>
		struct conreq *conreq;
		struct nodemsg msg;
		struct endpt *aep;

		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_eplock, sflags);
     d83:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     d8a:	e8 00 00 00 00       	callq  d8f <__scif_close+0x28f>
     d8f:	49 89 c6             	mov    %rax,%r14

		// remove from listen list
		list_for_each_safe(pos, tmpq, &ms_info.mi_listen) {
     d92:	48 8b 05 00 00 00 00 	mov    0x0(%rip),%rax        # d99 <__scif_close+0x299>
     d99:	48 3d 00 00 00 00    	cmp    $0x0,%rax
     d9f:	48 8b 10             	mov    (%rax),%rdx
     da2:	75 1e                	jne    dc2 <__scif_close+0x2c2>
     da4:	eb 6a                	jmp    e10 <__scif_close+0x310>
     da6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
     dad:	00 00 00 
     db0:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
     db7:	48 8b 0a             	mov    (%rdx),%rcx
     dba:	48 89 d0             	mov    %rdx,%rax
     dbd:	74 51                	je     e10 <__scif_close+0x310>
     dbf:	48 89 ca             	mov    %rcx,%rdx
			tmpep = list_entry(pos, struct endpt, list);
     dc2:	48 8d 88 c0 fd ff ff 	lea    -0x240(%rax),%rcx
			if (tmpep == ep) {
     dc9:	49 39 cd             	cmp    %rcx,%r13
     dcc:	75 e2                	jne    db0 <__scif_close+0x2b0>
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
     dce:	48 8b 30             	mov    (%rax),%rsi

		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_eplock, sflags);

		// remove from listen list
		list_for_each_safe(pos, tmpq, &ms_info.mi_listen) {
     dd1:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
     dd8:	48 8b 48 08          	mov    0x8(%rax),%rcx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
     ddc:	48 89 4e 08          	mov    %rcx,0x8(%rsi)
	prev->next = next;
     de0:	48 89 31             	mov    %rsi,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     de3:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
     dea:	00 ad de 
     ded:	48 89 30             	mov    %rsi,(%rax)
	entry->prev = LIST_POISON2;
     df0:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
     df7:	00 ad de 
     dfa:	48 89 70 08          	mov    %rsi,0x8(%rax)
     dfe:	48 89 d0             	mov    %rdx,%rax
     e01:	48 8b 0a             	mov    (%rdx),%rcx
     e04:	75 b9                	jne    dbf <__scif_close+0x2bf>
     e06:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
     e0d:	00 00 00 
			if (tmpep == ep) {
				list_del(pos);
			}
		}
		// Remove any dangling accepts
		while (ep->acceptcnt) {
     e10:	41 8b 8d 60 02 00 00 	mov    0x260(%r13),%ecx
     e17:	85 c9                	test   %ecx,%ecx
     e19:	0f 84 89 01 00 00    	je     fa8 <__scif_close+0x4a8>
     e1f:	90                   	nop
			aep = list_first_entry(&ep->li_accept, struct endpt, liacceptlist);
     e20:	4d 8b bd 50 02 00 00 	mov    0x250(%r13),%r15
			BUG_ON(!aep);
     e27:	4c 89 fb             	mov    %r15,%rbx
     e2a:	48 81 eb 68 02 00 00 	sub    $0x268,%rbx
     e31:	0f 84 a9 04 00 00    	je     12e0 <__scif_close+0x7e0>
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
     e37:	49 8b 47 08          	mov    0x8(%r15),%rax
     e3b:	49 8b 17             	mov    (%r15),%rdx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
     e3e:	48 89 42 08          	mov    %rax,0x8(%rdx)
	prev->next = next;
     e42:	48 89 10             	mov    %rdx,(%rax)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     e45:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
     e4c:	00 ad de 
     e4f:	49 89 07             	mov    %rax,(%r15)
	entry->prev = LIST_POISON2;
     e52:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
     e59:	00 ad de 
     e5c:	49 89 47 08          	mov    %rax,0x8(%r15)
			list_del(&aep->liacceptlist);
			if (aep->port.port && !aep->accepted_ep)
     e60:	41 0f b7 87 a0 fd ff 	movzwl -0x260(%r15),%eax
     e67:	ff 
     e68:	66 85 c0             	test   %ax,%ax
     e6b:	74 0e                	je     e7b <__scif_close+0x37b>
     e6d:	41 80 bf f8 fe ff ff 	cmpb   $0x0,-0x108(%r15)
     e74:	00 
     e75:	0f 84 b5 03 00 00    	je     1230 <__scif_close+0x730>
				put_scif_port(aep->port.port);
			list_for_each_safe(pos, tmpq, &ms_info.mi_uaccept) {
     e7b:	48 8b 15 00 00 00 00 	mov    0x0(%rip),%rdx        # e82 <__scif_close+0x382>
     e82:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
     e89:	48 8b 0a             	mov    (%rdx),%rcx
     e8c:	74 31                	je     ebf <__scif_close+0x3bf>
				tmpep = list_entry(pos, struct endpt, miacceptlist);
     e8e:	48 8d 82 88 fd ff ff 	lea    -0x278(%rdx),%rax
				if (tmpep == aep) {
     e95:	48 39 c3             	cmp    %rax,%rbx
     e98:	75 19                	jne    eb3 <__scif_close+0x3b3>
     e9a:	e9 d7 02 00 00       	jmpq   1176 <__scif_close+0x676>
     e9f:	90                   	nop
			BUG_ON(!aep);
			list_del(&aep->liacceptlist);
			if (aep->port.port && !aep->accepted_ep)
				put_scif_port(aep->port.port);
			list_for_each_safe(pos, tmpq, &ms_info.mi_uaccept) {
				tmpep = list_entry(pos, struct endpt, miacceptlist);
     ea0:	48 8d 91 88 fd ff ff 	lea    -0x278(%rcx),%rdx
				if (tmpep == aep) {
     ea7:	48 39 d3             	cmp    %rdx,%rbx
     eaa:	0f 84 d0 02 00 00    	je     1180 <__scif_close+0x680>
			aep = list_first_entry(&ep->li_accept, struct endpt, liacceptlist);
			BUG_ON(!aep);
			list_del(&aep->liacceptlist);
			if (aep->port.port && !aep->accepted_ep)
				put_scif_port(aep->port.port);
			list_for_each_safe(pos, tmpq, &ms_info.mi_uaccept) {
     eb0:	48 89 c1             	mov    %rax,%rcx
     eb3:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
     eba:	48 8b 01             	mov    (%rcx),%rax
     ebd:	75 e1                	jne    ea0 <__scif_close+0x3a0>
     ebf:	4c 89 f6             	mov    %r14,%rsi
     ec2:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     ec9:	e8 00 00 00 00       	callq  ece <__scif_close+0x3ce>
					list_del(pos);
					break;
				}
			}
			spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
			spin_lock_irqsave(&ms_info.mi_connlock, sflags);
     ece:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     ed5:	e8 00 00 00 00       	callq  eda <__scif_close+0x3da>
			list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
     eda:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # ee1 <__scif_close+0x3e1>
     ee1:	48 81 ff 00 00 00 00 	cmp    $0x0,%rdi
     ee8:	48 8b 0f             	mov    (%rdi),%rcx
     eeb:	74 32                	je     f1f <__scif_close+0x41f>
				tmpep = list_entry(pos, struct endpt, list);
     eed:	48 8d 97 c0 fd ff ff 	lea    -0x240(%rdi),%rdx
				if (tmpep == aep) {
     ef4:	48 39 d3             	cmp    %rdx,%rbx
     ef7:	75 1a                	jne    f13 <__scif_close+0x413>
     ef9:	e9 25 02 00 00       	jmpq   1123 <__scif_close+0x623>
     efe:	66 90                	xchg   %ax,%ax
				}
			}
			spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
			spin_lock_irqsave(&ms_info.mi_connlock, sflags);
			list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
				tmpep = list_entry(pos, struct endpt, list);
     f00:	48 8d b9 c0 fd ff ff 	lea    -0x240(%rcx),%rdi
				if (tmpep == aep) {
     f07:	48 39 fb             	cmp    %rdi,%rbx
     f0a:	0f 84 20 02 00 00    	je     1130 <__scif_close+0x630>
					break;
				}
			}
			spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
			spin_lock_irqsave(&ms_info.mi_connlock, sflags);
			list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
     f10:	48 89 d1             	mov    %rdx,%rcx
     f13:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
     f1a:	48 8b 11             	mov    (%rcx),%rdx
     f1d:	75 e1                	jne    f00 <__scif_close+0x400>
					list_del(pos);
					put_conn_count(aep->remote_dev);
					break;
				}
			}
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
     f1f:	48 8b 35 00 00 00 00 	mov    0x0(%rip),%rsi        # f26 <__scif_close+0x426>
     f26:	48 81 fe 00 00 00 00 	cmp    $0x0,%rsi
     f2d:	48 8b 0e             	mov    (%rsi),%rcx
     f30:	74 35                	je     f67 <__scif_close+0x467>
				tmpep = list_entry(pos, struct endpt, list);
     f32:	48 8d 96 c0 fd ff ff 	lea    -0x240(%rsi),%rdx
				if (tmpep == aep) {
     f39:	48 39 d3             	cmp    %rdx,%rbx
     f3c:	75 1d                	jne    f5b <__scif_close+0x45b>
     f3e:	e9 68 02 00 00       	jmpq   11ab <__scif_close+0x6ab>
     f43:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
					put_conn_count(aep->remote_dev);
					break;
				}
			}
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
				tmpep = list_entry(pos, struct endpt, list);
     f48:	48 8d b9 c0 fd ff ff 	lea    -0x240(%rcx),%rdi
				if (tmpep == aep) {
     f4f:	48 39 fb             	cmp    %rdi,%rbx
     f52:	0f 84 60 02 00 00    	je     11b8 <__scif_close+0x6b8>
					list_del(pos);
					put_conn_count(aep->remote_dev);
					break;
				}
			}
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
     f58:	48 89 d1             	mov    %rdx,%rcx
     f5b:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
     f62:	48 8b 11             	mov    (%rcx),%rdx
     f65:	75 e1                	jne    f48 <__scif_close+0x448>
     f67:	48 89 c6             	mov    %rax,%rsi
     f6a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     f71:	e8 00 00 00 00       	callq  f76 <__scif_close+0x476>
					list_del(pos);
					break;
				}
			}
			spin_unlock_irqrestore(&ms_info.mi_connlock, sflags);
			micscif_teardown_ep(aep);
     f76:	48 89 df             	mov    %rbx,%rdi
     f79:	e8 00 00 00 00       	callq  f7e <__scif_close+0x47e>
			spin_lock_irqsave(&ms_info.mi_eplock, sflags);
     f7e:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
     f85:	e8 00 00 00 00       	callq  f8a <__scif_close+0x48a>
			micscif_add_epd_to_zombie_list(aep, MI_EPLOCK_HELD);
     f8a:	be 01 00 00 00       	mov    $0x1,%esi
     f8f:	48 89 df             	mov    %rbx,%rdi
					break;
				}
			}
			spin_unlock_irqrestore(&ms_info.mi_connlock, sflags);
			micscif_teardown_ep(aep);
			spin_lock_irqsave(&ms_info.mi_eplock, sflags);
     f92:	49 89 c6             	mov    %rax,%r14
			micscif_add_epd_to_zombie_list(aep, MI_EPLOCK_HELD);
     f95:	e8 00 00 00 00       	callq  f9a <__scif_close+0x49a>
			if (tmpep == ep) {
				list_del(pos);
			}
		}
		// Remove any dangling accepts
		while (ep->acceptcnt) {
     f9a:	41 83 ad 60 02 00 00 	subl   $0x1,0x260(%r13)
     fa1:	01 
     fa2:	0f 85 78 fe ff ff    	jne    e20 <__scif_close+0x320>
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

static inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
     fa8:	4c 89 e7             	mov    %r12,%rdi
     fab:	e8 00 00 00 00       	callq  fb0 <__scif_close+0x4b0>
}

#if (NR_CPUS < 256)
static __always_inline void __ticket_spin_unlock(arch_spinlock_t *lock)
{
	asm volatile(UNLOCK_LOCK_PREFIX "incb %0"
     fb0:	fe 05 00 00 00 00    	incb   0x0(%rip)        # fb6 <__scif_close+0x4b6>

		spin_lock(&ep->lock);
		spin_unlock(&ms_info.mi_eplock);

		// Remove and reject any pending connection requests.
		while (ep->conreqcnt) {
     fb6:	41 8b 95 58 01 00 00 	mov    0x158(%r13),%edx
     fbd:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
     fc1:	85 d2                	test   %edx,%edx
     fc3:	0f 84 a2 00 00 00    	je     106b <__scif_close+0x56b>
     fc9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
			conreq = list_first_entry(&ep->conlist, struct conreq, list);
     fd0:	49 8b 9d 78 01 00 00 	mov    0x178(%r13),%rbx
			/*
			 * No Error Handling on purpose for micscif_nodeqp_send().
			 * If the remote node is lost we still want free the connection
			 * requests on the self node.
			 */
			micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, ep);
     fd7:	4c 89 fe             	mov    %r15,%rsi
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
     fda:	48 8b 13             	mov    (%rbx),%rdx
     fdd:	48 8b 43 08          	mov    0x8(%rbx),%rax
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
     fe1:	48 89 42 08          	mov    %rax,0x8(%rdx)
	prev->next = next;
     fe5:	48 89 10             	mov    %rdx,(%rax)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
     fe8:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
     fef:	00 ad de 
     ff2:	48 89 03             	mov    %rax,(%rbx)
	entry->prev = LIST_POISON2;
     ff5:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
     ffc:	00 ad de 
     fff:	48 89 43 08          	mov    %rax,0x8(%rbx)
		// Remove and reject any pending connection requests.
		while (ep->conreqcnt) {
			conreq = list_first_entry(&ep->conlist, struct conreq, list);
			list_del(&conreq->list);

			msg.uop = SCIF_CNCT_REJ;
    1003:	c7 45 ac 09 00 00 00 	movl   $0x9,-0x54(%rbp)
			msg.dst.node = conreq->msg.src.node;
    100a:	0f b7 43 d0          	movzwl -0x30(%rbx),%eax
    100e:	66 89 45 a8          	mov    %ax,-0x58(%rbp)
			msg.dst.port = conreq->msg.src.port;
    1012:	0f b7 43 d2          	movzwl -0x2e(%rbx),%eax
    1016:	66 89 45 aa          	mov    %ax,-0x56(%rbp)
			msg.payload[0] = conreq->msg.payload[0];
    101a:	48 8b 43 dc          	mov    -0x24(%rbx),%rax
    101e:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			msg.payload[1] = conreq->msg.payload[1];
    1022:	48 8b 43 e4          	mov    -0x1c(%rbx),%rax
    1026:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
			/*
			 * No Error Handling on purpose for micscif_nodeqp_send().
			 * If the remote node is lost we still want free the connection
			 * requests on the self node.
			 */
			micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, ep);
    102a:	0f b7 43 d0          	movzwl -0x30(%rbx),%eax
    102e:	48 89 c2             	mov    %rax,%rdx
    1031:	48 c1 e0 09          	shl    $0x9,%rax
    1035:	48 c1 e2 05          	shl    $0x5,%rdx
    1039:	48 29 d0             	sub    %rdx,%rax
    103c:	4c 89 ea             	mov    %r13,%rdx
    103f:	48 8d b8 00 00 00 00 	lea    0x0(%rax),%rdi
    1046:	e8 00 00 00 00       	callq  104b <__scif_close+0x54b>
		spin_lock(&ep->lock);
		spin_unlock(&ms_info.mi_eplock);

		// Remove and reject any pending connection requests.
		while (ep->conreqcnt) {
			conreq = list_first_entry(&ep->conlist, struct conreq, list);
    104b:	48 8d 7b d0          	lea    -0x30(%rbx),%rdi
			 * If the remote node is lost we still want free the connection
			 * requests on the self node.
			 */
			micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, ep);

			ep->conreqcnt--;
    104f:	41 83 ad 58 01 00 00 	subl   $0x1,0x158(%r13)
    1056:	01 
			kfree(conreq);
    1057:	e8 00 00 00 00       	callq  105c <__scif_close+0x55c>

		spin_lock(&ep->lock);
		spin_unlock(&ms_info.mi_eplock);

		// Remove and reject any pending connection requests.
		while (ep->conreqcnt) {
    105c:	41 8b 85 58 01 00 00 	mov    0x158(%r13),%eax
    1063:	85 c0                	test   %eax,%eax
    1065:	0f 85 65 ff ff ff    	jne    fd0 <__scif_close+0x4d0>
			ep->conreqcnt--;
			kfree(conreq);
		}

		// If a kSCIF accept is waiting wake it up
		wake_up_interruptible(&ep->conwq);
    106b:	49 8d bd 88 01 00 00 	lea    0x188(%r13),%rdi
    1072:	31 c9                	xor    %ecx,%ecx
    1074:	ba 01 00 00 00       	mov    $0x1,%edx
    1079:	be 01 00 00 00       	mov    $0x1,%esi
    107e:	e8 00 00 00 00       	callq  1083 <__scif_close+0x583>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1083:	4c 89 f6             	mov    %r14,%rsi
    1086:	4c 89 e7             	mov    %r12,%rdi
    1089:	e8 00 00 00 00       	callq  108e <__scif_close+0x58e>
    108e:	e9 d5 fb ff ff       	jmpq   c68 <__scif_close+0x168>
    1093:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	case SCIFEP_CLOSING:
	{
		struct nodemsg msg;
		struct endpt *fep = NULL;
		struct endpt *tmpep;
		unsigned long ts = jiffies;
    1098:	48 8b 05 00 00 00 00 	mov    0x0(%rip),%rax        # 109f <__scif_close+0x59f>
		struct list_head *pos, *tmpq;

		// Very short time before mapping completes and state becomes connected
		// and does a standard teardown.
		ts = jiffies;
    109f:	48 8b 05 00 00 00 00 	mov    0x0(%rip),%rax        # 10a6 <__scif_close+0x5a6>
		while (ep->state == SCIFEP_MAPPING) {
    10a6:	eb 27                	jmp    10cf <__scif_close+0x5cf>
    10a8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    10af:	00 
	asm volatile("delay %0" :: "r" (1) : "memory");
#else
	asm volatile("delay %0" :: "r" (1000) : "memory");
#endif
#else
	asm volatile("rep; nop" ::: "memory");
    10b0:	f3 90                	pause  
			cpu_relax();
			if (time_after((unsigned long)jiffies,ts + NODE_ALIVE_TIMEOUT)) {
    10b2:	6b 15 00 00 00 00 64 	imul   $0x64,0x0(%rip),%edx        # 10b9 <__scif_close+0x5b9>
    10b9:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # 10c0 <__scif_close+0x5c0>
    10c0:	48 63 d2             	movslq %edx,%rdx
    10c3:	48 01 c2             	add    %rax,%rdx
    10c6:	48 39 ca             	cmp    %rcx,%rdx
    10c9:	0f 88 8c fa ff ff    	js     b5b <__scif_close+0x5b>
		struct list_head *pos, *tmpq;

		// Very short time before mapping completes and state becomes connected
		// and does a standard teardown.
		ts = jiffies;
		while (ep->state == SCIFEP_MAPPING) {
    10cf:	41 8b 55 00          	mov    0x0(%r13),%edx
    10d3:	83 fa 06             	cmp    $0x6,%edx
    10d6:	74 d8                	je     10b0 <__scif_close+0x5b0>
    10d8:	e9 ab fa ff ff       	jmpq   b88 <__scif_close+0x88>
    10dd:	0f 1f 00             	nopl   (%rax)
    10e0:	48 89 c6             	mov    %rax,%rsi
    10e3:	4c 89 e7             	mov    %r12,%rdi
    10e6:	e8 00 00 00 00       	callq  10eb <__scif_close+0x5eb>
		wake_up_interruptible(&ep->conwq);
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	}
	if (ep->port.port && !ep->accepted_ep)
    10eb:	41 0f b7 45 08       	movzwl 0x8(%r13),%eax
    10f0:	66 85 c0             	test   %ax,%ax
    10f3:	0f 84 7f fb ff ff    	je     c78 <__scif_close+0x178>
    10f9:	e9 d2 fb ff ff       	jmpq   cd0 <__scif_close+0x1d0>
    10fe:	66 90                	xchg   %ax,%ax
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock and
			// proceed to teardown the end point below.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
				tmpep = list_entry(pos, struct endpt, list);
    1100:	48 8d 88 c0 fd ff ff 	lea    -0x240(%rax),%rcx
				if (tmpep == ep) {
    1107:	49 39 cd             	cmp    %rcx,%r13
    110a:	0f 84 20 fb ff ff    	je     c30 <__scif_close+0x130>
			// The other side has completed the disconnect before
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock and
			// proceed to teardown the end point below.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
    1110:	48 89 d0             	mov    %rdx,%rax
    1113:	48 3d 00 00 00 00    	cmp    $0x0,%rax
    1119:	48 8b 10             	mov    (%rax),%rdx
    111c:	75 e2                	jne    1100 <__scif_close+0x600>
    111e:	e9 33 fb ff ff       	jmpq   c56 <__scif_close+0x156>
			}
			spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
			spin_lock_irqsave(&ms_info.mi_connlock, sflags);
			list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
				tmpep = list_entry(pos, struct endpt, list);
				if (tmpep == aep) {
    1123:	48 89 ca             	mov    %rcx,%rdx
    1126:	48 89 f9             	mov    %rdi,%rcx
    1129:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    1130:	48 8b 71 08          	mov    0x8(%rcx),%rsi
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    1134:	48 89 72 08          	mov    %rsi,0x8(%rdx)
	prev->next = next;
    1138:	48 89 16             	mov    %rdx,(%rsi)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    113b:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
    1142:	00 ad de 
    1145:	48 89 31             	mov    %rsi,(%rcx)
	entry->prev = LIST_POISON2;
    1148:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
    114f:	00 ad de 
    1152:	48 89 71 08          	mov    %rsi,0x8(%rcx)
					list_del(pos);
					put_conn_count(aep->remote_dev);
    1156:	49 8b 8f e0 fe ff ff 	mov    -0x120(%r15),%rcx
 * to synchronize calling this API with get_conn_count.
 */
static __always_inline void
put_conn_count(struct micscif_dev *dev)
{
	dev->num_active_conn--;
    115d:	8b b1 dc 01 00 00    	mov    0x1dc(%rcx),%esi
    1163:	8d 56 ff             	lea    -0x1(%rsi),%edx
	BUG_ON(dev->num_active_conn < 0);
    1166:	85 d2                	test   %edx,%edx
 * to synchronize calling this API with get_conn_count.
 */
static __always_inline void
put_conn_count(struct micscif_dev *dev)
{
	dev->num_active_conn--;
    1168:	89 91 dc 01 00 00    	mov    %edx,0x1dc(%rcx)
	BUG_ON(dev->num_active_conn < 0);
    116e:	0f 89 ab fd ff ff    	jns    f1f <__scif_close+0x41f>
    1174:	0f 0b                	ud2    
			list_del(&aep->liacceptlist);
			if (aep->port.port && !aep->accepted_ep)
				put_scif_port(aep->port.port);
			list_for_each_safe(pos, tmpq, &ms_info.mi_uaccept) {
				tmpep = list_entry(pos, struct endpt, miacceptlist);
				if (tmpep == aep) {
    1176:	48 89 c8             	mov    %rcx,%rax
    1179:	48 89 d1             	mov    %rdx,%rcx
    117c:	0f 1f 40 00          	nopl   0x0(%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    1180:	48 8b 51 08          	mov    0x8(%rcx),%rdx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    1184:	48 89 50 08          	mov    %rdx,0x8(%rax)
	prev->next = next;
    1188:	48 89 02             	mov    %rax,(%rdx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    118b:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
    1192:	00 ad de 
    1195:	48 89 01             	mov    %rax,(%rcx)
	entry->prev = LIST_POISON2;
    1198:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
    119f:	00 ad de 
    11a2:	48 89 41 08          	mov    %rax,0x8(%rcx)
    11a6:	e9 14 fd ff ff       	jmpq   ebf <__scif_close+0x3bf>
					break;
				}
			}
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
				tmpep = list_entry(pos, struct endpt, list);
				if (tmpep == aep) {
    11ab:	48 89 ca             	mov    %rcx,%rdx
    11ae:	48 89 f1             	mov    %rsi,%rcx
    11b1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    11b8:	48 8b 71 08          	mov    0x8(%rcx),%rsi
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    11bc:	48 89 72 08          	mov    %rsi,0x8(%rdx)
	prev->next = next;
    11c0:	48 89 16             	mov    %rdx,(%rsi)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    11c3:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
    11ca:	00 ad de 
    11cd:	48 89 31             	mov    %rsi,(%rcx)
	entry->prev = LIST_POISON2;
    11d0:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
    11d7:	00 ad de 
    11da:	48 89 71 08          	mov    %rsi,0x8(%rcx)
    11de:	e9 84 fd ff ff       	jmpq   f67 <__scif_close+0x467>
		micscif_unregister_all_windows(epd);
		// Remove from the disconnected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
			tmpep = list_entry(pos, struct endpt, list);
			if (tmpep == ep) {
    11e3:	48 89 d1             	mov    %rdx,%rcx
    11e6:	48 89 fa             	mov    %rdi,%rdx
    11e9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    11f0:	48 8b 72 08          	mov    0x8(%rdx),%rsi
    11f4:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    11fb:	48 89 71 08          	mov    %rsi,0x8(%rcx)
	prev->next = next;
    11ff:	48 89 0e             	mov    %rcx,(%rsi)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    1202:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
    1209:	00 ad de 
    120c:	48 89 32             	mov    %rsi,(%rdx)
	entry->prev = LIST_POISON2;
    120f:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
    1216:	00 ad de 
    1219:	48 89 72 08          	mov    %rsi,0x8(%rdx)
    121d:	48 89 c6             	mov    %rax,%rsi
    1220:	e8 00 00 00 00       	callq  1225 <__scif_close+0x725>
    1225:	e9 34 fb ff ff       	jmpq   d5e <__scif_close+0x25e>
    122a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		while (ep->acceptcnt) {
			aep = list_first_entry(&ep->li_accept, struct endpt, liacceptlist);
			BUG_ON(!aep);
			list_del(&aep->liacceptlist);
			if (aep->port.port && !aep->accepted_ep)
				put_scif_port(aep->port.port);
    1230:	0f b7 f8             	movzwl %ax,%edi
    1233:	e8 00 00 00 00       	callq  1238 <__scif_close+0x738>
    1238:	e9 3e fc ff ff       	jmpq   e7b <__scif_close+0x37b>
    123d:	0f 1f 00             	nopl   (%rax)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    1240:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    1247:	48 8d bb 88 01 00 00 	lea    0x188(%rbx),%rdi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    124e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    1255:	00 00 80 
    1258:	48 39 f1             	cmp    %rsi,%rcx
    125b:	74 3e                	je     129b <__scif_close+0x79b>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    125d:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1261:	48 89 c8             	mov    %rcx,%rax
    1264:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    126b:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    126d:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1270:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1273:	0f 84 b7 f8 ff ff    	je     b30 <__scif_close+0x30>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1279:	48 39 f2             	cmp    %rsi,%rdx
    127c:	74 1d                	je     129b <__scif_close+0x79b>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    127e:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1282:	48 89 d0             	mov    %rdx,%rax
    1285:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    128a:	48 39 d0             	cmp    %rdx,%rax
    128d:	0f 84 9d f8 ff ff    	je     b30 <__scif_close+0x30>
    1293:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1296:	48 39 f2             	cmp    %rsi,%rdx
    1299:	75 e3                	jne    127e <__scif_close+0x77e>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    129b:	4c 8d a3 68 01 00 00 	lea    0x168(%rbx),%r12
    12a2:	4c 89 e7             	mov    %r12,%rdi
    12a5:	e8 00 00 00 00       	callq  12aa <__scif_close+0x7aa>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    12aa:	8b 43 04             	mov    0x4(%rbx),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    12ad:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    12b0:	83 fa 01             	cmp    $0x1,%edx
    12b3:	76 1e                	jbe    12d3 <__scif_close+0x7d3>
    12b5:	83 f8 01             	cmp    $0x1,%eax
    12b8:	74 19                	je     12d3 <__scif_close+0x7d3>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    12ba:	48 8b 83 88 01 00 00 	mov    0x188(%rbx),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    12c1:	48 85 c0             	test   %rax,%rax
    12c4:	0f 88 00 02 00 00    	js     14ca <__scif_close+0x9ca>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
    12ca:	f0 48 83 83 88 01 00 	lock addq $0x1,0x188(%rbx)
    12d1:	00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    12d3:	4c 89 e7             	mov    %r12,%rdi
    12d6:	e8 00 00 00 00       	callq  12db <__scif_close+0x7db>
    12db:	e9 50 f8 ff ff       	jmpq   b30 <__scif_close+0x30>
			}
		}
		// Remove any dangling accepts
		while (ep->acceptcnt) {
			aep = list_first_entry(&ep->li_accept, struct endpt, liacceptlist);
			BUG_ON(!aep);
    12e0:	0f 0b                	ud2    

		// Remove from the connected list
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
			tmpep = list_entry(pos, struct endpt, list);
			if (tmpep == ep) {
    12e2:	48 89 d0             	mov    %rdx,%rax
    12e5:	48 89 ca             	mov    %rcx,%rdx
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    12e8:	48 8b 4a 08          	mov    0x8(%rdx),%rcx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    12ec:	48 89 48 08          	mov    %rcx,0x8(%rax)
	prev->next = next;
    12f0:	48 89 01             	mov    %rax,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    12f3:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
    12fa:	00 ad de 
    12fd:	48 89 02             	mov    %rax,(%rdx)
	entry->prev = LIST_POISON2;
    1300:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
    1307:	00 ad de 
    130a:	48 89 42 08          	mov    %rax,0x8(%rdx)
				list_del(pos);
				put_conn_count(ep->remote_dev);
    130e:	49 8b 95 48 01 00 00 	mov    0x148(%r13),%rdx
    1315:	83 aa dc 01 00 00 01 	subl   $0x1,0x1dc(%rdx)
    131c:	0f 88 52 fe ff ff    	js     1174 <__scif_close+0x674>
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

static inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
    1322:	4c 89 e7             	mov    %r12,%rdi
    1325:	e8 00 00 00 00       	callq  132a <__scif_close+0x82a>
				spin_lock(&ep->lock);
				break;
			}
		}

		if (fep == NULL) {
    132a:	4d 85 ff             	test   %r15,%r15
    132d:	0f 84 d4 f8 ff ff    	je     c07 <__scif_close+0x107>
    1333:	fe 05 00 00 00 00    	incb   0x0(%rip)        # 1339 <__scif_close+0x839>

		spin_unlock(&ms_info.mi_connlock);

		// Now we are free to close out the connection
		msg.uop = SCIF_DISCNCT;
		msg.src = ep->port;
    1339:	41 8b 45 06          	mov    0x6(%r13),%eax
		msg.dst = ep->peer;
		msg.payload[0] = (uint64_t)ep;
		msg.payload[1] = ep->remote_ep;

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    133d:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    1341:	4c 89 ea             	mov    %r13,%rdx

		// Now we are free to close out the connection
		msg.uop = SCIF_DISCNCT;
		msg.src = ep->port;
		msg.dst = ep->peer;
		msg.payload[0] = (uint64_t)ep;
    1344:	4c 89 6d b0          	mov    %r13,-0x50(%rbp)
		msg.payload[1] = ep->remote_ep;

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    1348:	49 8b bd 48 01 00 00 	mov    0x148(%r13),%rdi
		}

		spin_unlock(&ms_info.mi_connlock);

		// Now we are free to close out the connection
		msg.uop = SCIF_DISCNCT;
    134f:	c7 45 ac 0c 00 00 00 	movl   $0xc,-0x54(%rbp)
		msg.src = ep->port;
    1356:	89 45 a4             	mov    %eax,-0x5c(%rbp)
		msg.dst = ep->peer;
    1359:	41 8b 45 0a          	mov    0xa(%r13),%eax
    135d:	89 45 a8             	mov    %eax,-0x58(%rbp)
		msg.payload[0] = (uint64_t)ep;
		msg.payload[1] = ep->remote_ep;
    1360:	49 8b 85 50 01 00 00 	mov    0x150(%r13),%rax
    1367:	48 89 45 b8          	mov    %rax,-0x48(%rbp)

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    136b:	e8 00 00 00 00       	callq  1370 <__scif_close+0x870>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1370:	48 89 de             	mov    %rbx,%rsi
    1373:	4c 89 e7             	mov    %r12,%rdi
    1376:	41 89 c7             	mov    %eax,%r15d
    1379:	e8 00 00 00 00       	callq  137e <__scif_close+0x87e>
		spin_unlock_irqrestore(&ep->lock, sflags);

		if (!err)
    137e:	45 85 ff             	test   %r15d,%r15d
    1381:	0f 85 85 00 00 00    	jne    140c <__scif_close+0x90c>
			/* Now wait for the remote node to respond */
			wait_event_timeout(ep->disconwq, 
    1387:	41 8b 55 00          	mov    0x0(%r13),%edx
    138b:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 1391 <__scif_close+0x891>
    1391:	83 fa 09             	cmp    $0x9,%edx
    1394:	74 76                	je     140c <__scif_close+0x90c>
    1396:	6b c0 64             	imul   $0x64,%eax,%eax
    1399:	4c 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%r15
    13a0:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    13a7:	00 00 00 00 
    13ab:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    13b2:	00 
    13b3:	48 63 d8             	movslq %eax,%rbx
    13b6:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    13bd:	00 00 
    13bf:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    13c3:	49 8d 47 18          	lea    0x18(%r15),%rax
    13c7:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    13cb:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    13cf:	eb 17                	jmp    13e8 <__scif_close+0x8e8>
    13d1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    13d8:	48 89 df             	mov    %rbx,%rdi
    13db:	e8 00 00 00 00       	callq  13e0 <__scif_close+0x8e0>
    13e0:	48 85 c0             	test   %rax,%rax
    13e3:	48 89 c3             	mov    %rax,%rbx
    13e6:	74 19                	je     1401 <__scif_close+0x901>
    13e8:	ba 02 00 00 00       	mov    $0x2,%edx
    13ed:	4c 89 fe             	mov    %r15,%rsi
    13f0:	4c 89 f7             	mov    %r14,%rdi
    13f3:	e8 00 00 00 00       	callq  13f8 <__scif_close+0x8f8>
    13f8:	41 8b 45 00          	mov    0x0(%r13),%eax
    13fc:	83 f8 09             	cmp    $0x9,%eax
    13ff:	75 d7                	jne    13d8 <__scif_close+0x8d8>
    1401:	4c 89 fe             	mov    %r15,%rsi
    1404:	4c 89 f7             	mov    %r14,%rdi
    1407:	e8 00 00 00 00       	callq  140c <__scif_close+0x90c>
		 * Grab and release the ep lock to synchronize with the
		 * thread waking us up. If we dont grab this lock, then
		 * the ep might be freed before the wakeup completes
		 * resulting in potential memory corruption.
		 */
		spin_lock_irqsave(&ep->lock, sflags);
    140c:	4c 89 e7             	mov    %r12,%rdi
    140f:	e8 00 00 00 00       	callq  1414 <__scif_close+0x914>
    1414:	4c 89 e7             	mov    %r12,%rdi
    1417:	48 89 c6             	mov    %rax,%rsi
    141a:	e8 00 00 00 00       	callq  141f <__scif_close+0x91f>
		wake_up_interruptible(&ep->conwq);
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	}
	if (ep->port.port && !ep->accepted_ep)
    141f:	41 0f b7 45 08       	movzwl 0x8(%r13),%eax
    1424:	66 85 c0             	test   %ax,%ax
    1427:	0f 84 4b f8 ff ff    	je     c78 <__scif_close+0x178>
    142d:	e9 9e f8 ff ff       	jmpq   cd0 <__scif_close+0x1d0>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    1432:	48 8b 45 08          	mov    0x8(%rbp),%rax
    1436:	48 89 d9             	mov    %rbx,%rcx
    1439:	ba a7 00 00 00       	mov    $0xa7,%edx
    143e:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    1445:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    144c:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    1453:	44 0f b7 03          	movzwl (%rbx),%r8d
    1457:	48 89 04 24          	mov    %rax,(%rsp)
    145b:	31 c0                	xor    %eax,%eax
    145d:	e8 00 00 00 00       	callq  1462 <__scif_close+0x962>
    1462:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1469:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    1470:	00 00 80 
    1473:	48 39 f1             	cmp    %rsi,%rcx
    1476:	0f 84 29 f8 ff ff    	je     ca5 <__scif_close+0x1a5>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    147c:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1480:	48 89 c8             	mov    %rcx,%rax
    1483:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    148a:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    148c:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    148f:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1492:	0f 84 0d f8 ff ff    	je     ca5 <__scif_close+0x1a5>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1498:	48 39 f2             	cmp    %rsi,%rdx
    149b:	0f 84 04 f8 ff ff    	je     ca5 <__scif_close+0x1a5>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    14a1:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    14a5:	48 89 d0             	mov    %rdx,%rax
    14a8:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    14ae:	48 39 d0             	cmp    %rdx,%rax
    14b1:	0f 84 ee f7 ff ff    	je     ca5 <__scif_close+0x1a5>
    14b7:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    14ba:	48 39 f2             	cmp    %rsi,%rdx
    14bd:	75 e2                	jne    14a1 <__scif_close+0x9a1>
    14bf:	e9 e1 f7 ff ff       	jmpq   ca5 <__scif_close+0x1a5>
    14c4:	0f 1f 40 00          	nopl   0x0(%rax)

	ep->state = SCIFEP_CLOSING;

	switch (oldstate) {
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == oldstate);
    14c8:	0f 0b                	ud2    
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    14ca:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 14d0 <__scif_close+0x9d0>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    14d0:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    14d4:	31 f6                	xor    %esi,%esi
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    14d6:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    14d8:	48 c7 83 b0 01 00 00 	movq   $0x2,0x1b0(%rbx)
    14df:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    14e3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    14ea:	66 89 75 a8          	mov    %si,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    14ee:	4c 89 fe             	mov    %r15,%rsi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    14f1:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
    14f8:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    14fc:	0f b7 03             	movzwl (%rbx),%eax
    14ff:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    1503:	e8 00 00 00 00       	callq  1508 <__scif_close+0xa08>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    1508:	48 8b 83 b0 01 00 00 	mov    0x1b0(%rbx),%rax
    150f:	48 83 f8 02          	cmp    $0x2,%rax
    1513:	74 1e                	je     1533 <__scif_close+0xa33>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    1515:	48 83 f8 03          	cmp    $0x3,%rax
    1519:	0f 85 ab fd ff ff    	jne    12ca <__scif_close+0x7ca>
				dev->sd_state = SCIFDEV_RUNNING;
    151f:	c7 43 04 02 00 00 00 	movl   $0x2,0x4(%rbx)
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    1526:	f0 80 a3 8f 01 00 00 	lock andb $0x7f,0x18f(%rbx)
    152d:	7f 
    152e:	e9 97 fd ff ff       	jmpq   12ca <__scif_close+0x7ca>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    1533:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    153a:	30 c0                	xor    %al,%al
    153c:	b9 0a 00 00 00       	mov    $0xa,%ecx
    1541:	f3 ab                	rep stos %eax,%es:(%rdi)
    1543:	4c 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%r15
    154a:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    1551:	00 
    1552:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    1559:	00 00 
    155b:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    155f:	49 8d 47 18          	lea    0x18(%r15),%rax
    1563:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    1567:	4c 8d b3 98 01 00 00 	lea    0x198(%rbx),%r14
    156e:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    1572:	ba 02 00 00 00       	mov    $0x2,%edx
    1577:	4c 89 fe             	mov    %r15,%rsi
    157a:	4c 89 f7             	mov    %r14,%rdi
    157d:	e8 00 00 00 00       	callq  1582 <__scif_close+0xa82>
    1582:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    1589:	02 
    158a:	75 07                	jne    1593 <__scif_close+0xa93>
    158c:	e8 00 00 00 00       	callq  1591 <__scif_close+0xa91>
    1591:	eb df                	jmp    1572 <__scif_close+0xa72>
    1593:	4c 89 fe             	mov    %r15,%rsi
    1596:	4c 89 f7             	mov    %r14,%rdi
    1599:	e8 00 00 00 00       	callq  159e <__scif_close+0xa9e>
    159e:	48 8b 83 b0 01 00 00 	mov    0x1b0(%rbx),%rax
    15a5:	e9 6b ff ff ff       	jmpq   1515 <__scif_close+0xa15>
    15aa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000000015b0 <scif_ref_rel>:
	return 0;
}

void
scif_ref_rel(struct kref *kref_count)
{
    15b0:	55                   	push   %rbp
    15b1:	48 89 e5             	mov    %rsp,%rbp
    15b4:	e8 00 00 00 00       	callq  15b9 <scif_ref_rel+0x9>
	struct endpt *epd;
	epd = container_of(kref_count, struct endpt, ref_count);
    15b9:	48 81 ef 70 01 00 00 	sub    $0x170,%rdi
	__scif_close((scif_epd_t)epd);
    15c0:	e8 00 00 00 00       	callq  15c5 <scif_ref_rel+0x15>
}
    15c5:	5d                   	pop    %rbp
    15c6:	c3                   	retq   
    15c7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    15ce:	00 00 

00000000000015d0 <__scif_flush>:
 * @epd:        The end point address returned from scif_open()
 *
 */
int
__scif_flush(scif_epd_t epd)
{
    15d0:	55                   	push   %rbp
    15d1:	48 89 e5             	mov    %rsp,%rbp
    15d4:	41 57                	push   %r15
    15d6:	41 56                	push   %r14
    15d8:	41 55                	push   %r13
    15da:	41 54                	push   %r12
    15dc:	53                   	push   %rbx
    15dd:	48 83 ec 78          	sub    $0x78,%rsp
    15e1:	e8 00 00 00 00       	callq  15e6 <__scif_flush+0x16>
    15e6:	48 89 fb             	mov    %rdi,%rbx
	struct endpt *tmpep;
	struct list_head *pos, *tmpq;
	unsigned long sflags;
	int err;

	might_sleep();
    15e9:	e8 00 00 00 00       	callq  15ee <__scif_flush+0x1e>
	micscif_inc_node_refcnt(ep->remote_dev, 1);
    15ee:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    15f5:	4d 85 e4             	test   %r12,%r12
    15f8:	0f 85 32 03 00 00    	jne    1930 <__scif_flush+0x360>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    15fe:	4c 8d 63 04          	lea    0x4(%rbx),%r12

	spin_lock_irqsave(&ep->lock, sflags);
    1602:	4c 89 e7             	mov    %r12,%rdi
    1605:	e8 00 00 00 00       	callq  160a <__scif_flush+0x3a>

	switch (ep->state) {
    160a:	8b 13                	mov    (%rbx),%edx
	int err;

	might_sleep();
	micscif_inc_node_refcnt(ep->remote_dev, 1);

	spin_lock_irqsave(&ep->lock, sflags);
    160c:	49 89 c6             	mov    %rax,%r14

	switch (ep->state) {
    160f:	83 fa 03             	cmp    $0x3,%edx
    1612:	0f 84 38 01 00 00    	je     1750 <__scif_flush+0x180>
    1618:	83 fa 04             	cmp    $0x4,%edx
    161b:	0f 85 df 00 00 00    	jne    1700 <__scif_flush+0x130>
	case SCIFEP_CONNECTED:
	{
		struct nodemsg msg;
		struct endpt *fep = NULL;

		init_waitqueue_head(&ep->disconwq);	// Wait for connection queue
    1621:	4c 8d ab a0 01 00 00 	lea    0x1a0(%rbx),%r13
    1628:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    162f:	4c 89 ef             	mov    %r13,%rdi
    1632:	e8 00 00 00 00       	callq  1637 <__scif_flush+0x67>
		WARN_ON(ep->files); // files should never be set while connected
    1637:	48 83 bb 68 01 00 00 	cmpq   $0x0,0x168(%rbx)
    163e:	00 
    163f:	0f 85 66 04 00 00    	jne    1aab <__scif_flush+0x4db>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1645:	4c 89 f6             	mov    %r14,%rsi
    1648:	4c 89 e7             	mov    %r12,%rdi
    164b:	e8 00 00 00 00       	callq  1650 <__scif_flush+0x80>
		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
    1650:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    1657:	e8 00 00 00 00       	callq  165c <__scif_flush+0x8c>

		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
    165c:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # 1663 <__scif_flush+0x93>
		struct endpt *fep = NULL;

		init_waitqueue_head(&ep->disconwq);	// Wait for connection queue
		WARN_ON(ep->files); // files should never be set while connected
		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
    1663:	49 89 c6             	mov    %rax,%r14

		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
    1666:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
    166d:	48 8b 11             	mov    (%rcx),%rdx
    1670:	74 35                	je     16a7 <__scif_flush+0xd7>
			tmpep = list_entry(pos, struct endpt, list);
    1672:	4c 8d b9 c0 fd ff ff 	lea    -0x240(%rcx),%r15
			if (tmpep == ep) {
    1679:	4c 39 fb             	cmp    %r15,%rbx
    167c:	75 1d                	jne    169b <__scif_flush+0xcb>
    167e:	e9 f8 00 00 00       	jmpq   177b <__scif_flush+0x1ab>
    1683:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		WARN_ON(ep->files); // files should never be set while connected
		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);

		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
			tmpep = list_entry(pos, struct endpt, list);
    1688:	4c 8d ba c0 fd ff ff 	lea    -0x240(%rdx),%r15
			if (tmpep == ep) {
    168f:	4c 39 fb             	cmp    %r15,%rbx
    1692:	0f 84 f0 00 00 00    	je     1788 <__scif_flush+0x1b8>
		init_waitqueue_head(&ep->disconwq);	// Wait for connection queue
		WARN_ON(ep->files); // files should never be set while connected
		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);

		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
    1698:	48 89 c2             	mov    %rax,%rdx
    169b:	48 81 fa 00 00 00 00 	cmp    $0x0,%rdx
    16a2:	48 8b 02             	mov    (%rdx),%rax
    16a5:	75 e1                	jne    1688 <__scif_flush+0xb8>
		if (fep == NULL) {
			// The other side has completed the disconnect before
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
    16a7:	48 8b 0d 00 00 00 00 	mov    0x0(%rip),%rcx        # 16ae <__scif_flush+0xde>
    16ae:	48 81 f9 00 00 00 00 	cmp    $0x0,%rcx
    16b5:	48 8b 01             	mov    (%rcx),%rax
    16b8:	74 34                	je     16ee <__scif_flush+0x11e>
				tmpep = list_entry(pos, struct endpt, list);
    16ba:	48 8d 91 c0 fd ff ff 	lea    -0x240(%rcx),%rdx
				if (tmpep == ep) {
    16c1:	48 39 d3             	cmp    %rdx,%rbx
    16c4:	75 1d                	jne    16e3 <__scif_flush+0x113>
    16c6:	e9 ad 03 00 00       	jmpq   1a78 <__scif_flush+0x4a8>
    16cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
			// The other side has completed the disconnect before
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
				tmpep = list_entry(pos, struct endpt, list);
    16d0:	48 8d 88 c0 fd ff ff 	lea    -0x240(%rax),%rcx
				if (tmpep == ep) {
    16d7:	48 39 cb             	cmp    %rcx,%rbx
    16da:	0f 84 a0 03 00 00    	je     1a80 <__scif_flush+0x4b0>
		if (fep == NULL) {
			// The other side has completed the disconnect before
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
    16e0:	48 89 d0             	mov    %rdx,%rax
    16e3:	48 3d 00 00 00 00    	cmp    $0x0,%rax
    16e9:	48 8b 10             	mov    (%rax),%rdx
    16ec:	75 e2                	jne    16d0 <__scif_flush+0x100>
    16ee:	4c 89 f6             	mov    %r14,%rsi
    16f1:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    16f8:	e8 00 00 00 00       	callq  16fd <__scif_flush+0x12d>
    16fd:	eb 0c                	jmp    170b <__scif_flush+0x13b>
    16ff:	90                   	nop
    1700:	48 89 c6             	mov    %rax,%rsi
    1703:	4c 89 e7             	mov    %r12,%rdi
    1706:	e8 00 00 00 00       	callq  170b <__scif_flush+0x13b>
	}
	default:
		spin_unlock_irqrestore(&ep->lock, sflags);
		break;
	}
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    170b:	48 8b 9b 48 01 00 00 	mov    0x148(%rbx),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    1712:	48 85 db             	test   %rbx,%rbx
    1715:	74 21                	je     1738 <__scif_flush+0x168>
		if (unlikely((atomic_long_sub_return(cnt, 
    1717:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    171e:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    1725:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    172c:	00 00 
    172e:	48 83 e8 01          	sub    $0x1,%rax
    1732:	0f 88 a8 02 00 00    	js     19e0 <__scif_flush+0x410>
	return 0;
}
    1738:	48 83 c4 78          	add    $0x78,%rsp
    173c:	31 c0                	xor    %eax,%eax
    173e:	5b                   	pop    %rbx
    173f:	41 5c                	pop    %r12
    1741:	41 5d                	pop    %r13
    1743:	41 5e                	pop    %r14
    1745:	41 5f                	pop    %r15
    1747:	5d                   	pop    %rbp
    1748:	c3                   	retq   
    1749:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	case SCIFEP_LISTENING:
	{
		ep->state = SCIFEP_CLLISTEN;

		// If an accept is waiting wake it up
		wake_up_interruptible(&ep->conwq);
    1750:	48 8d bb 88 01 00 00 	lea    0x188(%rbx),%rdi
		wake_up_interruptible(&ep->recvwq);
		break;
	}
	case SCIFEP_LISTENING:
	{
		ep->state = SCIFEP_CLLISTEN;
    1757:	c7 03 08 00 00 00    	movl   $0x8,(%rbx)

		// If an accept is waiting wake it up
		wake_up_interruptible(&ep->conwq);
    175d:	31 c9                	xor    %ecx,%ecx
    175f:	ba 01 00 00 00       	mov    $0x1,%edx
    1764:	be 01 00 00 00       	mov    $0x1,%esi
    1769:	e8 00 00 00 00       	callq  176e <__scif_flush+0x19e>
    176e:	4c 89 f6             	mov    %r14,%rsi
    1771:	4c 89 e7             	mov    %r12,%rdi
    1774:	e8 00 00 00 00       	callq  1779 <__scif_flush+0x1a9>
    1779:	eb 90                	jmp    170b <__scif_flush+0x13b>
		spin_unlock_irqrestore(&ep->lock, sflags);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);

		list_for_each_safe(pos, tmpq, &ms_info.mi_connected) {
			tmpep = list_entry(pos, struct endpt, list);
			if (tmpep == ep) {
    177b:	48 89 d0             	mov    %rdx,%rax
    177e:	48 89 ca             	mov    %rcx,%rdx
    1781:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    1788:	48 8b 4a 08          	mov    0x8(%rdx),%rcx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    178c:	48 89 48 08          	mov    %rcx,0x8(%rax)
	prev->next = next;
    1790:	48 89 01             	mov    %rax,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    1793:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
    179a:	00 ad de 
    179d:	48 89 02             	mov    %rax,(%rdx)
	entry->prev = LIST_POISON2;
    17a0:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
    17a7:	00 ad de 
    17aa:	48 89 42 08          	mov    %rax,0x8(%rdx)
				list_del(pos);
				put_conn_count(ep->remote_dev);
    17ae:	48 8b 93 48 01 00 00 	mov    0x148(%rbx),%rdx
    17b5:	83 aa dc 01 00 00 01 	subl   $0x1,0x1dc(%rdx)
    17bc:	0f 88 ff 02 00 00    	js     1ac1 <__scif_flush+0x4f1>
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

static inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
    17c2:	4c 89 e7             	mov    %r12,%rdi
    17c5:	e8 00 00 00 00       	callq  17ca <__scif_flush+0x1fa>
				spin_lock(&ep->lock);
				break;
			}
		}

		if (fep == NULL) {
    17ca:	4d 85 ff             	test   %r15,%r15
    17cd:	0f 84 d4 fe ff ff    	je     16a7 <__scif_flush+0xd7>
    17d3:	fe 05 00 00 00 00    	incb   0x0(%rip)        # 17d9 <__scif_flush+0x209>
		}

		spin_unlock(&ms_info.mi_connlock);

		msg.uop = SCIF_DISCNCT;
		msg.src = ep->port;
    17d9:	8b 43 06             	mov    0x6(%rbx),%eax
		msg.dst = ep->peer;
		msg.payload[0] = (uint64_t)ep;
		msg.payload[1] = ep->remote_ep;

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    17dc:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    17e0:	48 89 da             	mov    %rbx,%rdx
		spin_unlock(&ms_info.mi_connlock);

		msg.uop = SCIF_DISCNCT;
		msg.src = ep->port;
		msg.dst = ep->peer;
		msg.payload[0] = (uint64_t)ep;
    17e3:	48 89 5d b0          	mov    %rbx,-0x50(%rbp)
		msg.payload[1] = ep->remote_ep;

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    17e7:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
			break;
		}

		spin_unlock(&ms_info.mi_connlock);

		msg.uop = SCIF_DISCNCT;
    17ee:	c7 45 ac 0c 00 00 00 	movl   $0xc,-0x54(%rbp)
		msg.src = ep->port;
    17f5:	89 45 a4             	mov    %eax,-0x5c(%rbp)
		msg.dst = ep->peer;
    17f8:	8b 43 0a             	mov    0xa(%rbx),%eax
    17fb:	89 45 a8             	mov    %eax,-0x58(%rbp)
		msg.payload[0] = (uint64_t)ep;
		msg.payload[1] = ep->remote_ep;
    17fe:	48 8b 83 50 01 00 00 	mov    0x150(%rbx),%rax
    1805:	48 89 45 b8          	mov    %rax,-0x48(%rbp)

		err = micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    1809:	e8 00 00 00 00       	callq  180e <__scif_flush+0x23e>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    180e:	4c 89 f6             	mov    %r14,%rsi
    1811:	4c 89 e7             	mov    %r12,%rdi
    1814:	41 89 c7             	mov    %eax,%r15d
    1817:	e8 00 00 00 00       	callq  181c <__scif_flush+0x24c>

		spin_unlock_irqrestore(&ep->lock, sflags);
		if (!err)
    181c:	45 85 ff             	test   %r15d,%r15d
    181f:	0f 85 7d 00 00 00    	jne    18a2 <__scif_flush+0x2d2>
			/* Now wait for the remote node to respond */
			wait_event_timeout(ep->disconwq, 
    1825:	8b 03                	mov    (%rbx),%eax
    1827:	44 8b 35 00 00 00 00 	mov    0x0(%rip),%r14d        # 182e <__scif_flush+0x25e>
    182e:	83 f8 09             	cmp    $0x9,%eax
    1831:	74 6f                	je     18a2 <__scif_flush+0x2d2>
    1833:	45 6b f6 64          	imul   $0x64,%r14d,%r14d
    1837:	4c 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%r15
    183e:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    1845:	00 00 00 00 
    1849:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    1850:	00 00 
    1852:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    1856:	49 8d 47 18          	lea    0x18(%r15),%rax
    185a:	4d 63 f6             	movslq %r14d,%r14
    185d:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    1864:	00 
    1865:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    1869:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    186d:	eb 11                	jmp    1880 <__scif_flush+0x2b0>
    186f:	90                   	nop
    1870:	4c 89 f7             	mov    %r14,%rdi
    1873:	e8 00 00 00 00       	callq  1878 <__scif_flush+0x2a8>
    1878:	48 85 c0             	test   %rax,%rax
    187b:	49 89 c6             	mov    %rax,%r14
    187e:	74 17                	je     1897 <__scif_flush+0x2c7>
    1880:	ba 02 00 00 00       	mov    $0x2,%edx
    1885:	4c 89 fe             	mov    %r15,%rsi
    1888:	4c 89 ef             	mov    %r13,%rdi
    188b:	e8 00 00 00 00       	callq  1890 <__scif_flush+0x2c0>
    1890:	8b 03                	mov    (%rbx),%eax
    1892:	83 f8 09             	cmp    $0x9,%eax
    1895:	75 d9                	jne    1870 <__scif_flush+0x2a0>
    1897:	4c 89 fe             	mov    %r15,%rsi
    189a:	4c 89 ef             	mov    %r13,%rdi
    189d:	e8 00 00 00 00       	callq  18a2 <__scif_flush+0x2d2>
				(ep->state == SCIFEP_DISCONNECTED), NODE_ALIVE_TIMEOUT);
		spin_lock_irqsave(&ms_info.mi_connlock, sflags);
    18a2:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    18a9:	e8 00 00 00 00       	callq  18ae <__scif_flush+0x2de>
	raw_spin_lock_init(&(_lock)->rlock);		\
} while (0)

static inline void spin_lock(spinlock_t *lock)
{
	raw_spin_lock(&lock->rlock);
    18ae:	4c 89 e7             	mov    %r12,%rdi
    18b1:	49 89 c5             	mov    %rax,%r13
    18b4:	e8 00 00 00 00       	callq  18b9 <__scif_flush+0x2e9>
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
    18b9:	48 8b 15 00 00 00 00 	mov    0x0(%rip),%rdx        # 18c0 <__scif_flush+0x2f0>
		spin_lock(&ep->lock);
		list_add_tail(&ep->list, &ms_info.mi_disconnected);
    18c0:	48 8d 8b 40 02 00 00 	lea    0x240(%rbx),%rcx
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
    18c7:	48 c7 83 40 02 00 00 	movq   $0x0,0x240(%rbx)
    18ce:	00 00 00 00 
#ifndef CONFIG_DEBUG_LIST
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
    18d2:	48 89 0d 00 00 00 00 	mov    %rcx,0x0(%rip)        # 18d9 <__scif_flush+0x309>
	new->next = next;
	new->prev = prev;
    18d9:	48 89 93 48 02 00 00 	mov    %rdx,0x248(%rbx)
	prev->next = new;
    18e0:	48 89 0a             	mov    %rcx,(%rdx)
		ep->state = SCIFEP_DISCONNECTED;
    18e3:	c7 03 09 00 00 00    	movl   $0x9,(%rbx)
    18e9:	fe 43 04             	incb   0x4(%rbx)
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    18ec:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    18f3:	4c 89 ee             	mov    %r13,%rsi
    18f6:	e8 00 00 00 00       	callq  18fb <__scif_flush+0x32b>
		spin_unlock(&ep->lock);
		spin_unlock_irqrestore(&ms_info.mi_connlock, sflags);
		// Wake up threads blocked in send and recv
		wake_up_interruptible(&ep->sendwq);
    18fb:	48 8d bb d0 01 00 00 	lea    0x1d0(%rbx),%rdi
    1902:	31 c9                	xor    %ecx,%ecx
    1904:	ba 01 00 00 00       	mov    $0x1,%edx
    1909:	be 01 00 00 00       	mov    $0x1,%esi
    190e:	e8 00 00 00 00       	callq  1913 <__scif_flush+0x343>
		wake_up_interruptible(&ep->recvwq);
    1913:	48 8d bb e8 01 00 00 	lea    0x1e8(%rbx),%rdi
    191a:	31 c9                	xor    %ecx,%ecx
    191c:	ba 01 00 00 00       	mov    $0x1,%edx
    1921:	be 01 00 00 00       	mov    $0x1,%esi
    1926:	e8 00 00 00 00       	callq  192b <__scif_flush+0x35b>
		break;
    192b:	e9 db fd ff ff       	jmpq   170b <__scif_flush+0x13b>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    1930:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    1937:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    1938:	49 8d bc 24 88 01 00 	lea    0x188(%r12),%rdi
    193f:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1940:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    1947:	00 00 80 
    194a:	48 39 f1             	cmp    %rsi,%rcx
    194d:	74 3f                	je     198e <__scif_flush+0x3be>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    194f:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1953:	48 89 c8             	mov    %rcx,%rax
    1956:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    195d:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1960:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1963:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1966:	0f 84 92 fc ff ff    	je     15fe <__scif_flush+0x2e>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    196c:	48 39 f2             	cmp    %rsi,%rdx
    196f:	74 1d                	je     198e <__scif_flush+0x3be>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    1971:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1975:	48 89 d0             	mov    %rdx,%rax
    1978:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    197d:	48 39 c2             	cmp    %rax,%rdx
    1980:	0f 84 78 fc ff ff    	je     15fe <__scif_flush+0x2e>
    1986:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1989:	48 39 f2             	cmp    %rsi,%rdx
    198c:	75 e3                	jne    1971 <__scif_flush+0x3a1>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    198e:	4d 8d ac 24 68 01 00 	lea    0x168(%r12),%r13
    1995:	00 
    1996:	4c 89 ef             	mov    %r13,%rdi
    1999:	e8 00 00 00 00       	callq  199e <__scif_flush+0x3ce>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    199e:	41 8b 44 24 04       	mov    0x4(%r12),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    19a3:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    19a6:	83 fa 01             	cmp    $0x1,%edx
    19a9:	76 20                	jbe    19cb <__scif_flush+0x3fb>
    19ab:	83 f8 01             	cmp    $0x1,%eax
    19ae:	74 1b                	je     19cb <__scif_flush+0x3fb>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    19b0:	49 8b 84 24 88 01 00 	mov    0x188(%r12),%rax
    19b7:	00 
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    19b8:	48 85 c0             	test   %rax,%rax
    19bb:	0f 88 02 01 00 00    	js     1ac3 <__scif_flush+0x4f3>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
    19c1:	f0 49 83 84 24 88 01 	lock addq $0x1,0x188(%r12)
    19c8:	00 00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    19cb:	4c 89 ef             	mov    %r13,%rdi
    19ce:	e8 00 00 00 00       	callq  19d3 <__scif_flush+0x403>
    19d3:	e9 26 fc ff ff       	jmpq   15fe <__scif_flush+0x2e>
    19d8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    19df:	00 
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    19e0:	48 8b 45 08          	mov    0x8(%rbp),%rax
    19e4:	48 89 d9             	mov    %rbx,%rcx
    19e7:	ba a7 00 00 00       	mov    $0xa7,%edx
    19ec:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    19f3:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    19fa:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    1a01:	44 0f b7 03          	movzwl (%rbx),%r8d
    1a05:	48 89 04 24          	mov    %rax,(%rsp)
    1a09:	31 c0                	xor    %eax,%eax
    1a0b:	e8 00 00 00 00       	callq  1a10 <__scif_flush+0x440>
    1a10:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1a17:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    1a1e:	00 00 80 
    1a21:	48 39 f1             	cmp    %rsi,%rcx
    1a24:	0f 84 0e fd ff ff    	je     1738 <__scif_flush+0x168>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    1a2a:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1a2e:	48 89 c8             	mov    %rcx,%rax
    1a31:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    1a38:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1a3a:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1a3d:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1a40:	0f 84 f2 fc ff ff    	je     1738 <__scif_flush+0x168>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1a46:	48 39 f2             	cmp    %rsi,%rdx
    1a49:	0f 84 e9 fc ff ff    	je     1738 <__scif_flush+0x168>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    1a4f:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    1a53:	48 89 d0             	mov    %rdx,%rax
    1a56:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    1a5c:	48 39 c2             	cmp    %rax,%rdx
    1a5f:	0f 84 d3 fc ff ff    	je     1738 <__scif_flush+0x168>
    1a65:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    1a68:	48 39 f2             	cmp    %rsi,%rdx
    1a6b:	75 e2                	jne    1a4f <__scif_flush+0x47f>
    1a6d:	e9 c6 fc ff ff       	jmpq   1738 <__scif_flush+0x168>
    1a72:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
			// the end point can be removed from the list.  Therefore
			// the ep lock is not locked, traverse the disconnected list
			// to find the endpoint, release the conn lock.
			list_for_each_safe(pos, tmpq, &ms_info.mi_disconnected) {
				tmpep = list_entry(pos, struct endpt, list);
				if (tmpep == ep) {
    1a78:	48 89 c2             	mov    %rax,%rdx
    1a7b:	48 89 c8             	mov    %rcx,%rax
    1a7e:	66 90                	xchg   %ax,%ax
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    1a80:	48 be 00 01 10 00 00 	movabs $0xdead000000100100,%rsi
    1a87:	00 ad de 
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    1a8a:	48 8b 48 08          	mov    0x8(%rax),%rcx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    1a8e:	48 89 4a 08          	mov    %rcx,0x8(%rdx)
	prev->next = next;
    1a92:	48 89 11             	mov    %rdx,(%rcx)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    1a95:	48 89 30             	mov    %rsi,(%rax)
	entry->prev = LIST_POISON2;
    1a98:	48 be 00 02 20 00 00 	movabs $0xdead000000200200,%rsi
    1a9f:	00 ad de 
    1aa2:	48 89 70 08          	mov    %rsi,0x8(%rax)
    1aa6:	e9 43 fc ff ff       	jmpq   16ee <__scif_flush+0x11e>
	{
		struct nodemsg msg;
		struct endpt *fep = NULL;

		init_waitqueue_head(&ep->disconwq);	// Wait for connection queue
		WARN_ON(ep->files); // files should never be set while connected
    1aab:	be 8f 01 00 00       	mov    $0x18f,%esi
    1ab0:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    1ab7:	e8 00 00 00 00       	callq  1abc <__scif_flush+0x4ec>
    1abc:	e9 84 fb ff ff       	jmpq   1645 <__scif_flush+0x75>
    1ac1:	0f 0b                	ud2    
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    1ac3:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 1ac9 <__scif_flush+0x4f9>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    1ac9:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    1acb:	49 c7 84 24 b0 01 00 	movq   $0x2,0x1b0(%r12)
    1ad2:	00 02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    1ad7:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    1ade:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    1ae5:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    1ae9:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
    1aed:	31 c0                	xor    %eax,%eax
    1aef:	66 89 45 a8          	mov    %ax,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
    1af3:	41 0f b7 04 24       	movzwl (%r12),%eax
    1af8:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    1afc:	e8 00 00 00 00       	callq  1b01 <__scif_flush+0x531>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    1b01:	49 8b 84 24 b0 01 00 	mov    0x1b0(%r12),%rax
    1b08:	00 
    1b09:	48 83 f8 02          	cmp    $0x2,%rax
    1b0d:	74 22                	je     1b31 <__scif_flush+0x561>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    1b0f:	48 83 f8 03          	cmp    $0x3,%rax
    1b13:	0f 85 a8 fe ff ff    	jne    19c1 <__scif_flush+0x3f1>
				dev->sd_state = SCIFDEV_RUNNING;
    1b19:	41 c7 44 24 04 02 00 	movl   $0x2,0x4(%r12)
    1b20:	00 00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    1b22:	f0 41 80 a4 24 8f 01 	lock andb $0x7f,0x18f(%r12)
    1b29:	00 00 7f 
    1b2c:	e9 90 fe ff ff       	jmpq   19c1 <__scif_flush+0x3f1>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    1b31:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    1b38:	30 c0                	xor    %al,%al
    1b3a:	b9 0a 00 00 00       	mov    $0xa,%ecx
    1b3f:	f3 ab                	rep stos %eax,%es:(%rdi)
    1b41:	4c 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%r15
    1b48:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    1b4f:	00 
    1b50:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    1b57:	00 00 
    1b59:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    1b5d:	49 8d 47 18          	lea    0x18(%r15),%rax
    1b61:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    1b65:	4d 8d b4 24 98 01 00 	lea    0x198(%r12),%r14
    1b6c:	00 
    1b6d:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    1b71:	eb 0a                	jmp    1b7d <__scif_flush+0x5ad>
    1b73:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    1b78:	e8 00 00 00 00       	callq  1b7d <__scif_flush+0x5ad>
    1b7d:	ba 02 00 00 00       	mov    $0x2,%edx
    1b82:	4c 89 fe             	mov    %r15,%rsi
    1b85:	4c 89 f7             	mov    %r14,%rdi
    1b88:	e8 00 00 00 00       	callq  1b8d <__scif_flush+0x5bd>
    1b8d:	49 83 bc 24 b0 01 00 	cmpq   $0x2,0x1b0(%r12)
    1b94:	00 02 
    1b96:	74 e0                	je     1b78 <__scif_flush+0x5a8>
    1b98:	4c 89 fe             	mov    %r15,%rsi
    1b9b:	4c 89 f7             	mov    %r14,%rdi
    1b9e:	e8 00 00 00 00       	callq  1ba3 <__scif_flush+0x5d3>
    1ba3:	49 8b 84 24 b0 01 00 	mov    0x1b0(%r12),%rax
    1baa:	00 
    1bab:	e9 5f ff ff ff       	jmpq   1b0f <__scif_flush+0x53f>

0000000000001bb0 <scif_close>:
	__scif_close((scif_epd_t)epd);
}

int
scif_close(scif_epd_t epd)
{
    1bb0:	55                   	push   %rbp
    1bb1:	48 89 e5             	mov    %rsp,%rbp
    1bb4:	53                   	push   %rbx
    1bb5:	48 83 ec 08          	sub    $0x8,%rsp
    1bb9:	e8 00 00 00 00       	callq  1bbe <scif_close+0xe>
    1bbe:	48 89 fb             	mov    %rdi,%rbx
	__scif_flush(epd);
    1bc1:	e8 00 00 00 00       	callq  1bc6 <scif_close+0x16>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    1bc6:	48 8d bb 70 01 00 00 	lea    0x170(%rbx),%rdi
    1bcd:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    1bd4:	e8 00 00 00 00       	callq  1bd9 <scif_close+0x29>
	put_kref_count(epd);
	return 0;
}
    1bd9:	48 83 c4 08          	add    $0x8,%rsp
    1bdd:	31 c0                	xor    %eax,%eax
    1bdf:	5b                   	pop    %rbx
    1be0:	5d                   	pop    %rbp
    1be1:	c3                   	retq   
    1be2:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    1be9:	1f 84 00 00 00 00 00 

0000000000001bf0 <__scif_bind>:
 * If port ID zero is specified and allocation of a port ID fails -ENOSPC
 * will be returned.
 */
int
__scif_bind(scif_epd_t epd, uint16_t pn)
{
    1bf0:	55                   	push   %rbp
    1bf1:	48 89 e5             	mov    %rsp,%rbp
    1bf4:	41 57                	push   %r15
    1bf6:	41 56                	push   %r14
    1bf8:	41 55                	push   %r13
    1bfa:	41 54                	push   %r12
    1bfc:	53                   	push   %rbx
    1bfd:	48 83 ec 18          	sub    $0x18,%rsp
    1c01:	e8 00 00 00 00       	callq  1c06 <__scif_bind+0x16>
	struct endpt *ep = (struct endpt *)epd;
	unsigned long sflags;
	int ret = 0;
	int tmp;

	pr_debug("SCIFAPI bind: ep %p %s requested port number %d\n", 
    1c06:	8b 07                	mov    (%rdi),%eax
 * If port ID zero is specified and allocation of a port ID fails -ENOSPC
 * will be returned.
 */
int
__scif_bind(scif_epd_t epd, uint16_t pn)
{
    1c08:	41 89 f4             	mov    %esi,%r12d
    1c0b:	48 89 fb             	mov    %rdi,%rbx
    1c0e:	41 89 f7             	mov    %esi,%r15d
	int tmp;

	pr_debug("SCIFAPI bind: ep %p %s requested port number %d\n", 
		    ep, scif_ep_states[ep->state], pn);

	might_sleep();
    1c11:	e8 00 00 00 00       	callq  1c16 <__scif_bind+0x26>
		 * Modeled on http://www.ietf.org/rfc/rfc1700.txt?number=1700
		 * SCIF ports below SCIF_ADMIN_PORT_END can only be bound by
		 * system (or root) processes or by processes executed by
		 * privileged users.
		 */
		if ( pn < SCIF_ADMIN_PORT_END && !capable(CAP_SYS_ADMIN)) {
    1c16:	41 8d 44 24 ff       	lea    -0x1(%r12),%eax
    1c1b:	66 3d fe 03          	cmp    $0x3fe,%ax
    1c1f:	76 6f                	jbe    1c90 <__scif_bind+0xa0>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    1c21:	4c 8d 6b 04          	lea    0x4(%rbx),%r13
			ret = -EACCES;
			goto scif_bind_admin_exit;
		}
	}

	spin_lock_irqsave(&ep->lock, sflags);
    1c25:	4c 89 ef             	mov    %r13,%rdi
    1c28:	e8 00 00 00 00       	callq  1c2d <__scif_bind+0x3d>
    1c2d:	48 89 c6             	mov    %rax,%rsi
	if (ep->state == SCIFEP_BOUND) {
    1c30:	8b 03                	mov    (%rbx),%eax
    1c32:	83 f8 02             	cmp    $0x2,%eax
    1c35:	74 4b                	je     1c82 <__scif_bind+0x92>
		ret = -EINVAL;
		goto scif_bind_exit;
	} else if (ep->state != SCIFEP_UNBOUND) {
    1c37:	8b 03                	mov    (%rbx),%eax
		ret = -EISCONN;
    1c39:	41 be 96 ff ff ff    	mov    $0xffffff96,%r14d

	spin_lock_irqsave(&ep->lock, sflags);
	if (ep->state == SCIFEP_BOUND) {
		ret = -EINVAL;
		goto scif_bind_exit;
	} else if (ep->state != SCIFEP_UNBOUND) {
    1c3f:	83 f8 01             	cmp    $0x1,%eax
    1c42:	74 1c                	je     1c60 <__scif_bind+0x70>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1c44:	4c 89 ef             	mov    %r13,%rdi
    1c47:	e8 00 00 00 00       	callq  1c4c <__scif_bind+0x5c>

scif_bind_exit:
	spin_unlock_irqrestore(&ep->lock, sflags);
scif_bind_admin_exit:
	return ret;
}
    1c4c:	48 83 c4 18          	add    $0x18,%rsp
    1c50:	44 89 f0             	mov    %r14d,%eax
    1c53:	5b                   	pop    %rbx
    1c54:	41 5c                	pop    %r12
    1c56:	41 5d                	pop    %r13
    1c58:	41 5e                	pop    %r14
    1c5a:	41 5f                	pop    %r15
    1c5c:	5d                   	pop    %rbp
    1c5d:	c3                   	retq   
    1c5e:	66 90                	xchg   %ax,%ax
	} else if (ep->state != SCIFEP_UNBOUND) {
		ret = -EISCONN;
		goto scif_bind_exit;
	}

	if (pn) {
    1c60:	66 45 85 e4          	test   %r12w,%r12w
    1c64:	48 89 75 c8          	mov    %rsi,-0x38(%rbp)
    1c68:	74 46                	je     1cb0 <__scif_bind+0xc0>
		if ((tmp = rsrv_scif_port(pn)) != pn) {
    1c6a:	45 0f b7 f4          	movzwl %r12w,%r14d
    1c6e:	44 89 f7             	mov    %r14d,%edi
    1c71:	e8 00 00 00 00       	callq  1c76 <__scif_bind+0x86>
    1c76:	48 8b 75 c8          	mov    -0x38(%rbp),%rsi
    1c7a:	0f b7 c0             	movzwl %ax,%eax
    1c7d:	41 39 c6             	cmp    %eax,%r14d
    1c80:	74 4d                	je     1ccf <__scif_bind+0xdf>
		}
	}

	spin_lock_irqsave(&ep->lock, sflags);
	if (ep->state == SCIFEP_BOUND) {
		ret = -EINVAL;
    1c82:	41 be ea ff ff ff    	mov    $0xffffffea,%r14d
    1c88:	eb ba                	jmp    1c44 <__scif_bind+0x54>
    1c8a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		 * Modeled on http://www.ietf.org/rfc/rfc1700.txt?number=1700
		 * SCIF ports below SCIF_ADMIN_PORT_END can only be bound by
		 * system (or root) processes or by processes executed by
		 * privileged users.
		 */
		if ( pn < SCIF_ADMIN_PORT_END && !capable(CAP_SYS_ADMIN)) {
    1c90:	bf 15 00 00 00       	mov    $0x15,%edi
			ret = -EACCES;
    1c95:	41 be f3 ff ff ff    	mov    $0xfffffff3,%r14d
		 * Modeled on http://www.ietf.org/rfc/rfc1700.txt?number=1700
		 * SCIF ports below SCIF_ADMIN_PORT_END can only be bound by
		 * system (or root) processes or by processes executed by
		 * privileged users.
		 */
		if ( pn < SCIF_ADMIN_PORT_END && !capable(CAP_SYS_ADMIN)) {
    1c9b:	e8 00 00 00 00       	callq  1ca0 <__scif_bind+0xb0>
    1ca0:	84 c0                	test   %al,%al
    1ca2:	0f 85 79 ff ff ff    	jne    1c21 <__scif_bind+0x31>
    1ca8:	eb a2                	jmp    1c4c <__scif_bind+0x5c>
    1caa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		if ((tmp = rsrv_scif_port(pn)) != pn) {
			ret = -EINVAL;
			goto scif_bind_exit;
		}
	} else {
		pn = get_scif_port();
    1cb0:	e8 00 00 00 00       	callq  1cb5 <__scif_bind+0xc5>
		if (!pn) {
			ret = -ENOSPC;
    1cb5:	41 be e4 ff ff ff    	mov    $0xffffffe4,%r14d
			ret = -EINVAL;
			goto scif_bind_exit;
		}
	} else {
		pn = get_scif_port();
		if (!pn) {
    1cbb:	48 8b 75 c8          	mov    -0x38(%rbp),%rsi
    1cbf:	66 85 c0             	test   %ax,%ax
		if ((tmp = rsrv_scif_port(pn)) != pn) {
			ret = -EINVAL;
			goto scif_bind_exit;
		}
	} else {
		pn = get_scif_port();
    1cc2:	41 89 c7             	mov    %eax,%r15d
		if (!pn) {
    1cc5:	0f 84 79 ff ff ff    	je     1c44 <__scif_bind+0x54>
    1ccb:	44 0f b7 f0          	movzwl %ax,%r14d
			ret = -ENOSPC;
			goto scif_bind_exit;
		}
	}

	ep->state = SCIFEP_BOUND;
    1ccf:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
	ep->port.node = ms_info.mi_nodeid;
    1cd5:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 1cdb <__scif_bind+0xeb>
	ep->port.port = pn;
    1cdb:	66 44 89 7b 08       	mov    %r15w,0x8(%rbx)
			goto scif_bind_exit;
		}
	}

	ep->state = SCIFEP_BOUND;
	ep->port.node = ms_info.mi_nodeid;
    1ce0:	66 89 43 06          	mov    %ax,0x6(%rbx)
    1ce4:	e9 5b ff ff ff       	jmpq   1c44 <__scif_bind+0x54>
    1ce9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000001cf0 <scif_bind>:
	return ret;
}

int
scif_bind(scif_epd_t epd, uint16_t pn)
{
    1cf0:	55                   	push   %rbp
    1cf1:	48 89 e5             	mov    %rsp,%rbp
    1cf4:	41 55                	push   %r13
    1cf6:	41 54                	push   %r12
    1cf8:	53                   	push   %rbx
    1cf9:	48 83 ec 08          	sub    $0x8,%rsp
    1cfd:	e8 00 00 00 00       	callq  1d02 <scif_bind+0x12>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    1d02:	4c 8d a7 70 01 00 00 	lea    0x170(%rdi),%r12
    1d09:	49 89 fd             	mov    %rdi,%r13
    1d0c:	89 f3                	mov    %esi,%ebx
    1d0e:	4c 89 e7             	mov    %r12,%rdi
    1d11:	e8 00 00 00 00       	callq  1d16 <scif_bind+0x26>
	int ret;
	get_kref_count(epd);
	ret = __scif_bind(epd, pn);
    1d16:	0f b7 f3             	movzwl %bx,%esi
    1d19:	4c 89 ef             	mov    %r13,%rdi
    1d1c:	e8 00 00 00 00       	callq  1d21 <scif_bind+0x31>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    1d21:	4c 89 e7             	mov    %r12,%rdi
    1d24:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    1d2b:	89 c3                	mov    %eax,%ebx
    1d2d:	e8 00 00 00 00       	callq  1d32 <scif_bind+0x42>
	put_kref_count(epd);
	return ret;
}
    1d32:	48 83 c4 08          	add    $0x8,%rsp
    1d36:	89 d8                	mov    %ebx,%eax
    1d38:	5b                   	pop    %rbx
    1d39:	41 5c                	pop    %r12
    1d3b:	41 5d                	pop    %r13
    1d3d:	5d                   	pop    %rbp
    1d3e:	c3                   	retq   
    1d3f:	90                   	nop

0000000000001d40 <__scif_listen>:
 * If the end point is not in the bound state -EINVAL or -EISCONN is returned.
 *
 */
int
__scif_listen(scif_epd_t epd, int backlog)
{
    1d40:	55                   	push   %rbp
    1d41:	48 89 e5             	mov    %rsp,%rbp
    1d44:	41 56                	push   %r14
    1d46:	41 55                	push   %r13
    1d48:	41 54                	push   %r12
    1d4a:	53                   	push   %rbx
    1d4b:	e8 00 00 00 00       	callq  1d50 <__scif_listen+0x10>
	struct endpt *ep = (struct endpt *)epd;
	unsigned long sflags;

	pr_debug("SCIFAPI listen: ep %p %s\n", ep, scif_ep_states[ep->state]);
    1d50:	8b 07                	mov    (%rdi),%eax
 * If the end point is not in the bound state -EINVAL or -EISCONN is returned.
 *
 */
int
__scif_listen(scif_epd_t epd, int backlog)
{
    1d52:	48 89 fb             	mov    %rdi,%rbx
    1d55:	41 89 f5             	mov    %esi,%r13d
	struct endpt *ep = (struct endpt *)epd;
	unsigned long sflags;

	pr_debug("SCIFAPI listen: ep %p %s\n", ep, scif_ep_states[ep->state]);

	might_sleep();
    1d58:	e8 00 00 00 00       	callq  1d5d <__scif_listen+0x1d>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    1d5d:	4c 8d 63 04          	lea    0x4(%rbx),%r12
	spin_lock_irqsave(&ep->lock, sflags);
    1d61:	4c 89 e7             	mov    %r12,%rdi
    1d64:	e8 00 00 00 00       	callq  1d69 <__scif_listen+0x29>
	switch (ep->state) {
    1d69:	8b 0b                	mov    (%rbx),%ecx
	unsigned long sflags;

	pr_debug("SCIFAPI listen: ep %p %s\n", ep, scif_ep_states[ep->state]);

	might_sleep();
	spin_lock_irqsave(&ep->lock, sflags);
    1d6b:	49 89 c6             	mov    %rax,%r14
    1d6e:	83 f9 0a             	cmp    $0xa,%ecx
    1d71:	77 45                	ja     1db8 <__scif_listen+0x78>
    1d73:	ba 01 00 00 00       	mov    $0x1,%edx
    1d78:	48 d3 e2             	shl    %cl,%rdx
    1d7b:	f7 c2 83 03 00 00    	test   $0x383,%edx
    1d81:	75 19                	jne    1d9c <__scif_listen+0x5c>
    1d83:	f6 c2 78             	test   $0x78,%dl
    1d86:	0f 85 f6 00 00 00    	jne    1e82 <__scif_listen+0x142>
    1d8c:	80 e6 04             	and    $0x4,%dh
    1d8f:	74 27                	je     1db8 <__scif_listen+0x78>
	switch (ep->state) {
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == ep->state);
    1d91:	8b 03                	mov    (%rbx),%eax
    1d93:	83 f8 0a             	cmp    $0xa,%eax
    1d96:	0f 84 e4 00 00 00    	je     1e80 <__scif_listen+0x140>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1d9c:	4c 89 f6             	mov    %r14,%rsi
    1d9f:	4c 89 e7             	mov    %r12,%rdi
    1da2:	e8 00 00 00 00       	callq  1da7 <__scif_listen+0x67>

	spin_lock_irqsave(&ms_info.mi_eplock, sflags);
	list_add_tail(&ep->list, &ms_info.mi_listen);
	spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
	return 0;
}
    1da7:	5b                   	pop    %rbx
	case SCIFEP_CLOSING:
	case SCIFEP_CLLISTEN:
	case SCIFEP_UNBOUND:
	case SCIFEP_DISCONNECTED:
		spin_unlock_irqrestore(&ep->lock, sflags);
		return -EINVAL;
    1da8:	b8 ea ff ff ff       	mov    $0xffffffea,%eax

	spin_lock_irqsave(&ms_info.mi_eplock, sflags);
	list_add_tail(&ep->list, &ms_info.mi_listen);
	spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
	return 0;
}
    1dad:	41 5c                	pop    %r12
    1daf:	41 5d                	pop    %r13
    1db1:	41 5e                	pop    %r14
    1db3:	5d                   	pop    %rbp
    1db4:	c3                   	retq   
    1db5:	0f 1f 00             	nopl   (%rax)
	ep->state = SCIFEP_LISTENING;
	ep->backlog = backlog;

	ep->conreqcnt = 0;
	ep->acceptcnt = 0;
	INIT_LIST_HEAD(&ep->conlist);	// List of connection requests
    1db8:	48 8d 93 78 01 00 00 	lea    0x178(%rbx),%rdx
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
    1dbf:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
		return -EISCONN;
	case SCIFEP_BOUND:
		break;
	}

	ep->state = SCIFEP_LISTENING;
    1dc6:	c7 03 03 00 00 00    	movl   $0x3,(%rbx)
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
    1dcc:	48 89 93 78 01 00 00 	mov    %rdx,0x178(%rbx)
	ep->backlog = backlog;

	ep->conreqcnt = 0;
	ep->acceptcnt = 0;
	INIT_LIST_HEAD(&ep->conlist);	// List of connection requests
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
    1dd3:	48 8d bb 88 01 00 00 	lea    0x188(%rbx),%rdi
	list->prev = list;
    1dda:	48 89 93 80 01 00 00 	mov    %rdx,0x180(%rbx)
	case SCIFEP_BOUND:
		break;
	}

	ep->state = SCIFEP_LISTENING;
	ep->backlog = backlog;
    1de1:	44 88 6b 0e          	mov    %r13b,0xe(%rbx)

	ep->conreqcnt = 0;
    1de5:	c7 83 58 01 00 00 00 	movl   $0x0,0x158(%rbx)
    1dec:	00 00 00 
	ep->acceptcnt = 0;
    1def:	c7 83 60 02 00 00 00 	movl   $0x0,0x260(%rbx)
    1df6:	00 00 00 
	INIT_LIST_HEAD(&ep->conlist);	// List of connection requests
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
    1df9:	e8 00 00 00 00       	callq  1dfe <__scif_listen+0xbe>
	INIT_LIST_HEAD(&ep->li_accept);	// User ep list for ACCEPTREG calls
    1dfe:	48 8d 93 50 02 00 00 	lea    0x250(%rbx),%rdx
    1e05:	4c 89 f6             	mov    %r14,%rsi
    1e08:	4c 89 e7             	mov    %r12,%rdi
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
    1e0b:	48 89 93 50 02 00 00 	mov    %rdx,0x250(%rbx)
	list->prev = list;
    1e12:	48 89 93 58 02 00 00 	mov    %rdx,0x258(%rbx)
    1e19:	e8 00 00 00 00       	callq  1e1e <__scif_listen+0xde>
	spin_unlock_irqrestore(&ep->lock, sflags);

	// Listen status is complete so delete the qp information not needed
	// on a listen before placing on the list of listening ep's
	micscif_teardown_ep((void *)ep);
    1e1e:	48 89 df             	mov    %rbx,%rdi
    1e21:	e8 00 00 00 00       	callq  1e26 <__scif_listen+0xe6>
	ep->qp_info.qp = NULL;

	spin_lock_irqsave(&ms_info.mi_eplock, sflags);
    1e26:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
	spin_unlock_irqrestore(&ep->lock, sflags);

	// Listen status is complete so delete the qp information not needed
	// on a listen before placing on the list of listening ep's
	micscif_teardown_ep((void *)ep);
	ep->qp_info.qp = NULL;
    1e2d:	48 c7 43 10 00 00 00 	movq   $0x0,0x10(%rbx)
    1e34:	00 

	spin_lock_irqsave(&ms_info.mi_eplock, sflags);
    1e35:	e8 00 00 00 00       	callq  1e3a <__scif_listen+0xfa>
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
    1e3a:	48 8b 15 00 00 00 00 	mov    0x0(%rip),%rdx        # 1e41 <__scif_listen+0x101>
	list_add_tail(&ep->list, &ms_info.mi_listen);
    1e41:	48 8d 8b 40 02 00 00 	lea    0x240(%rbx),%rcx
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
    1e48:	48 c7 83 40 02 00 00 	movq   $0x0,0x240(%rbx)
    1e4f:	00 00 00 00 
#ifndef CONFIG_DEBUG_LIST
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
    1e53:	48 89 0d 00 00 00 00 	mov    %rcx,0x0(%rip)        # 1e5a <__scif_listen+0x11a>
    1e5a:	48 89 c6             	mov    %rax,%rsi
    1e5d:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
	new->next = next;
	new->prev = prev;
    1e64:	48 89 93 48 02 00 00 	mov    %rdx,0x248(%rbx)
	prev->next = new;
    1e6b:	48 89 0a             	mov    %rcx,(%rdx)
    1e6e:	e8 00 00 00 00       	callq  1e73 <__scif_listen+0x133>
	spin_unlock_irqrestore(&ms_info.mi_eplock, sflags);
	return 0;
    1e73:	31 c0                	xor    %eax,%eax
}
    1e75:	5b                   	pop    %rbx
    1e76:	41 5c                	pop    %r12
    1e78:	41 5d                	pop    %r13
    1e7a:	41 5e                	pop    %r14
    1e7c:	5d                   	pop    %rbp
    1e7d:	c3                   	retq   
    1e7e:	66 90                	xchg   %ax,%ax

	might_sleep();
	spin_lock_irqsave(&ep->lock, sflags);
	switch (ep->state) {
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == ep->state);
    1e80:	0f 0b                	ud2    
    1e82:	48 89 c6             	mov    %rax,%rsi
    1e85:	4c 89 e7             	mov    %r12,%rdi
    1e88:	e8 00 00 00 00       	callq  1e8d <__scif_listen+0x14d>
	case SCIFEP_LISTENING:
	case SCIFEP_CONNECTED:
	case SCIFEP_CONNECTING:
	case SCIFEP_MAPPING:
		spin_unlock_irqrestore(&ep->lock, sflags);
		return -EISCONN;
    1e8d:	b8 96 ff ff ff       	mov    $0xffffff96,%eax
    1e92:	eb e1                	jmp    1e75 <__scif_listen+0x135>
    1e94:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
    1e9b:	00 00 00 00 00 

0000000000001ea0 <scif_listen>:
	return 0;
}

int
scif_listen(scif_epd_t epd, int backlog)
{
    1ea0:	55                   	push   %rbp
    1ea1:	48 89 e5             	mov    %rsp,%rbp
    1ea4:	41 55                	push   %r13
    1ea6:	41 54                	push   %r12
    1ea8:	53                   	push   %rbx
    1ea9:	48 83 ec 08          	sub    $0x8,%rsp
    1ead:	e8 00 00 00 00       	callq  1eb2 <scif_listen+0x12>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    1eb2:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    1eb9:	49 89 fc             	mov    %rdi,%r12
    1ebc:	41 89 f5             	mov    %esi,%r13d
    1ebf:	48 89 df             	mov    %rbx,%rdi
    1ec2:	e8 00 00 00 00       	callq  1ec7 <scif_listen+0x27>
	int ret;
	get_kref_count(epd);
	ret = __scif_listen(epd, backlog);
    1ec7:	44 89 ee             	mov    %r13d,%esi
    1eca:	4c 89 e7             	mov    %r12,%rdi
    1ecd:	e8 00 00 00 00       	callq  1ed2 <scif_listen+0x32>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    1ed2:	48 89 df             	mov    %rbx,%rdi
    1ed5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    1edc:	41 89 c4             	mov    %eax,%r12d
    1edf:	e8 00 00 00 00       	callq  1ee4 <scif_listen+0x44>
	put_kref_count(epd);
	return ret;
}
    1ee4:	48 83 c4 08          	add    $0x8,%rsp
    1ee8:	44 89 e0             	mov    %r12d,%eax
    1eeb:	5b                   	pop    %rbx
    1eec:	41 5c                	pop    %r12
    1eee:	41 5d                	pop    %r13
    1ef0:	5d                   	pop    %rbp
    1ef1:	c3                   	retq   
    1ef2:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    1ef9:	1f 84 00 00 00 00 00 

0000000000001f00 <__scif_connect>:
 * If the remote side is not responding to connection requests the caller may
 * terminate this funciton with a signal.  If so a -EINTR will be returned.
 */
int
__scif_connect(scif_epd_t epd, struct scif_portID *dst)
{
    1f00:	55                   	push   %rbp
    1f01:	48 89 e5             	mov    %rsp,%rbp
    1f04:	41 57                	push   %r15
    1f06:	41 56                	push   %r14
    1f08:	41 55                	push   %r13
    1f0a:	41 54                	push   %r12
    1f0c:	53                   	push   %rbx
    1f0d:	48 81 ec a8 00 00 00 	sub    $0xa8,%rsp
    1f14:	e8 00 00 00 00       	callq  1f19 <__scif_connect+0x19>
#endif

	pr_debug("SCIFAPI connect: ep %p %s\n", ep, 
					scif_ep_states[ep->state]);

	if (dst->node > MAX_BOARD_SUPPORTED)
    1f19:	66 81 3e 00 01       	cmpw   $0x100,(%rsi)
	int term_sent = 0;
#ifdef _MIC_SCIF_
	struct micscif_dev *remote_dev;
#endif

	pr_debug("SCIFAPI connect: ep %p %s\n", ep, 
    1f1e:	8b 07                	mov    (%rdi),%eax
					scif_ep_states[ep->state]);

	if (dst->node > MAX_BOARD_SUPPORTED)
    1f20:	77 4e                	ja     1f70 <__scif_connect+0x70>
    1f22:	49 89 f4             	mov    %rsi,%r12
    1f25:	48 89 fb             	mov    %rdi,%rbx
		return -ENODEV;

	might_sleep();
    1f28:	e8 00 00 00 00       	callq  1f2d <__scif_connect+0x2d>
		SCIFDEV_STOPPED == remote_dev->sd_state) && mic_p2p_enable)
		if ((err = scif_p2p_connect(dst->node)))
			goto connect_error_simple;
#endif

	if (SCIFDEV_RUNNING != scif_dev[dst->node].sd_state &&
    1f2d:	41 0f b7 04 24       	movzwl (%r12),%eax
    1f32:	48 89 c2             	mov    %rax,%rdx
    1f35:	48 c1 e0 09          	shl    $0x9,%rax
    1f39:	48 c1 e2 05          	shl    $0x5,%rdx
    1f3d:	48 29 d0             	sub    %rdx,%rax
    1f40:	8b 80 00 00 00 00    	mov    0x0(%rax),%eax
    1f46:	83 e8 02             	sub    $0x2,%eax
    1f49:	83 f8 01             	cmp    $0x1,%eax
    1f4c:	77 22                	ja     1f70 <__scif_connect+0x70>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    1f4e:	4c 8d 7b 04          	lea    0x4(%rbx),%r15
		SCIFDEV_SLEEPING != scif_dev[dst->node].sd_state)
		return -ENODEV;

	spin_lock_irqsave(&ep->lock, sflags);
    1f52:	4c 89 ff             	mov    %r15,%rdi
    1f55:	e8 00 00 00 00       	callq  1f5a <__scif_connect+0x5a>
	switch (ep->state) {
    1f5a:	8b 13                	mov    (%rbx),%edx

	if (SCIFDEV_RUNNING != scif_dev[dst->node].sd_state &&
		SCIFDEV_SLEEPING != scif_dev[dst->node].sd_state)
		return -ENODEV;

	spin_lock_irqsave(&ep->lock, sflags);
    1f5c:	49 89 c6             	mov    %rax,%r14
	switch (ep->state) {
    1f5f:	83 fa 0a             	cmp    $0xa,%edx
    1f62:	77 68                	ja     1fcc <__scif_connect+0xcc>
    1f64:	ff 24 d5 00 00 00 00 	jmpq   *0x0(,%rdx,8)
    1f6b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

	pr_debug("SCIFAPI connect: ep %p %s\n", ep, 
					scif_ep_states[ep->state]);

	if (dst->node > MAX_BOARD_SUPPORTED)
		return -ENODEV;
    1f70:	bb ed ff ff ff       	mov    $0xffffffed,%ebx

connect_simple_unlock:
	spin_unlock_irqrestore(&ep->lock, sflags);
connect_error_simple:
	return err;
}
    1f75:	48 81 c4 a8 00 00 00 	add    $0xa8,%rsp
    1f7c:	89 d8                	mov    %ebx,%eax
    1f7e:	5b                   	pop    %rbx
    1f7f:	41 5c                	pop    %r12
    1f81:	41 5d                	pop    %r13
    1f83:	41 5e                	pop    %r14
    1f85:	41 5f                	pop    %r15
    1f87:	5d                   	pop    %rbp
    1f88:	c3                   	retq   
    1f89:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
		err = -EOPNOTSUPP;
		goto connect_simple_unlock;
	case SCIFEP_CONNECTED:
	case SCIFEP_CONNECTING:
	case SCIFEP_MAPPING:
		err = -EISCONN;
    1f90:	bb 96 ff ff ff       	mov    $0xffffff96,%ebx
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    1f95:	4c 89 f6             	mov    %r14,%rsi
    1f98:	4c 89 ff             	mov    %r15,%rdi
    1f9b:	e8 00 00 00 00       	callq  1fa0 <__scif_connect+0xa0>
    1fa0:	eb d3                	jmp    1f75 <__scif_connect+0x75>
    1fa2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == ep->state);
	case SCIFEP_CLOSED:
	case SCIFEP_CLOSING:
	case SCIFEP_DISCONNECTED:
		err = -EINVAL;
    1fa8:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    1fad:	eb e6                	jmp    1f95 <__scif_connect+0x95>
    1faf:	90                   	nop
	case SCIFEP_CONNECTING:
	case SCIFEP_MAPPING:
		err = -EISCONN;
		goto connect_simple_unlock;
	case SCIFEP_UNBOUND:
		if ((ep->port.port = get_scif_port()) == 0) {
    1fb0:	e8 00 00 00 00       	callq  1fb5 <__scif_connect+0xb5>
    1fb5:	66 85 c0             	test   %ax,%ax
    1fb8:	66 89 43 08          	mov    %ax,0x8(%rbx)
    1fbc:	0f 84 ae 02 00 00    	je     2270 <__scif_connect+0x370>
			err = -ENOSPC;
			goto connect_simple_unlock;
		}
		ep->port.node = ms_info.mi_nodeid;
    1fc2:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 1fc8 <__scif_connect+0xc8>
    1fc8:	66 89 43 06          	mov    %ax,0x6(%rbx)
		break;
	}

	// State of end point now becomes CONNECTING
	ep->state = SCIFEP_CONNECTING;
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
    1fcc:	48 8d 83 88 01 00 00 	lea    0x188(%rbx),%rax
    1fd3:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
	case SCIFEP_BOUND:
		break;
	}

	// State of end point now becomes CONNECTING
	ep->state = SCIFEP_CONNECTING;
    1fda:	c7 03 05 00 00 00    	movl   $0x5,(%rbx)
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
    1fe0:	48 89 c7             	mov    %rax,%rdi
    1fe3:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    1fea:	e8 00 00 00 00       	callq  1fef <__scif_connect+0xef>
	init_waitqueue_head(&ep->diswq);	// Wait for disconnect ack queue
    1fef:	4c 8d ab b8 01 00 00 	lea    0x1b8(%rbx),%r13
    1ff6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    1ffd:	4c 89 ef             	mov    %r13,%rdi
    2000:	e8 00 00 00 00       	callq  2005 <__scif_connect+0x105>
	init_waitqueue_head(&ep->sendwq);	// Wait for data to be consumed.
    2005:	48 8d bb d0 01 00 00 	lea    0x1d0(%rbx),%rdi
    200c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2013:	e8 00 00 00 00       	callq  2018 <__scif_connect+0x118>
	init_waitqueue_head(&ep->recvwq);	// Wait for data to be produced.
    2018:	48 8d bb e8 01 00 00 	lea    0x1e8(%rbx),%rdi
    201f:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2026:	e8 00 00 00 00       	callq  202b <__scif_connect+0x12b>
	ep->remote_dev = &scif_dev[dst->node];
    202b:	41 0f b7 14 24       	movzwl (%r12),%edx
    2030:	4c 89 ff             	mov    %r15,%rdi
    2033:	4c 89 f6             	mov    %r14,%rsi
	ep->sd_state = SCIFDEV_RUNNING;

	ep->qp_info.qp->magic = SCIFEP_MAGIC;
    2036:	48 b9 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rcx
    203d:	00 1f 5c 
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
	init_waitqueue_head(&ep->diswq);	// Wait for disconnect ack queue
	init_waitqueue_head(&ep->sendwq);	// Wait for data to be consumed.
	init_waitqueue_head(&ep->recvwq);	// Wait for data to be produced.
	ep->remote_dev = &scif_dev[dst->node];
	ep->sd_state = SCIFDEV_RUNNING;
    2040:	c7 83 5c 01 00 00 02 	movl   $0x2,0x15c(%rbx)
    2047:	00 00 00 
	ep->state = SCIFEP_CONNECTING;
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
	init_waitqueue_head(&ep->diswq);	// Wait for disconnect ack queue
	init_waitqueue_head(&ep->sendwq);	// Wait for data to be consumed.
	init_waitqueue_head(&ep->recvwq);	// Wait for data to be produced.
	ep->remote_dev = &scif_dev[dst->node];
    204a:	48 89 d0             	mov    %rdx,%rax
    204d:	48 c1 e2 09          	shl    $0x9,%rdx
    2051:	48 c1 e0 05          	shl    $0x5,%rax
    2055:	48 29 c2             	sub    %rax,%rdx
	ep->sd_state = SCIFDEV_RUNNING;

	ep->qp_info.qp->magic = SCIFEP_MAGIC;
    2058:	48 8b 43 10          	mov    0x10(%rbx),%rax
	ep->state = SCIFEP_CONNECTING;
	init_waitqueue_head(&ep->conwq);	// Wait for connection queue
	init_waitqueue_head(&ep->diswq);	// Wait for disconnect ack queue
	init_waitqueue_head(&ep->sendwq);	// Wait for data to be consumed.
	init_waitqueue_head(&ep->recvwq);	// Wait for data to be produced.
	ep->remote_dev = &scif_dev[dst->node];
    205c:	48 81 c2 00 00 00 00 	add    $0x0,%rdx
    2063:	48 89 93 48 01 00 00 	mov    %rdx,0x148(%rbx)
	ep->sd_state = SCIFDEV_RUNNING;

	ep->qp_info.qp->magic = SCIFEP_MAGIC;
    206a:	48 89 48 08          	mov    %rcx,0x8(%rax)
	ep->qp_info.qp->ep = (uint64_t)ep;
    206e:	48 8b 43 10          	mov    0x10(%rbx),%rax
    2072:	48 89 18             	mov    %rbx,(%rax)
    2075:	e8 00 00 00 00       	callq  207a <__scif_connect+0x17a>
	spin_unlock_irqrestore(&ep->lock, sflags);

	if ((err = micscif_reserve_dma_chan(ep))) {
    207a:	48 89 df             	mov    %rbx,%rdi
    207d:	e8 00 00 00 00       	callq  2082 <__scif_connect+0x182>
    2082:	85 c0                	test   %eax,%eax
    2084:	41 89 c7             	mov    %eax,%r15d
    2087:	0f 85 3a 09 00 00    	jne    29c7 <__scif_connect+0xac7>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		ep->state = SCIFEP_BOUND;
		goto connect_error_simple;
	}
	// Initiate the first part of the endpoint QP setup
	err = micscif_setup_qp_connect(ep->qp_info.qp, &ep->qp_info.qp_offset,
    208d:	48 8b 8b 48 01 00 00 	mov    0x148(%rbx),%rcx
    2094:	48 8d 73 18          	lea    0x18(%rbx),%rsi
    2098:	ba 00 10 00 00       	mov    $0x1000,%edx
    209d:	48 8b 7b 10          	mov    0x10(%rbx),%rdi
    20a1:	e8 00 00 00 00       	callq  20a6 <__scif_connect+0x1a6>
			ENDPT_QP_SIZE, ep->remote_dev);
	if (err) {
    20a6:	85 c0                	test   %eax,%eax
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		ep->state = SCIFEP_BOUND;
		goto connect_error_simple;
	}
	// Initiate the first part of the endpoint QP setup
	err = micscif_setup_qp_connect(ep->qp_info.qp, &ep->qp_info.qp_offset,
    20a8:	41 89 c7             	mov    %eax,%r15d
			ENDPT_QP_SIZE, ep->remote_dev);
	if (err) {
    20ab:	0f 85 ed 08 00 00    	jne    299e <__scif_connect+0xa9e>
			__func__, err, ep->qp_info.qp_offset);
		ep->state = SCIFEP_BOUND;
		goto connect_error_simple;
	}

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    20b1:	4c 8b 83 48 01 00 00 	mov    0x148(%rbx),%r8
    20b8:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    20bc:	4d 85 c0             	test   %r8,%r8
    20bf:	0f 85 1b 05 00 00    	jne    25e0 <__scif_connect+0x6e0>

	// Format connect message and send it
	msg.src = ep->port;
    20c5:	8b 43 06             	mov    0x6(%rbx),%eax
	msg.dst = *dst;
	msg.uop = SCIF_CNCT_REQ;
	msg.payload[0] = (uint64_t)ep;
	msg.payload[1] = ep->qp_info.qp_offset;
	if ((err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    20c8:	48 89 da             	mov    %rbx,%rdx
    20cb:	4c 89 fe             	mov    %r15,%rsi
    20ce:	4c 89 c7             	mov    %r8,%rdi
	micscif_inc_node_refcnt(ep->remote_dev, 1);

	// Format connect message and send it
	msg.src = ep->port;
	msg.dst = *dst;
	msg.uop = SCIF_CNCT_REQ;
    20d1:	c7 45 ac 05 00 00 00 	movl   $0x5,-0x54(%rbp)
	msg.payload[0] = (uint64_t)ep;
    20d8:	48 89 5d b0          	mov    %rbx,-0x50(%rbp)
	}

	micscif_inc_node_refcnt(ep->remote_dev, 1);

	// Format connect message and send it
	msg.src = ep->port;
    20dc:	89 45 a4             	mov    %eax,-0x5c(%rbp)
	msg.dst = *dst;
    20df:	41 8b 04 24          	mov    (%r12),%eax
    20e3:	89 45 a8             	mov    %eax,-0x58(%rbp)
	msg.uop = SCIF_CNCT_REQ;
	msg.payload[0] = (uint64_t)ep;
	msg.payload[1] = ep->qp_info.qp_offset;
    20e6:	48 8b 43 18          	mov    0x18(%rbx),%rax
    20ea:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
	if ((err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    20ee:	e8 00 00 00 00       	callq  20f3 <__scif_connect+0x1f3>
    20f3:	85 c0                	test   %eax,%eax
    20f5:	41 89 c2             	mov    %eax,%r10d
    20f8:	0f 84 02 01 00 00    	je     2200 <__scif_connect+0x300>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    20fe:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2105:	89 c3                	mov    %eax,%ebx
    2107:	4d 85 e4             	test   %r12,%r12
    210a:	0f 84 65 fe ff ff    	je     1f75 <__scif_connect+0x75>
		if (unlikely((atomic_long_sub_return(cnt, 
    2110:	4d 8d ac 24 88 01 00 	lea    0x188(%r12),%r13
    2117:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    2118:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    211f:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    2126:	01 00 00 
    2129:	48 83 e8 01          	sub    $0x1,%rax
    212d:	0f 89 42 fe ff ff    	jns    1f75 <__scif_connect+0x75>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    2133:	48 8b 45 08          	mov    0x8(%rbp),%rax
    2137:	4c 89 e1             	mov    %r12,%rcx
    213a:	ba a7 00 00 00       	mov    $0xa7,%edx
    213f:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    2146:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    214d:	00 
    214e:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    2155:	44 89 95 68 ff ff ff 	mov    %r10d,-0x98(%rbp)
    215c:	45 0f b7 04 24       	movzwl (%r12),%r8d
    2161:	48 89 04 24          	mov    %rax,(%rsp)
    2165:	31 c0                	xor    %eax,%eax
    2167:	e8 00 00 00 00       	callq  216c <__scif_connect+0x26c>
    216c:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    2173:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    2174:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    217b:	00 00 80 
    217e:	48 39 f1             	cmp    %rsi,%rcx
    2181:	0f 84 ee fd ff ff    	je     1f75 <__scif_connect+0x75>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    2187:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    218b:	48 89 c8             	mov    %rcx,%rax
    218e:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    2195:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    2198:	44 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10d
    219f:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    21a2:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    21a5:	44 89 d3             	mov    %r10d,%ebx
    21a8:	0f 84 c7 fd ff ff    	je     1f75 <__scif_connect+0x75>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    21ae:	48 39 f2             	cmp    %rsi,%rdx
    21b1:	74 16                	je     21c9 <__scif_connect+0x2c9>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    21b3:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    21b7:	48 89 d0             	mov    %rdx,%rax
    21ba:	f0 49 0f b1 4d 00    	lock cmpxchg %rcx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    21c0:	48 39 c2             	cmp    %rax,%rdx
    21c3:	0f 85 08 07 00 00    	jne    28d1 <__scif_connect+0x9d1>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    21c9:	44 89 d3             	mov    %r10d,%ebx
    21cc:	e9 a4 fd ff ff       	jmpq   1f75 <__scif_connect+0x75>
    21d1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
		return -ENODEV;

	spin_lock_irqsave(&ep->lock, sflags);
	switch (ep->state) {
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == ep->state);
    21d8:	8b 03                	mov    (%rbx),%eax
	case SCIFEP_CLOSED:
	case SCIFEP_CLOSING:
	case SCIFEP_DISCONNECTED:
		err = -EINVAL;
    21da:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
		return -ENODEV;

	spin_lock_irqsave(&ep->lock, sflags);
	switch (ep->state) {
	case SCIFEP_ZOMBIE:
		BUG_ON(SCIFEP_ZOMBIE == ep->state);
    21df:	83 f8 0a             	cmp    $0xa,%eax
    21e2:	0f 85 ad fd ff ff    	jne    1f95 <__scif_connect+0x95>
    21e8:	0f 0b                	ud2    
    21ea:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	case SCIFEP_DISCONNECTED:
		err = -EINVAL;
		goto connect_simple_unlock;
	case SCIFEP_LISTENING:
	case SCIFEP_CLLISTEN:
		err = -EOPNOTSUPP;
    21f0:	bb a1 ff ff ff       	mov    $0xffffffa1,%ebx
    21f5:	e9 9b fd ff ff       	jmpq   1f95 <__scif_connect+0x95>
    21fa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	if ((err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;
	}
	// Wait for request to be processed.
	while ((err = wait_event_interruptible_timeout(ep->conwq, 
    2200:	8b 13                	mov    (%rbx),%edx
    2202:	44 6b 25 00 00 00 00 	imul   $0x64,0x0(%rip),%r12d        # 220a <__scif_connect+0x30a>
    2209:	64 
    220a:	83 fa 05             	cmp    $0x5,%edx
    220d:	49 63 c4             	movslq %r12d,%rax
    2210:	0f 84 3a 01 00 00    	je     2350 <__scif_connect+0x450>
    2216:	41 83 fc 00          	cmp    $0x0,%r12d
    221a:	0f 8e d0 01 00 00    	jle    23f0 <__scif_connect+0x4f0>
	if (term_sent || err) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;
	}

	if (ep->state == SCIFEP_MAPPING) {
    2220:	8b 03                	mov    (%rbx),%eax
    2222:	83 f8 06             	cmp    $0x6,%eax
    2225:	74 53                	je     227a <__scif_connect+0x37a>
		} else
			ep->state = SCIFEP_BOUND;
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;

	} else if (ep->state == SCIFEP_BOUND) {
    2227:	8b 03                	mov    (%rbx),%eax
		pr_debug("SCIFAPI connect: ep %p connection refused\n", ep);
		err = -ECONNREFUSED;
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    2229:	48 8b 8b 48 01 00 00 	mov    0x148(%rbx),%rcx
		} else
			ep->state = SCIFEP_BOUND;
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;

	} else if (ep->state == SCIFEP_BOUND) {
    2230:	83 f8 02             	cmp    $0x2,%eax
    2233:	0f 84 2f 04 00 00    	je     2668 <__scif_connect+0x768>
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2239:	48 85 c9             	test   %rcx,%rcx
    223c:	74 21                	je     225f <__scif_connect+0x35f>
		if (unlikely((atomic_long_sub_return(cnt, 
    223e:	48 8d 99 88 01 00 00 	lea    0x188(%rcx),%rbx
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    2245:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    224c:	f0 48 0f c1 81 88 01 	lock xadd %rax,0x188(%rcx)
    2253:	00 00 
    2255:	48 83 e8 01          	sub    $0x1,%rax
    2259:	0f 88 0a 05 00 00    	js     2769 <__scif_connect+0x869>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;

	} else {
		pr_debug("SCIFAPI connect: ep %p connection interrupted\n", ep);
		err = -EINTR;
    225f:	bb fc ff ff ff       	mov    $0xfffffffc,%ebx
    2264:	e9 0c fd ff ff       	jmpq   1f75 <__scif_connect+0x75>
    2269:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	case SCIFEP_MAPPING:
		err = -EISCONN;
		goto connect_simple_unlock;
	case SCIFEP_UNBOUND:
		if ((ep->port.port = get_scif_port()) == 0) {
			err = -ENOSPC;
    2270:	bb e4 ff ff ff       	mov    $0xffffffe4,%ebx
    2275:	e9 1b fd ff ff       	jmpq   1f95 <__scif_connect+0x95>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;
	}

	if (ep->state == SCIFEP_MAPPING) {
		err = micscif_setup_qp_connect_response(ep->remote_dev,
    227a:	48 8b 53 20          	mov    0x20(%rbx),%rdx
    227e:	48 8b 73 10          	mov    0x10(%rbx),%rsi
    2282:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
    2289:	e8 00 00 00 00       	callq  228e <__scif_connect+0x38e>
    228e:	41 89 c4             	mov    %eax,%r12d
			ep->qp_info.qp, ep->qp_info.cnct_gnt_payload);
		ep->remote_ep = ep->qp_info.qp->remote_qp->ep;
    2291:	48 8b 43 10          	mov    0x10(%rbx),%rax

		// If the resource to map the queue are not available then we need
		// to tell the other side to terminate the accept
		if (err) {
    2295:	45 85 e4             	test   %r12d,%r12d
	}

	if (ep->state == SCIFEP_MAPPING) {
		err = micscif_setup_qp_connect_response(ep->remote_dev,
			ep->qp_info.qp, ep->qp_info.cnct_gnt_payload);
		ep->remote_ep = ep->qp_info.qp->remote_qp->ep;
    2298:	48 8b 80 80 00 00 00 	mov    0x80(%rax),%rax
    229f:	48 8b 10             	mov    (%rax),%rdx
    22a2:	48 89 93 50 01 00 00 	mov    %rdx,0x150(%rbx)

		// If the resource to map the queue are not available then we need
		// to tell the other side to terminate the accept
		if (err) {
    22a9:	0f 85 4a 07 00 00    	jne    29f9 <__scif_connect+0xaf9>
		}

		// Send a grant ack to inform the accept we are done mapping its resources.
		msg.uop = SCIF_CNCT_GNTACK;
		msg.payload[0] = ep->remote_ep;
		if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    22af:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
			goto connect_error_simple;
		}

		// Send a grant ack to inform the accept we are done mapping its resources.
		msg.uop = SCIF_CNCT_GNTACK;
		msg.payload[0] = ep->remote_ep;
    22b6:	48 89 55 b0          	mov    %rdx,-0x50(%rbp)
		if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    22ba:	4c 89 fe             	mov    %r15,%rsi
    22bd:	48 89 da             	mov    %rbx,%rdx
			micscif_dec_node_refcnt(ep->remote_dev, 1);
			goto connect_error_simple;
		}

		// Send a grant ack to inform the accept we are done mapping its resources.
		msg.uop = SCIF_CNCT_GNTACK;
    22c0:	c7 45 ac 07 00 00 00 	movl   $0x7,-0x54(%rbp)
		msg.payload[0] = ep->remote_ep;
		if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    22c7:	e8 00 00 00 00       	callq  22cc <__scif_connect+0x3cc>
    22cc:	85 c0                	test   %eax,%eax
    22ce:	41 89 c4             	mov    %eax,%r12d
    22d1:	0f 84 cc 04 00 00    	je     27a3 <__scif_connect+0x8a3>
			list_add_tail(&ep->list, &ms_info.mi_connected);
			get_conn_count(ep->remote_dev);
			spin_unlock_irqrestore(&ms_info.mi_connlock, sflags);
			pr_debug("SCIFAPI connect: ep %p connected\n", ep);
		} else
			ep->state = SCIFEP_BOUND;
    22d7:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    22dd:	48 8b 8b 48 01 00 00 	mov    0x148(%rbx),%rcx
		}

		// Send a grant ack to inform the accept we are done mapping its resources.
		msg.uop = SCIF_CNCT_GNTACK;
		msg.payload[0] = ep->remote_ep;
		if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    22e4:	44 89 e3             	mov    %r12d,%ebx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    22e7:	48 85 c9             	test   %rcx,%rcx
    22ea:	0f 84 85 fc ff ff    	je     1f75 <__scif_connect+0x75>
		if (unlikely((atomic_long_sub_return(cnt, 
    22f0:	4c 8d a1 88 01 00 00 	lea    0x188(%rcx),%r12
    22f7:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    22fe:	f0 48 0f c1 81 88 01 	lock xadd %rax,0x188(%rcx)
    2305:	00 00 
    2307:	48 83 e8 01          	sub    $0x1,%rax
    230b:	0f 89 64 fc ff ff    	jns    1f75 <__scif_connect+0x75>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    2311:	48 8b 45 08          	mov    0x8(%rbp),%rax
    2315:	ba a7 00 00 00       	mov    $0xa7,%edx
    231a:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2321:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    2328:	4c 8b 89 88 01 00 00 	mov    0x188(%rcx),%r9
    232f:	44 0f b7 01          	movzwl (%rcx),%r8d
    2333:	48 89 04 24          	mov    %rax,(%rsp)
    2337:	31 c0                	xor    %eax,%eax
    2339:	e8 00 00 00 00       	callq  233e <__scif_connect+0x43e>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    233e:	4c 89 e7             	mov    %r12,%rdi
    2341:	e8 1a e1 ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    2346:	e9 2a fc ff ff       	jmpq   1f75 <__scif_connect+0x75>
    234b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	if ((err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;
	}
	// Wait for request to be processed.
	while ((err = wait_event_interruptible_timeout(ep->conwq, 
    2350:	4c 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%r14
    2357:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    235e:	00 
    235f:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    2366:	00 00 00 00 
    236a:	49 8d 56 18          	lea    0x18(%r14),%rdx
    236e:	65 4c 8b 24 25 00 00 	mov    %gs:0x0,%r12
    2375:	00 00 
    2377:	48 89 55 90          	mov    %rdx,-0x70(%rbp)
    237b:	4c 89 65 80          	mov    %r12,-0x80(%rbp)
    237f:	48 89 55 98          	mov    %rdx,-0x68(%rbp)
    2383:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    238a:	ba 01 00 00 00       	mov    $0x1,%edx
    238f:	4c 89 f6             	mov    %r14,%rsi
    2392:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    2399:	e8 00 00 00 00       	callq  239e <__scif_connect+0x49e>
    239e:	8b 13                	mov    (%rbx),%edx
    23a0:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
    23a7:	83 fa 05             	cmp    $0x5,%edx
    23aa:	0f 85 51 04 00 00    	jne    2801 <__scif_connect+0x901>
	return test_and_clear_ti_thread_flag(task_thread_info(tsk), flag);
}

static inline int test_tsk_thread_flag(struct task_struct *tsk, int flag)
{
	return test_ti_thread_flag(task_thread_info(tsk), flag);
    23b0:	49 8b 54 24 08       	mov    0x8(%r12),%rdx
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    23b5:	48 8b 52 10          	mov    0x10(%rdx),%rdx
    23b9:	83 e2 04             	and    $0x4,%edx
    23bc:	0f 85 d6 02 00 00    	jne    2698 <__scif_connect+0x798>
    23c2:	48 89 c7             	mov    %rax,%rdi
    23c5:	e8 00 00 00 00       	callq  23ca <__scif_connect+0x4ca>
    23ca:	48 85 c0             	test   %rax,%rax
    23cd:	75 b4                	jne    2383 <__scif_connect+0x483>
    23cf:	45 31 e4             	xor    %r12d,%r12d
    23d2:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    23d9:	4c 89 f6             	mov    %r14,%rsi
    23dc:	e8 00 00 00 00       	callq  23e1 <__scif_connect+0x4e1>
    23e1:	e9 30 fe ff ff       	jmpq   2216 <__scif_connect+0x316>
    23e6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    23ed:	00 00 00 
		pr_debug("SCIFAPI connect: ep %p ^C detected\n", ep);
		// interrupted out of the wait
		if (!term_sent++) {
			int bak_err = err;
			msg.uop = SCIF_CNCT_TERM;
			if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    23f0:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
	}
	// Wait for request to be processed.
	while ((err = wait_event_interruptible_timeout(ep->conwq, 
		(ep->state != SCIFEP_CONNECTING), NODE_ALIVE_TIMEOUT)) <= 0) {
		if (!err)
			err = -ENODEV;
    23f7:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
		pr_debug("SCIFAPI connect: ep %p ^C detected\n", ep);
		// interrupted out of the wait
		if (!term_sent++) {
			int bak_err = err;
			msg.uop = SCIF_CNCT_TERM;
			if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    23fc:	48 89 da             	mov    %rbx,%rdx
    23ff:	4c 89 fe             	mov    %r15,%rsi
	}
	// Wait for request to be processed.
	while ((err = wait_event_interruptible_timeout(ep->conwq, 
		(ep->state != SCIFEP_CONNECTING), NODE_ALIVE_TIMEOUT)) <= 0) {
		if (!err)
			err = -ENODEV;
    2402:	44 0f 44 e0          	cmove  %eax,%r12d

		pr_debug("SCIFAPI connect: ep %p ^C detected\n", ep);
		// interrupted out of the wait
		if (!term_sent++) {
			int bak_err = err;
			msg.uop = SCIF_CNCT_TERM;
    2406:	c7 45 ac 0a 00 00 00 	movl   $0xa,-0x54(%rbp)
			if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
    240d:	e8 00 00 00 00       	callq  2412 <__scif_connect+0x512>
    2412:	85 c0                	test   %eax,%eax
    2414:	0f 85 3e 02 00 00    	jne    2658 <__scif_connect+0x758>
    241a:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    2421:	00 00 
    2423:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
retry:
				err = wait_event_timeout(ep->diswq, 
    242a:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    2431:	48 83 c0 18          	add    $0x18,%rax
    2435:	48 89 85 58 ff ff ff 	mov    %rax,-0xa8(%rbp)
    243c:	8b 0b                	mov    (%rbx),%ecx
    243e:	6b 15 00 00 00 00 64 	imul   $0x64,0x0(%rip),%edx        # 2445 <__scif_connect+0x545>
    2445:	83 f9 05             	cmp    $0x5,%ecx
    2448:	4c 63 f2             	movslq %edx,%r14
    244b:	0f 84 ef 00 00 00    	je     2540 <__scif_connect+0x640>
					(ep->state != SCIFEP_CONNECTING), NODE_ALIVE_TIMEOUT);
				if (!err && scifdev_alive(ep))
    2451:	85 d2                	test   %edx,%edx
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    2453:	4c 8b 93 48 01 00 00 	mov    0x148(%rbx),%r10
    245a:	75 10                	jne    246c <__scif_connect+0x56c>
    245c:	41 8b 42 04          	mov    0x4(%r10),%eax
    2460:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    2463:	83 f8 01             	cmp    $0x1,%eax
    2466:	0f 86 54 01 00 00    	jbe    25c0 <__scif_connect+0x6c0>
				if (!err)
					err = -ENODEV;
				if (err > 0)
					err = 0;
			}
			if (ep->state == SCIFEP_MAPPING) {
    246c:	8b 03                	mov    (%rbx),%eax
    246e:	83 f8 06             	cmp    $0x6,%eax
    2471:	0f 84 2c 02 00 00    	je     26a3 <__scif_connect+0x7a3>
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2477:	4d 85 d2             	test   %r10,%r10
				msg.payload[0] = ep->remote_ep;
				/* No error handling for Notification messages */
				micscif_nodeqp_send(ep->remote_dev, &msg, ep);
			}
			// Ensure after that even after a timeout the state of the end point is bound
			ep->state = SCIFEP_BOUND;
    247a:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
    2480:	0f 84 b0 00 00 00    	je     2536 <__scif_connect+0x636>
		if (unlikely((atomic_long_sub_return(cnt, 
    2486:	4d 8d aa 88 01 00 00 	lea    0x188(%r10),%r13
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    248d:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    2494:	f0 49 0f c1 82 88 01 	lock xadd %rax,0x188(%r10)
    249b:	00 00 
    249d:	48 83 e8 01          	sub    $0x1,%rax
    24a1:	44 89 e3             	mov    %r12d,%ebx
    24a4:	0f 89 cb fa ff ff    	jns    1f75 <__scif_connect+0x75>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    24aa:	48 8b 45 08          	mov    0x8(%rbp),%rax
    24ae:	4c 89 d1             	mov    %r10,%rcx
    24b1:	ba a7 00 00 00       	mov    $0xa7,%edx
    24b6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    24bd:	4d 8b 8a 88 01 00 00 	mov    0x188(%r10),%r9
    24c4:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    24cb:	4c 89 95 68 ff ff ff 	mov    %r10,-0x98(%rbp)
    24d2:	45 0f b7 02          	movzwl (%r10),%r8d
    24d6:	48 89 04 24          	mov    %rax,(%rsp)
    24da:	31 c0                	xor    %eax,%eax
    24dc:	e8 00 00 00 00       	callq  24e1 <__scif_connect+0x5e1>
    24e1:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    24e8:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    24ef:	00 00 80 
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    24f2:	49 8b 8a 88 01 00 00 	mov    0x188(%r10),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    24f9:	48 39 f1             	cmp    %rsi,%rcx
    24fc:	0f 84 73 fa ff ff    	je     1f75 <__scif_connect+0x75>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    2502:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2506:	48 89 c8             	mov    %rcx,%rax
    2509:	f0 49 0f b1 55 00    	lock cmpxchg %rdx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    250f:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2512:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    2515:	0f 84 5a fa ff ff    	je     1f75 <__scif_connect+0x75>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    251b:	48 39 f2             	cmp    %rsi,%rdx
    251e:	74 16                	je     2536 <__scif_connect+0x636>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    2520:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2524:	48 89 d0             	mov    %rdx,%rax
    2527:	f0 49 0f b1 4d 00    	lock cmpxchg %rcx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    252d:	48 39 c2             	cmp    %rax,%rdx
    2530:	0f 85 bb 04 00 00    	jne    29f1 <__scif_connect+0xaf1>
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2536:	44 89 e3             	mov    %r12d,%ebx
    2539:	e9 37 fa ff ff       	jmpq   1f75 <__scif_connect+0x75>
    253e:	66 90                	xchg   %ax,%ax
		if (!term_sent++) {
			int bak_err = err;
			msg.uop = SCIF_CNCT_TERM;
			if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
retry:
				err = wait_event_timeout(ep->diswq, 
    2540:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
    2547:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    254e:	00 
    254f:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    2556:	00 00 00 00 
    255a:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    255e:	48 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%rax
    2565:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    2569:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    256d:	eb 11                	jmp    2580 <__scif_connect+0x680>
    256f:	90                   	nop
    2570:	4c 89 f7             	mov    %r14,%rdi
    2573:	e8 00 00 00 00       	callq  2578 <__scif_connect+0x678>
    2578:	48 85 c0             	test   %rax,%rax
    257b:	49 89 c6             	mov    %rax,%r14
    257e:	74 58                	je     25d8 <__scif_connect+0x6d8>
    2580:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    2587:	ba 02 00 00 00       	mov    $0x2,%edx
    258c:	4c 89 ef             	mov    %r13,%rdi
    258f:	e8 00 00 00 00       	callq  2594 <__scif_connect+0x694>
    2594:	8b 13                	mov    (%rbx),%edx
    2596:	83 fa 05             	cmp    $0x5,%edx
    2599:	74 d5                	je     2570 <__scif_connect+0x670>
    259b:	44 89 f2             	mov    %r14d,%edx
    259e:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    25a5:	4c 89 ef             	mov    %r13,%rdi
    25a8:	89 95 68 ff ff ff    	mov    %edx,-0x98(%rbp)
    25ae:	e8 00 00 00 00       	callq  25b3 <__scif_connect+0x6b3>
    25b3:	8b 95 68 ff ff ff    	mov    -0x98(%rbp),%edx
    25b9:	e9 93 fe ff ff       	jmpq   2451 <__scif_connect+0x551>
    25be:	66 90                	xchg   %ax,%ax
    25c0:	83 bb 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rbx)
    25c7:	0f 85 9f fe ff ff    	jne    246c <__scif_connect+0x56c>
    25cd:	e9 6a fe ff ff       	jmpq   243c <__scif_connect+0x53c>
    25d2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    25d8:	31 d2                	xor    %edx,%edx
    25da:	eb c2                	jmp    259e <__scif_connect+0x69e>
    25dc:	0f 1f 40 00          	nopl   0x0(%rax)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    25e0:	49 8b 88 88 01 00 00 	mov    0x188(%r8),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    25e7:	4d 8d 88 88 01 00 00 	lea    0x188(%r8),%r9
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    25ee:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    25f5:	00 00 80 
    25f8:	48 39 f1             	cmp    %rsi,%rcx
    25fb:	0f 84 f0 00 00 00    	je     26f1 <__scif_connect+0x7f1>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    2601:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2605:	48 89 c8             	mov    %rcx,%rax
    2608:	f0 49 0f b1 90 88 01 	lock cmpxchg %rdx,0x188(%r8)
    260f:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    2611:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2614:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    2617:	75 0f                	jne    2628 <__scif_connect+0x728>
    2619:	4c 8b 83 48 01 00 00 	mov    0x148(%rbx),%r8
    2620:	e9 a0 fa ff ff       	jmpq   20c5 <__scif_connect+0x1c5>
    2625:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    2628:	48 39 f2             	cmp    %rsi,%rdx
    262b:	0f 84 c0 00 00 00    	je     26f1 <__scif_connect+0x7f1>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    2631:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    2635:	48 89 d0             	mov    %rdx,%rax
    2638:	f0 49 0f b1 09       	lock cmpxchg %rcx,(%r9)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    263d:	48 39 c2             	cmp    %rax,%rdx
    2640:	75 e3                	jne    2625 <__scif_connect+0x725>
    2642:	4c 8b 83 48 01 00 00 	mov    0x148(%rbx),%r8
    2649:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
    264d:	e9 73 fa ff ff       	jmpq   20c5 <__scif_connect+0x1c5>
    2652:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    2658:	4c 8b 93 48 01 00 00 	mov    0x148(%rbx),%r10
    265f:	e9 08 fe ff ff       	jmpq   246c <__scif_connect+0x56c>
    2664:	0f 1f 40 00          	nopl   0x0(%rax)
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2668:	48 85 c9             	test   %rcx,%rcx
    266b:	74 21                	je     268e <__scif_connect+0x78e>
		if (unlikely((atomic_long_sub_return(cnt, 
    266d:	48 8d 99 88 01 00 00 	lea    0x188(%rcx),%rbx
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    2674:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    267b:	f0 48 0f c1 81 88 01 	lock xadd %rax,0x188(%rcx)
    2682:	00 00 
    2684:	48 83 e8 01          	sub    $0x1,%rax
    2688:	0f 88 84 01 00 00    	js     2812 <__scif_connect+0x912>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		goto connect_error_simple;

	} else if (ep->state == SCIFEP_BOUND) {
		pr_debug("SCIFAPI connect: ep %p connection refused\n", ep);
		err = -ECONNREFUSED;
    268e:	bb 91 ff ff ff       	mov    $0xffffff91,%ebx
    2693:	e9 dd f8 ff ff       	jmpq   1f75 <__scif_connect+0x75>
    2698:	41 bc 00 fe ff ff    	mov    $0xfffffe00,%r12d
    269e:	e9 2f fd ff ff       	jmpq   23d2 <__scif_connect+0x4d2>
					err = -ENODEV;
				if (err > 0)
					err = 0;
			}
			if (ep->state == SCIFEP_MAPPING) {
				micscif_setup_qp_connect_response(ep->remote_dev,
    26a3:	48 8b 53 20          	mov    0x20(%rbx),%rdx
    26a7:	4c 89 d7             	mov    %r10,%rdi
    26aa:	48 8b 73 10          	mov    0x10(%rbx),%rsi
    26ae:	e8 00 00 00 00       	callq  26b3 <__scif_connect+0x7b3>
					ep->qp_info.qp, ep->qp_info.cnct_gnt_payload);
				ep->remote_ep = ep->qp_info.qp->remote_qp->ep;
    26b3:	48 8b 43 10          	mov    0x10(%rbx),%rax
				// Send grant nack
				msg.uop = SCIF_CNCT_GNTNACK;
				msg.payload[0] = ep->remote_ep;
				/* No error handling for Notification messages */
				micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    26b7:	48 89 da             	mov    %rbx,%rdx
    26ba:	4c 89 fe             	mov    %r15,%rsi
    26bd:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
					err = 0;
			}
			if (ep->state == SCIFEP_MAPPING) {
				micscif_setup_qp_connect_response(ep->remote_dev,
					ep->qp_info.qp, ep->qp_info.cnct_gnt_payload);
				ep->remote_ep = ep->qp_info.qp->remote_qp->ep;
    26c4:	48 8b 80 80 00 00 00 	mov    0x80(%rax),%rax
    26cb:	48 8b 00             	mov    (%rax),%rax
				// Send grant nack
				msg.uop = SCIF_CNCT_GNTNACK;
    26ce:	c7 45 ac 08 00 00 00 	movl   $0x8,-0x54(%rbp)
					err = 0;
			}
			if (ep->state == SCIFEP_MAPPING) {
				micscif_setup_qp_connect_response(ep->remote_dev,
					ep->qp_info.qp, ep->qp_info.cnct_gnt_payload);
				ep->remote_ep = ep->qp_info.qp->remote_qp->ep;
    26d5:	48 89 83 50 01 00 00 	mov    %rax,0x150(%rbx)
				// Send grant nack
				msg.uop = SCIF_CNCT_GNTNACK;
				msg.payload[0] = ep->remote_ep;
    26dc:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
				/* No error handling for Notification messages */
				micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    26e0:	e8 00 00 00 00       	callq  26e5 <__scif_connect+0x7e5>
    26e5:	4c 8b 93 48 01 00 00 	mov    0x148(%rbx),%r10
    26ec:	e9 86 fd ff ff       	jmpq   2477 <__scif_connect+0x577>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    26f1:	49 8d 80 68 01 00 00 	lea    0x168(%r8),%rax
    26f8:	4c 89 8d 58 ff ff ff 	mov    %r9,-0xa8(%rbp)
    26ff:	48 89 c7             	mov    %rax,%rdi
    2702:	4c 89 85 60 ff ff ff 	mov    %r8,-0xa0(%rbp)
    2709:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    2710:	e8 00 00 00 00       	callq  2715 <__scif_connect+0x815>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    2715:	4c 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%r8
    271c:	4c 8b 8d 58 ff ff ff 	mov    -0xa8(%rbp),%r9
    2723:	41 8b 40 04          	mov    0x4(%r8),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    2727:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    272a:	83 fa 01             	cmp    $0x1,%edx
    272d:	0f 86 d6 00 00 00    	jbe    2809 <__scif_connect+0x909>
    2733:	83 f8 01             	cmp    $0x1,%eax
    2736:	0f 84 cd 00 00 00    	je     2809 <__scif_connect+0x909>
    273c:	49 8b 80 88 01 00 00 	mov    0x188(%r8),%rax
    2743:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    2747:	48 85 c0             	test   %rax,%rax
    274a:	0f 88 fc 00 00 00    	js     284c <__scif_connect+0x94c>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    2750:	4c 89 cf             	mov    %r9,%rdi
    2753:	e8 98 dd ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    2758:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    275f:	e8 00 00 00 00       	callq  2764 <__scif_connect+0x864>
    2764:	e9 b0 fe ff ff       	jmpq   2619 <__scif_connect+0x719>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    2769:	48 8b 45 08          	mov    0x8(%rbp),%rax
    276d:	ba a7 00 00 00       	mov    $0xa7,%edx
    2772:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2779:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    2780:	4c 8b 89 88 01 00 00 	mov    0x188(%rcx),%r9
    2787:	44 0f b7 01          	movzwl (%rcx),%r8d
    278b:	48 89 04 24          	mov    %rax,(%rsp)
    278f:	31 c0                	xor    %eax,%eax
    2791:	e8 00 00 00 00       	callq  2796 <__scif_connect+0x896>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    2796:	48 89 df             	mov    %rbx,%rdi
    2799:	e8 c2 dc ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    279e:	e9 bc fa ff ff       	jmpq   225f <__scif_connect+0x35f>

		// Send a grant ack to inform the accept we are done mapping its resources.
		msg.uop = SCIF_CNCT_GNTACK;
		msg.payload[0] = ep->remote_ep;
		if (!(err = micscif_nodeqp_send(ep->remote_dev, &msg, ep))) {
			ep->state = SCIFEP_CONNECTED;
    27a3:	c7 03 04 00 00 00    	movl   $0x4,(%rbx)
			spin_lock_irqsave(&ms_info.mi_connlock, sflags);
    27a9:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    27b0:	e8 00 00 00 00       	callq  27b5 <__scif_connect+0x8b5>
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
    27b5:	48 8b 15 00 00 00 00 	mov    0x0(%rip),%rdx        # 27bc <__scif_connect+0x8bc>
			list_add_tail(&ep->list, &ms_info.mi_connected);
    27bc:	48 8d 8b 40 02 00 00 	lea    0x240(%rbx),%rcx
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
    27c3:	48 c7 83 40 02 00 00 	movq   $0x0,0x240(%rbx)
    27ca:	00 00 00 00 
#ifndef CONFIG_DEBUG_LIST
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
    27ce:	48 89 0d 00 00 00 00 	mov    %rcx,0x0(%rip)        # 27d5 <__scif_connect+0x8d5>
    27d5:	48 89 c6             	mov    %rax,%rsi
    27d8:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
	new->next = next;
	new->prev = prev;
    27df:	48 89 93 48 02 00 00 	mov    %rdx,0x248(%rbx)
	prev->next = new;
    27e6:	48 89 0a             	mov    %rcx,(%rdx)
			get_conn_count(ep->remote_dev);
    27e9:	48 8b 93 48 01 00 00 	mov    0x148(%rbx),%rdx
 * to synchronize calling this API with put_conn_count.
 */
static __always_inline void
get_conn_count(struct micscif_dev *dev)
{
	dev->num_active_conn++;
    27f0:	83 82 dc 01 00 00 01 	addl   $0x1,0x1dc(%rdx)
    27f7:	e8 00 00 00 00       	callq  27fc <__scif_connect+0x8fc>
    27fc:	e9 dc fa ff ff       	jmpq   22dd <__scif_connect+0x3dd>
    2801:	41 89 c4             	mov    %eax,%r12d
    2804:	e9 c9 fb ff ff       	jmpq   23d2 <__scif_connect+0x4d2>
    2809:	4c 8d 7d a4          	lea    -0x5c(%rbp),%r15
    280d:	e9 46 ff ff ff       	jmpq   2758 <__scif_connect+0x858>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    2812:	48 8b 45 08          	mov    0x8(%rbp),%rax
    2816:	ba a7 00 00 00       	mov    $0xa7,%edx
    281b:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2822:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    2829:	4c 8b 89 88 01 00 00 	mov    0x188(%rcx),%r9
    2830:	44 0f b7 01          	movzwl (%rcx),%r8d
    2834:	48 89 04 24          	mov    %rax,(%rsp)
    2838:	31 c0                	xor    %eax,%eax
    283a:	e8 00 00 00 00       	callq  283f <__scif_connect+0x93f>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    283f:	48 89 df             	mov    %rbx,%rdi
    2842:	e8 19 dc ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    2847:	e9 42 fe ff ff       	jmpq   268e <__scif_connect+0x78e>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    284c:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 2852 <__scif_connect+0x952>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    2852:	31 d2                	xor    %edx,%edx
    2854:	4c 89 fe             	mov    %r15,%rsi
    2857:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    285e:	49 c7 80 b0 01 00 00 	movq   $0x2,0x1b0(%r8)
    2865:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    2869:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    2870:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    2876:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    287a:	41 0f b7 00          	movzwl (%r8),%eax
    287e:	4c 89 8d 58 ff ff ff 	mov    %r9,-0xa8(%rbp)
    2885:	4c 89 85 60 ff ff ff 	mov    %r8,-0xa0(%rbp)
    288c:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    2890:	e8 00 00 00 00       	callq  2895 <__scif_connect+0x995>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    2895:	4c 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%r8
    289c:	4c 8b 8d 58 ff ff ff 	mov    -0xa8(%rbp),%r9
    28a3:	49 83 b8 b0 01 00 00 	cmpq   $0x2,0x1b0(%r8)
    28aa:	02 
    28ab:	74 2c                	je     28d9 <__scif_connect+0x9d9>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    28ad:	49 83 b8 b0 01 00 00 	cmpq   $0x3,0x1b0(%r8)
    28b4:	03 
    28b5:	0f 85 95 fe ff ff    	jne    2750 <__scif_connect+0x850>
				dev->sd_state = SCIFDEV_RUNNING;
    28bb:	41 c7 40 04 02 00 00 	movl   $0x2,0x4(%r8)
    28c2:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    28c3:	f0 41 80 a0 8f 01 00 	lock andb $0x7f,0x18f(%r8)
    28ca:	00 7f 
    28cc:	e9 7f fe ff ff       	jmpq   2750 <__scif_connect+0x850>
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    28d1:	48 89 c2             	mov    %rax,%rdx
    28d4:	e9 d5 f8 ff ff       	jmpq   21ae <__scif_connect+0x2ae>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    28d9:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    28e0:	31 c0                	xor    %eax,%eax
    28e2:	b9 0a 00 00 00       	mov    $0xa,%ecx
    28e7:	f3 ab                	rep stos %eax,%es:(%rdi)
    28e9:	4c 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%r14
    28f0:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    28f7:	00 
    28f8:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    28ff:	00 00 
    2901:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    2905:	49 8d 46 18          	lea    0x18(%r14),%rax
    2909:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    290d:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    2911:	49 8d 80 98 01 00 00 	lea    0x198(%r8),%rax
    2918:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    291f:	eb 13                	jmp    2934 <__scif_connect+0xa34>
    2921:	e8 00 00 00 00       	callq  2926 <__scif_connect+0xa26>
    2926:	4c 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%r8
    292d:	4c 8b 8d 48 ff ff ff 	mov    -0xb8(%rbp),%r9
    2934:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    293b:	ba 02 00 00 00       	mov    $0x2,%edx
    2940:	4c 89 f6             	mov    %r14,%rsi
    2943:	4c 89 8d 48 ff ff ff 	mov    %r9,-0xb8(%rbp)
    294a:	4c 89 85 58 ff ff ff 	mov    %r8,-0xa8(%rbp)
    2951:	e8 00 00 00 00       	callq  2956 <__scif_connect+0xa56>
    2956:	4c 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%r8
    295d:	4c 8b 8d 48 ff ff ff 	mov    -0xb8(%rbp),%r9
    2964:	49 83 b8 b0 01 00 00 	cmpq   $0x2,0x1b0(%r8)
    296b:	02 
    296c:	74 b3                	je     2921 <__scif_connect+0xa21>
    296e:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    2975:	4c 89 f6             	mov    %r14,%rsi
    2978:	4c 89 8d 48 ff ff ff 	mov    %r9,-0xb8(%rbp)
    297f:	4c 89 85 58 ff ff ff 	mov    %r8,-0xa8(%rbp)
    2986:	e8 00 00 00 00       	callq  298b <__scif_connect+0xa8b>
    298b:	4c 8b 8d 48 ff ff ff 	mov    -0xb8(%rbp),%r9
    2992:	4c 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%r8
    2999:	e9 0f ff ff ff       	jmpq   28ad <__scif_connect+0x9ad>
	}
	// Initiate the first part of the endpoint QP setup
	err = micscif_setup_qp_connect(ep->qp_info.qp, &ep->qp_info.qp_offset,
			ENDPT_QP_SIZE, ep->remote_dev);
	if (err) {
		printk(KERN_ERR "%s err %d qp_offset 0x%llx\n", 
    299e:	48 8b 4b 18          	mov    0x18(%rbx),%rcx
    29a2:	89 c2                	mov    %eax,%edx
    29a4:	31 c0                	xor    %eax,%eax
    29a6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    29ad:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    29b4:	e8 00 00 00 00       	callq  29b9 <__scif_connect+0xab9>
			__func__, err, ep->qp_info.qp_offset);
		ep->state = SCIFEP_BOUND;
    29b9:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
		goto connect_error_simple;
    29bf:	44 89 fb             	mov    %r15d,%ebx
    29c2:	e9 ae f5 ff ff       	jmpq   1f75 <__scif_connect+0x75>
	ep->qp_info.qp->magic = SCIFEP_MAGIC;
	ep->qp_info.qp->ep = (uint64_t)ep;
	spin_unlock_irqrestore(&ep->lock, sflags);

	if ((err = micscif_reserve_dma_chan(ep))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    29c7:	89 c1                	mov    %eax,%ecx
    29c9:	ba 06 03 00 00       	mov    $0x306,%edx
    29ce:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    29d5:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    29dc:	31 c0                	xor    %eax,%eax
    29de:	e8 00 00 00 00       	callq  29e3 <__scif_connect+0xae3>
		ep->state = SCIFEP_BOUND;
    29e3:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
		goto connect_error_simple;
    29e9:	44 89 fb             	mov    %r15d,%ebx
    29ec:	e9 84 f5 ff ff       	jmpq   1f75 <__scif_connect+0x75>
    29f1:	48 89 c2             	mov    %rax,%rdx
    29f4:	e9 22 fb ff ff       	jmpq   251b <__scif_connect+0x61b>
		ep->remote_ep = ep->qp_info.qp->remote_qp->ep;

		// If the resource to map the queue are not available then we need
		// to tell the other side to terminate the accept
		if (err) {
			printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    29f9:	44 89 e1             	mov    %r12d,%ecx
    29fc:	ba 58 03 00 00       	mov    $0x358,%edx
    2a01:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2a08:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    2a0f:	31 c0                	xor    %eax,%eax
    2a11:	e8 00 00 00 00       	callq  2a16 <__scif_connect+0xb16>

			// Send grant nack
			msg.uop = SCIF_CNCT_GNTNACK;
			msg.payload[0] = ep->remote_ep;
    2a16:	48 8b 83 50 01 00 00 	mov    0x150(%rbx),%rax
			/* No error handling for Notification messages */
			micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    2a1d:	48 89 da             	mov    %rbx,%rdx
    2a20:	4c 89 fe             	mov    %r15,%rsi
    2a23:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
		// to tell the other side to terminate the accept
		if (err) {
			printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);

			// Send grant nack
			msg.uop = SCIF_CNCT_GNTNACK;
    2a2a:	c7 45 ac 08 00 00 00 	movl   $0x8,-0x54(%rbp)
			msg.payload[0] = ep->remote_ep;
    2a31:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Notification messages */
			micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    2a35:	e8 00 00 00 00       	callq  2a3a <__scif_connect+0xb3a>

			ep->state = SCIFEP_BOUND;
			micscif_dec_node_refcnt(ep->remote_dev, 1);
    2a3a:	4c 8b ab 48 01 00 00 	mov    0x148(%rbx),%r13
			msg.uop = SCIF_CNCT_GNTNACK;
			msg.payload[0] = ep->remote_ep;
			/* No error handling for Notification messages */
			micscif_nodeqp_send(ep->remote_dev, &msg, ep);

			ep->state = SCIFEP_BOUND;
    2a41:	c7 03 02 00 00 00    	movl   $0x2,(%rbx)
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2a47:	4d 85 ed             	test   %r13,%r13
    2a4a:	0f 84 e6 fa ff ff    	je     2536 <__scif_connect+0x636>
		if (unlikely((atomic_long_sub_return(cnt, 
    2a50:	4d 8d b5 88 01 00 00 	lea    0x188(%r13),%r14
    2a57:	44 89 e3             	mov    %r12d,%ebx
    2a5a:	4c 89 f7             	mov    %r14,%rdi
    2a5d:	e8 6e da ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    2a62:	48 85 c0             	test   %rax,%rax
    2a65:	0f 89 0a f5 ff ff    	jns    1f75 <__scif_connect+0x75>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    2a6b:	48 8b 45 08          	mov    0x8(%rbp),%rax
    2a6f:	4c 89 e9             	mov    %r13,%rcx
    2a72:	ba a7 00 00 00       	mov    $0xa7,%edx
    2a77:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    2a7e:	4d 8b 8d 88 01 00 00 	mov    0x188(%r13),%r9
    2a85:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    2a8c:	45 0f b7 45 00       	movzwl 0x0(%r13),%r8d
    2a91:	48 89 04 24          	mov    %rax,(%rsp)
    2a95:	31 c0                	xor    %eax,%eax
    2a97:	e8 00 00 00 00       	callq  2a9c <__scif_connect+0xb9c>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    2a9c:	4c 89 f7             	mov    %r14,%rdi
    2a9f:	e8 bc d9 ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    2aa4:	e9 cc f4 ff ff       	jmpq   1f75 <__scif_connect+0x75>
    2aa9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000002ab0 <scif_connect>:
	return err;
}

int
scif_connect(scif_epd_t epd, struct scif_portID *dst)
{
    2ab0:	55                   	push   %rbp
    2ab1:	48 89 e5             	mov    %rsp,%rbp
    2ab4:	41 55                	push   %r13
    2ab6:	41 54                	push   %r12
    2ab8:	53                   	push   %rbx
    2ab9:	48 83 ec 08          	sub    $0x8,%rsp
    2abd:	e8 00 00 00 00       	callq  2ac2 <scif_connect+0x12>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    2ac2:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    2ac9:	49 89 fc             	mov    %rdi,%r12
    2acc:	49 89 f5             	mov    %rsi,%r13
    2acf:	48 89 df             	mov    %rbx,%rdi
    2ad2:	e8 00 00 00 00       	callq  2ad7 <scif_connect+0x27>
	int ret;
	get_kref_count(epd);
	ret = __scif_connect(epd, dst);
    2ad7:	4c 89 ee             	mov    %r13,%rsi
    2ada:	4c 89 e7             	mov    %r12,%rdi
    2add:	e8 00 00 00 00       	callq  2ae2 <scif_connect+0x32>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    2ae2:	48 89 df             	mov    %rbx,%rdi
    2ae5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2aec:	41 89 c4             	mov    %eax,%r12d
    2aef:	e8 00 00 00 00       	callq  2af4 <scif_connect+0x44>
	put_kref_count(epd);
	return ret;
}
    2af4:	48 83 c4 08          	add    $0x8,%rsp
    2af8:	44 89 e0             	mov    %r12d,%eax
    2afb:	5b                   	pop    %rbx
    2afc:	41 5c                	pop    %r12
    2afe:	41 5d                	pop    %r13
    2b00:	5d                   	pop    %rbp
    2b01:	c3                   	retq   
    2b02:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    2b09:	1f 84 00 00 00 00 00 

0000000000002b10 <__scif_accept>:
 * If the remote side is not sending any connection requests the caller may
 * terminate this funciton with a signal.  If so a -EINTR will be returned.
 */
int
__scif_accept(scif_epd_t epd, struct scif_portID *peer, scif_epd_t *newepd, int flags)
{
    2b10:	55                   	push   %rbp
    2b11:	48 89 e5             	mov    %rsp,%rbp
    2b14:	41 57                	push   %r15
    2b16:	41 56                	push   %r14
    2b18:	41 55                	push   %r13
    2b1a:	41 54                	push   %r12
    2b1c:	53                   	push   %rbx
    2b1d:	48 81 ec 18 01 00 00 	sub    $0x118,%rsp
    2b24:	e8 00 00 00 00       	callq  2b29 <__scif_accept+0x19>
	struct conreq *conreq;
	struct nodemsg msg;
	unsigned long sflags;
	int err;

	pr_debug("SCIFAPI accept: ep %p %s\n", lep, scif_ep_states[lep->state]);
    2b29:	8b 07                	mov    (%rdi),%eax

	// Error if flags other than SCIF_ACCEPT_SYNC are set
	if (flags & ~SCIF_ACCEPT_SYNC) {
    2b2b:	89 c8                	mov    %ecx,%eax
    2b2d:	83 e0 fe             	and    $0xfffffffe,%eax
    2b30:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    2b36:	0f 85 35 07 00 00    	jne    3271 <__scif_accept+0x761>
		pr_debug("SCIFAPI accept: ep %p invalid flags %x\n", lep, flags & ~SCIF_ACCEPT_SYNC);
		return -EINVAL;
	}

	if (!peer || !newepd) {
    2b3c:	48 85 f6             	test   %rsi,%rsi
    2b3f:	0f 84 2c 07 00 00    	je     3271 <__scif_accept+0x761>
    2b45:	48 85 d2             	test   %rdx,%rdx
    2b48:	0f 84 23 07 00 00    	je     3271 <__scif_accept+0x761>
    2b4e:	48 89 fb             	mov    %rdi,%rbx
    2b51:	48 89 b5 10 ff ff ff 	mov    %rsi,-0xf0(%rbp)
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    2b58:	4c 8d 7b 04          	lea    0x4(%rbx),%r15
    2b5c:	89 8d 0c ff ff ff    	mov    %ecx,-0xf4(%rbp)
    2b62:	48 89 95 f0 fe ff ff 	mov    %rdx,-0x110(%rbp)
		pr_debug("SCIFAPI accept: ep %p peer %p or newepd %p NULL\n", 
			lep, peer, newepd);
		return -EINVAL;
	}

	might_sleep();
    2b69:	e8 00 00 00 00       	callq  2b6e <__scif_accept+0x5e>
	spin_lock_irqsave(&lep->lock, sflags);
    2b6e:	4c 89 ff             	mov    %r15,%rdi
    2b71:	e8 00 00 00 00       	callq  2b76 <__scif_accept+0x66>
    2b76:	48 89 c6             	mov    %rax,%rsi
	if (lep->state != SCIFEP_LISTENING) {
    2b79:	8b 03                	mov    (%rbx),%eax
    2b7b:	83 f8 03             	cmp    $0x3,%eax
    2b7e:	0f 85 54 07 00 00    	jne    32d8 <__scif_accept+0x7c8>
		pr_debug("SCIFAPI accept: ep %p not listending\n", lep);
		spin_unlock_irqrestore(&lep->lock, sflags);
		return -EINVAL;
	}

	if (!lep->conreqcnt && !(flags & SCIF_ACCEPT_SYNC)) {
    2b84:	44 8b 9b 58 01 00 00 	mov    0x158(%rbx),%r11d
    2b8b:	45 85 db             	test   %r11d,%r11d
    2b8e:	0f 84 4c 01 00 00    	je     2ce0 <__scif_accept+0x1d0>
	}

retry_connection:
	spin_unlock_irqrestore(&lep->lock, sflags);
	lep->files = current ? current->files : NULL;
	if ((err = wait_event_interruptible(lep->conwq, 
    2b94:	48 8d 45 a0          	lea    -0x60(%rbp),%rax
    2b98:	48 83 c0 18          	add    $0x18,%rax
    2b9c:	65 4c 8b 2c 25 00 00 	mov    %gs:0x0,%r13
    2ba3:	00 00 
    2ba5:	48 89 85 38 ff ff ff 	mov    %rax,-0xc8(%rbp)
    2bac:	4c 89 ad 18 ff ff ff 	mov    %r13,-0xe8(%rbp)
    2bb3:	eb 48                	jmp    2bfd <__scif_accept+0xed>
    2bb5:	0f 1f 00             	nopl   (%rax)
		return -EAGAIN;
	}

retry_connection:
	spin_unlock_irqrestore(&lep->lock, sflags);
	lep->files = current ? current->files : NULL;
    2bb8:	49 8b 85 28 05 00 00 	mov    0x528(%r13),%rax
	if ((err = wait_event_interruptible(lep->conwq, 
    2bbf:	44 8b 8b 58 01 00 00 	mov    0x158(%rbx),%r9d
		return -EAGAIN;
	}

retry_connection:
	spin_unlock_irqrestore(&lep->lock, sflags);
	lep->files = current ? current->files : NULL;
    2bc6:	48 89 83 68 01 00 00 	mov    %rax,0x168(%rbx)
	if ((err = wait_event_interruptible(lep->conwq, 
    2bcd:	45 85 c9             	test   %r9d,%r9d
    2bd0:	75 07                	jne    2bd9 <__scif_accept+0xc9>
    2bd2:	8b 03                	mov    (%rbx),%eax
    2bd4:	83 f8 03             	cmp    $0x3,%eax
    2bd7:	74 37                	je     2c10 <__scif_accept+0x100>
		// wait was interrupted
		pr_debug("SCIFAPI accept: ep %p ^C detected\n", lep);
		return err;	// -ERESTARTSYS
	}

	if (lep->state != SCIFEP_LISTENING) {
    2bd9:	8b 03                	mov    (%rbx),%eax
    2bdb:	83 f8 03             	cmp    $0x3,%eax
    2bde:	0f 85 ec 00 00 00    	jne    2cd0 <__scif_accept+0x1c0>
		return -EINTR;
	}

	spin_lock_irqsave(&lep->lock, sflags);
    2be4:	4c 89 ff             	mov    %r15,%rdi
    2be7:	e8 00 00 00 00       	callq  2bec <__scif_accept+0xdc>

	if (!lep->conreqcnt) {
    2bec:	8b bb 58 01 00 00    	mov    0x158(%rbx),%edi

	if (lep->state != SCIFEP_LISTENING) {
		return -EINTR;
	}

	spin_lock_irqsave(&lep->lock, sflags);
    2bf2:	48 89 c6             	mov    %rax,%rsi

	if (!lep->conreqcnt) {
    2bf5:	85 ff                	test   %edi,%edi
    2bf7:	0f 85 0b 01 00 00    	jne    2d08 <__scif_accept+0x1f8>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    2bfd:	4c 89 ff             	mov    %r15,%rdi
    2c00:	e8 00 00 00 00       	callq  2c05 <__scif_accept+0xf5>
		return -EAGAIN;
	}

retry_connection:
	spin_unlock_irqrestore(&lep->lock, sflags);
	lep->files = current ? current->files : NULL;
    2c05:	4d 85 ed             	test   %r13,%r13
    2c08:	75 ae                	jne    2bb8 <__scif_accept+0xa8>
    2c0a:	31 c0                	xor    %eax,%eax
    2c0c:	eb b1                	jmp    2bbf <__scif_accept+0xaf>
    2c0e:	66 90                	xchg   %ax,%ax
	if ((err = wait_event_interruptible(lep->conwq, 
    2c10:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    2c17:	48 c7 45 a0 00 00 00 	movq   $0x0,-0x60(%rbp)
    2c1e:	00 
    2c1f:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
    2c26:	48 c7 45 b0 00 00 00 	movq   $0x0,-0x50(%rbp)
    2c2d:	00 
    2c2e:	65 4c 8b 34 25 00 00 	mov    %gs:0x0,%r14
    2c35:	00 00 
    2c37:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    2c3b:	48 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%rax
    2c42:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    2c46:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    2c4a:	eb 09                	jmp    2c55 <__scif_accept+0x145>
    2c4c:	0f 1f 40 00          	nopl   0x0(%rax)
    2c50:	e8 00 00 00 00       	callq  2c55 <__scif_accept+0x145>
    2c55:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    2c59:	ba 01 00 00 00       	mov    $0x1,%edx
    2c5e:	4c 89 e7             	mov    %r12,%rdi
    2c61:	e8 00 00 00 00       	callq  2c66 <__scif_accept+0x156>
    2c66:	44 8b 83 58 01 00 00 	mov    0x158(%rbx),%r8d
    2c6d:	45 85 c0             	test   %r8d,%r8d
    2c70:	75 46                	jne    2cb8 <__scif_accept+0x1a8>
    2c72:	8b 03                	mov    (%rbx),%eax
    2c74:	83 f8 03             	cmp    $0x3,%eax
    2c77:	75 3f                	jne    2cb8 <__scif_accept+0x1a8>
    2c79:	49 8b 46 08          	mov    0x8(%r14),%rax
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    2c7d:	48 8b 40 10          	mov    0x10(%rax),%rax
    2c81:	a8 04                	test   $0x4,%al
    2c83:	74 cb                	je     2c50 <__scif_accept+0x140>
    2c85:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    2c89:	4c 89 e7             	mov    %r12,%rdi
    2c8c:	e8 00 00 00 00       	callq  2c91 <__scif_accept+0x181>
    2c91:	c7 85 30 ff ff ff 00 	movl   $0xfffffe00,-0xd0(%rbp)
    2c98:	fe ff ff 
	micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, NULL);
	micscif_dec_node_refcnt(&scif_dev[conreq->msg.src.node], 1);

	kfree(conreq);
	return err;
}
    2c9b:	8b 85 30 ff ff ff    	mov    -0xd0(%rbp),%eax
    2ca1:	48 81 c4 18 01 00 00 	add    $0x118,%rsp
    2ca8:	5b                   	pop    %rbx
    2ca9:	41 5c                	pop    %r12
    2cab:	41 5d                	pop    %r13
    2cad:	41 5e                	pop    %r14
    2caf:	41 5f                	pop    %r15
    2cb1:	5d                   	pop    %rbp
    2cb2:	c3                   	retq   
    2cb3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	}

retry_connection:
	spin_unlock_irqrestore(&lep->lock, sflags);
	lep->files = current ? current->files : NULL;
	if ((err = wait_event_interruptible(lep->conwq, 
    2cb8:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    2cbc:	4c 89 e7             	mov    %r12,%rdi
    2cbf:	e8 00 00 00 00       	callq  2cc4 <__scif_accept+0x1b4>
		// wait was interrupted
		pr_debug("SCIFAPI accept: ep %p ^C detected\n", lep);
		return err;	// -ERESTARTSYS
	}

	if (lep->state != SCIFEP_LISTENING) {
    2cc4:	8b 03                	mov    (%rbx),%eax
    2cc6:	83 f8 03             	cmp    $0x3,%eax
    2cc9:	0f 84 15 ff ff ff    	je     2be4 <__scif_accept+0xd4>
    2ccf:	90                   	nop
		return -EINTR;
    2cd0:	c7 85 30 ff ff ff fc 	movl   $0xfffffffc,-0xd0(%rbp)
    2cd7:	ff ff ff 
    2cda:	eb bf                	jmp    2c9b <__scif_accept+0x18b>
    2cdc:	0f 1f 40 00          	nopl   0x0(%rax)
		pr_debug("SCIFAPI accept: ep %p not listending\n", lep);
		spin_unlock_irqrestore(&lep->lock, sflags);
		return -EINVAL;
	}

	if (!lep->conreqcnt && !(flags & SCIF_ACCEPT_SYNC)) {
    2ce0:	44 8b 95 0c ff ff ff 	mov    -0xf4(%rbp),%r10d
    2ce7:	45 85 d2             	test   %r10d,%r10d
    2cea:	0f 85 a4 fe ff ff    	jne    2b94 <__scif_accept+0x84>
    2cf0:	4c 89 ff             	mov    %r15,%rdi
    2cf3:	e8 00 00 00 00       	callq  2cf8 <__scif_accept+0x1e8>
		// No connection request present and we do not want to wait
		pr_debug("SCIFAPI accept: ep %p async request with nothing pending\n", lep);
		spin_unlock_irqrestore(&lep->lock, sflags);
		return -EAGAIN;
    2cf8:	c7 85 30 ff ff ff f5 	movl   $0xfffffff5,-0xd0(%rbp)
    2cff:	ff ff ff 
    2d02:	eb 97                	jmp    2c9b <__scif_accept+0x18b>
    2d04:	0f 1f 40 00          	nopl   0x0(%rax)
	if (!lep->conreqcnt) {
		goto retry_connection;
	}

	// Get the first connect request off the list
	conreq = list_first_entry(&lep->conlist, struct conreq, list);
    2d08:	48 8b 83 78 01 00 00 	mov    0x178(%rbx),%rax
    2d0f:	4c 89 ff             	mov    %r15,%rdi
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    2d12:	48 8b 10             	mov    (%rax),%rdx
    2d15:	48 8d 48 d0          	lea    -0x30(%rax),%rcx
    2d19:	49 89 c6             	mov    %rax,%r14
    2d1c:	48 89 85 f8 fe ff ff 	mov    %rax,-0x108(%rbp)
    2d23:	48 8b 40 08          	mov    0x8(%rax),%rax
    2d27:	48 89 8d 28 ff ff ff 	mov    %rcx,-0xd8(%rbp)
	entry->next = LIST_POISON1;
    2d2e:	48 b9 00 01 10 00 00 	movabs $0xdead000000100100,%rcx
    2d35:	00 ad de 
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    2d38:	48 89 42 08          	mov    %rax,0x8(%rdx)
	prev->next = next;
    2d3c:	48 89 10             	mov    %rdx,(%rax)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    2d3f:	49 89 0e             	mov    %rcx,(%r14)
	entry->prev = LIST_POISON2;
    2d42:	48 b9 00 02 20 00 00 	movabs $0xdead000000200200,%rcx
    2d49:	00 ad de 
    2d4c:	49 89 4e 08          	mov    %rcx,0x8(%r14)
	list_del(&conreq->list);
	lep->conreqcnt--;
    2d50:	83 ab 58 01 00 00 01 	subl   $0x1,0x158(%rbx)
    2d57:	e8 00 00 00 00       	callq  2d5c <__scif_accept+0x24c>
	spin_unlock_irqrestore(&lep->lock, sflags);

	// Fill in the peer information
	peer->node = conreq->msg.src.node;
    2d5c:	41 0f b7 46 d0       	movzwl -0x30(%r14),%eax
    2d61:	48 8b 95 10 ff ff ff 	mov    -0xf0(%rbp),%rdx
    2d68:	66 89 02             	mov    %ax,(%rdx)
	peer->port = conreq->msg.src.port;
    2d6b:	41 0f b7 46 d2       	movzwl -0x2e(%r14),%eax
    2d70:	66 89 42 02          	mov    %ax,0x2(%rdx)
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
    2d74:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 2d7b <__scif_accept+0x26b>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
    2d7b:	48 85 ff             	test   %rdi,%rdi
    2d7e:	0f 84 fc 04 00 00    	je     3280 <__scif_accept+0x770>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
    2d84:	ba 90 02 00 00       	mov    $0x290,%edx
    2d89:	be d0 80 00 00       	mov    $0x80d0,%esi
    2d8e:	e8 00 00 00 00       	callq  2d93 <__scif_accept+0x283>

	// Create the connection endpoint
	cep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL);
	if (!cep) {
    2d93:	48 85 c0             	test   %rax,%rax
    2d96:	49 89 c4             	mov    %rax,%r12
    2d99:	0f 84 19 05 00 00    	je     32b8 <__scif_accept+0x7a8>
		pr_debug("SCIFAPI accept: ep %p new end point allocation failed\n", lep);
		err = -ENOMEM;
		goto scif_accept_error_epalloc;
	}
	spin_lock_init(&cep->lock);
    2d9f:	31 f6                	xor    %esi,%esi
	mutex_init (&cep->sendlock);
    2da1:	48 c7 c2 00 00 00 00 	mov    $0x0,%rdx
	if (!cep) {
		pr_debug("SCIFAPI accept: ep %p new end point allocation failed\n", lep);
		err = -ENOMEM;
		goto scif_accept_error_epalloc;
	}
	spin_lock_init(&cep->lock);
    2da8:	66 41 89 74 24 04    	mov    %si,0x4(%r12)
	mutex_init (&cep->sendlock);
    2dae:	49 8d bc 24 00 02 00 	lea    0x200(%r12),%rdi
    2db5:	00 
    2db6:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2dbd:	e8 00 00 00 00       	callq  2dc2 <__scif_accept+0x2b2>
	mutex_init (&cep->recvlock);
    2dc2:	48 c7 c2 00 00 00 00 	mov    $0x0,%rdx
    2dc9:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2dd0:	49 8d bc 24 20 02 00 	lea    0x220(%r12),%rdi
    2dd7:	00 
    2dd8:	e8 00 00 00 00       	callq  2ddd <__scif_accept+0x2cd>
	cep->state = SCIFEP_CONNECTING;
	cep->remote_dev = &scif_dev[peer->node];
    2ddd:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
		goto scif_accept_error_epalloc;
	}
	spin_lock_init(&cep->lock);
	mutex_init (&cep->sendlock);
	mutex_init (&cep->recvlock);
	cep->state = SCIFEP_CONNECTING;
    2de4:	41 c7 04 24 05 00 00 	movl   $0x5,(%r12)
    2deb:	00 
	cep->remote_dev = &scif_dev[peer->node];
	cep->remote_ep = conreq->msg.payload[0];
    2dec:	48 8b 8d f8 fe ff ff 	mov    -0x108(%rbp),%rcx
	}
	spin_lock_init(&cep->lock);
	mutex_init (&cep->sendlock);
	mutex_init (&cep->recvlock);
	cep->state = SCIFEP_CONNECTING;
	cep->remote_dev = &scif_dev[peer->node];
    2df3:	0f b7 00             	movzwl (%rax),%eax
    2df6:	48 89 c2             	mov    %rax,%rdx
    2df9:	48 c1 e0 09          	shl    $0x9,%rax
    2dfd:	48 c1 e2 05          	shl    $0x5,%rdx
    2e01:	48 29 d0             	sub    %rdx,%rax
    2e04:	48 8d 90 00 00 00 00 	lea    0x0(%rax),%rdx
    2e0b:	49 89 94 24 48 01 00 	mov    %rdx,0x148(%r12)
    2e12:	00 
	cep->remote_ep = conreq->msg.payload[0];
    2e13:	48 8b 51 dc          	mov    -0x24(%rcx),%rdx
	cep->sd_state = SCIFDEV_RUNNING;
    2e17:	41 c7 84 24 5c 01 00 	movl   $0x2,0x15c(%r12)
    2e1e:	00 02 00 00 00 
	spin_lock_init(&cep->lock);
	mutex_init (&cep->sendlock);
	mutex_init (&cep->recvlock);
	cep->state = SCIFEP_CONNECTING;
	cep->remote_dev = &scif_dev[peer->node];
	cep->remote_ep = conreq->msg.payload[0];
    2e23:	49 89 94 24 50 01 00 	mov    %rdx,0x150(%r12)
    2e2a:	00 
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    2e2b:	8b 80 00 00 00 00    	mov    0x0(%rax),%eax
    2e31:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    2e34:	83 f8 01             	cmp    $0x1,%eax
    2e37:	0f 87 fc 04 00 00    	ja     3339 <__scif_accept+0x829>
		err = -ENODEV;
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto scif_accept_error_qpalloc;
	}

	if (micscif_rma_ep_init(cep) < 0) {
    2e3d:	4c 89 e7             	mov    %r12,%rdi
    2e40:	e8 00 00 00 00       	callq  2e45 <__scif_accept+0x335>
    2e45:	85 c0                	test   %eax,%eax
    2e47:	0f 88 db 05 00 00    	js     3428 <__scif_accept+0x918>
		pr_debug("SCIFAPI accept: ep %p new %p RMA EP init failed\n", lep, cep);
		err = -ENOMEM;
		goto scif_accept_error_qpalloc;
	}

	if ((err = micscif_reserve_dma_chan(cep))) {
    2e4d:	4c 89 e7             	mov    %r12,%rdi
    2e50:	e8 00 00 00 00       	callq  2e55 <__scif_accept+0x345>
    2e55:	85 c0                	test   %eax,%eax
    2e57:	0f 85 40 08 00 00    	jne    369d <__scif_accept+0xb8d>
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
    2e5d:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 2e64 <__scif_accept+0x354>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
    2e64:	48 85 ff             	test   %rdi,%rdi
    2e67:	0f 84 1e 04 00 00    	je     328b <__scif_accept+0x77b>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
    2e6d:	ba b0 00 00 00       	mov    $0xb0,%edx
    2e72:	be d0 80 00 00       	mov    $0x80d0,%esi
    2e77:	e8 00 00 00 00       	callq  2e7c <__scif_accept+0x36c>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto scif_accept_error_qpalloc;
	}

	cep->qp_info.qp = (struct micscif_qp *)kzalloc(sizeof(struct micscif_qp), GFP_KERNEL);
	if (!cep->qp_info.qp) {
    2e7c:	48 85 c0             	test   %rax,%rax
	if ((err = micscif_reserve_dma_chan(cep))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto scif_accept_error_qpalloc;
	}

	cep->qp_info.qp = (struct micscif_qp *)kzalloc(sizeof(struct micscif_qp), GFP_KERNEL);
    2e7f:	49 89 44 24 10       	mov    %rax,0x10(%r12)
	if (!cep->qp_info.qp) {
    2e84:	0f 84 ee 07 00 00    	je     3678 <__scif_accept+0xb68>
		printk(KERN_ERR "Port Qp Allocation Failed\n");
		err = -ENOMEM;
		goto scif_accept_error_qpalloc;
	}

	cep->qp_info.qp->magic = SCIFEP_MAGIC;
    2e8a:	48 b9 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rcx
    2e91:	00 1f 5c 
    2e94:	48 89 48 08          	mov    %rcx,0x8(%rax)
	cep->qp_info.qp->ep = (uint64_t)cep;
    2e98:	49 8b 44 24 10       	mov    0x10(%r12),%rax
    2e9d:	4c 89 20             	mov    %r12,(%rax)
	micscif_inc_node_refcnt(cep->remote_dev, 1);
    2ea0:	4d 8b 84 24 48 01 00 	mov    0x148(%r12),%r8
    2ea7:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    2ea8:	4d 85 c0             	test   %r8,%r8
    2eab:	0f 85 3e 04 00 00    	jne    32ef <__scif_accept+0x7df>
	err = micscif_setup_qp_accept(cep->qp_info.qp, &cep->qp_info.qp_offset,
    2eb1:	49 8b 7c 24 10       	mov    0x10(%r12),%rdi
    2eb6:	49 8d 74 24 18       	lea    0x18(%r12),%rsi
    2ebb:	b9 00 10 00 00       	mov    $0x1000,%ecx
    2ec0:	48 8b 85 f8 fe ff ff 	mov    -0x108(%rbp),%rax
    2ec7:	48 8b 50 e4          	mov    -0x1c(%rax),%rdx
    2ecb:	e8 00 00 00 00       	callq  2ed0 <__scif_accept+0x3c0>
		conreq->msg.payload[1], ENDPT_QP_SIZE, cep->remote_dev);
	if (err) {
    2ed0:	85 c0                	test   %eax,%eax
	}

	cep->qp_info.qp->magic = SCIFEP_MAGIC;
	cep->qp_info.qp->ep = (uint64_t)cep;
	micscif_inc_node_refcnt(cep->remote_dev, 1);
	err = micscif_setup_qp_accept(cep->qp_info.qp, &cep->qp_info.qp_offset,
    2ed2:	41 89 c3             	mov    %eax,%r11d
		conreq->msg.payload[1], ENDPT_QP_SIZE, cep->remote_dev);
	if (err) {
    2ed5:	0f 85 75 05 00 00    	jne    3450 <__scif_accept+0x940>
			    lep, cep, err, cep->qp_info.qp_offset);
		micscif_dec_node_refcnt(cep->remote_dev, 1);
		goto scif_accept_error_map;
	}

	cep->port.node = lep->port.node;
    2edb:	0f b7 43 06          	movzwl 0x6(%rbx),%eax
	cep->port.port = lep->port.port;
	cep->peer.node = peer->node;
	cep->peer.port = peer->port;
	cep->accepted_ep = true;
	init_waitqueue_head(&cep->sendwq); // Wait for data to be consumed
    2edf:	49 8d bc 24 d0 01 00 	lea    0x1d0(%r12),%rdi
    2ee6:	00 
    2ee7:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
		goto scif_accept_error_map;
	}

	cep->port.node = lep->port.node;
	cep->port.port = lep->port.port;
	cep->peer.node = peer->node;
    2eee:	48 8b 8d 10 ff ff ff 	mov    -0xf0(%rbp),%rcx
			    lep, cep, err, cep->qp_info.qp_offset);
		micscif_dec_node_refcnt(cep->remote_dev, 1);
		goto scif_accept_error_map;
	}

	cep->port.node = lep->port.node;
    2ef5:	66 41 89 44 24 06    	mov    %ax,0x6(%r12)
	cep->port.port = lep->port.port;
    2efb:	0f b7 43 08          	movzwl 0x8(%rbx),%eax
    2eff:	66 41 89 44 24 08    	mov    %ax,0x8(%r12)
	cep->peer.node = peer->node;
    2f05:	0f b7 01             	movzwl (%rcx),%eax
    2f08:	66 41 89 44 24 0a    	mov    %ax,0xa(%r12)
	cep->peer.port = peer->port;
    2f0e:	0f b7 41 02          	movzwl 0x2(%rcx),%eax
	cep->accepted_ep = true;
    2f12:	41 c6 84 24 60 01 00 	movb   $0x1,0x160(%r12)
    2f19:	00 01 
	}

	cep->port.node = lep->port.node;
	cep->port.port = lep->port.port;
	cep->peer.node = peer->node;
	cep->peer.port = peer->port;
    2f1b:	66 41 89 44 24 0c    	mov    %ax,0xc(%r12)
	cep->accepted_ep = true;
	init_waitqueue_head(&cep->sendwq); // Wait for data to be consumed
    2f21:	e8 00 00 00 00       	callq  2f26 <__scif_accept+0x416>
	init_waitqueue_head(&cep->recvwq); // Wait for data to be produced
    2f26:	49 8d bc 24 e8 01 00 	lea    0x1e8(%r12),%rdi
    2f2d:	00 
    2f2e:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2f35:	e8 00 00 00 00       	callq  2f3a <__scif_accept+0x42a>
	init_waitqueue_head(&cep->conwq);  // Wait for connection request
    2f3a:	49 8d 84 24 88 01 00 	lea    0x188(%r12),%rax
    2f41:	00 
    2f42:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    2f49:	48 89 c7             	mov    %rax,%rdi
    2f4c:	49 89 c6             	mov    %rax,%r14
    2f4f:	e8 00 00 00 00       	callq  2f54 <__scif_accept+0x444>
	msg.uop = SCIF_CNCT_GNT;
	msg.src = cep->port;
	msg.payload[0] = cep->remote_ep;
	msg.payload[1] = cep->qp_info.qp_offset;

	err = micscif_nodeqp_send(cep->remote_dev, &msg, cep);
    2f54:	49 8b bc 24 48 01 00 	mov    0x148(%r12),%rdi
    2f5b:	00 
    2f5c:	4c 89 e2             	mov    %r12,%rdx
	init_waitqueue_head(&cep->sendwq); // Wait for data to be consumed
	init_waitqueue_head(&cep->recvwq); // Wait for data to be produced
	init_waitqueue_head(&cep->conwq);  // Wait for connection request

	// Return the grant message
	msg.uop = SCIF_CNCT_GNT;
    2f5f:	c7 85 7c ff ff ff 06 	movl   $0x6,-0x84(%rbp)
    2f66:	00 00 00 
	msg.src = cep->port;
    2f69:	41 8b 44 24 06       	mov    0x6(%r12),%eax
    2f6e:	89 85 74 ff ff ff    	mov    %eax,-0x8c(%rbp)
	msg.payload[0] = cep->remote_ep;
    2f74:	49 8b 84 24 50 01 00 	mov    0x150(%r12),%rax
    2f7b:	00 
    2f7c:	48 89 45 80          	mov    %rax,-0x80(%rbp)
	msg.payload[1] = cep->qp_info.qp_offset;
    2f80:	49 8b 44 24 18       	mov    0x18(%r12),%rax
    2f85:	48 89 45 88          	mov    %rax,-0x78(%rbp)

	err = micscif_nodeqp_send(cep->remote_dev, &msg, cep);
    2f89:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
    2f90:	48 89 c6             	mov    %rax,%rsi
    2f93:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    2f9a:	e8 00 00 00 00       	callq  2f9f <__scif_accept+0x48f>

	micscif_dec_node_refcnt(cep->remote_dev, 1);
    2f9f:	4d 8b 94 24 48 01 00 	mov    0x148(%r12),%r10
    2fa6:	00 
	msg.uop = SCIF_CNCT_GNT;
	msg.src = cep->port;
	msg.payload[0] = cep->remote_ep;
	msg.payload[1] = cep->qp_info.qp_offset;

	err = micscif_nodeqp_send(cep->remote_dev, &msg, cep);
    2fa7:	41 89 c3             	mov    %eax,%r11d
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    2faa:	4d 85 d2             	test   %r10,%r10
    2fad:	74 28                	je     2fd7 <__scif_accept+0x4c7>
		if (unlikely((atomic_long_sub_return(cnt, 
    2faf:	49 8d 82 88 01 00 00 	lea    0x188(%r10),%rax
    2fb6:	48 89 85 20 ff ff ff 	mov    %rax,-0xe0(%rbp)
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    2fbd:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    2fc4:	f0 49 0f c1 82 88 01 	lock xadd %rax,0x188(%r10)
    2fcb:	00 00 
    2fcd:	48 83 e8 01          	sub    $0x1,%rax
    2fd1:	0f 88 9a 03 00 00    	js     3371 <__scif_accept+0x861>

	micscif_dec_node_refcnt(cep->remote_dev, 1);
	if (err)
    2fd7:	45 85 db             	test   %r11d,%r11d
    2fda:	0f 85 2f 01 00 00    	jne    310f <__scif_accept+0x5ff>
    2fe0:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    2fe7:	00 00 
    2fe9:	48 89 9d 20 ff ff ff 	mov    %rbx,-0xe0(%rbp)
    2ff0:	48 89 85 00 ff ff ff 	mov    %rax,-0x100(%rbp)
		goto scif_accept_error_map;
retry:
	err = wait_event_timeout(cep->conwq, 
    2ff7:	41 8b 04 24          	mov    (%r12),%eax
    2ffb:	83 f8 05             	cmp    $0x5,%eax
    2ffe:	74 78                	je     3078 <__scif_accept+0x568>
	}

	if (err > 0)
		err = 0;

	kfree(conreq);
    3000:	48 8b bd 28 ff ff ff 	mov    -0xd8(%rbp),%rdi
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    3007:	4d 8d 74 24 04       	lea    0x4(%r12),%r14
    300c:	48 8b 9d 20 ff ff ff 	mov    -0xe0(%rbp),%rbx
    3013:	e8 00 00 00 00       	callq  3018 <__scif_accept+0x508>

	spin_lock_irqsave(&cep->lock, sflags);
    3018:	4c 89 f7             	mov    %r14,%rdi
    301b:	e8 00 00 00 00       	callq  3020 <__scif_accept+0x510>

	if (cep->state == SCIFEP_CONNECTED) {
    3020:	41 8b 14 24          	mov    (%r12),%edx
    3024:	83 fa 04             	cmp    $0x4,%edx
    3027:	0f 84 71 02 00 00    	je     329e <__scif_accept+0x78e>
		spin_unlock_irqrestore(&cep->lock, sflags);
		pr_debug("SCIFAPI accept: ep %p new %p returning new epnd point\n", lep, cep);
		return 0;
	}

	if (cep->state == SCIFEP_CLOSING) {
    302d:	41 8b 14 24          	mov    (%r12),%edx
    3031:	83 fa 07             	cmp    $0x7,%edx
    3034:	0f 85 64 02 00 00    	jne    329e <__scif_accept+0x78e>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    303a:	48 89 c6             	mov    %rax,%rsi
    303d:	4c 89 f7             	mov    %r14,%rdi
    3040:	e8 00 00 00 00       	callq  3045 <__scif_accept+0x535>
		// Remote failed to allocate resources and NAKed the grant.
		// There is at this point nothing referencing the new end point.
		spin_unlock_irqrestore(&cep->lock, sflags);
		micscif_teardown_ep((void *)cep);
    3045:	4c 89 e7             	mov    %r12,%rdi
    3048:	e8 00 00 00 00       	callq  304d <__scif_accept+0x53d>
		kfree(cep);
    304d:	4c 89 e7             	mov    %r12,%rdi
    3050:	e8 00 00 00 00       	callq  3055 <__scif_accept+0x545>

		// If call with sync flag then go back and wait.
		if (flags & SCIF_ACCEPT_SYNC) {
    3055:	8b 95 0c ff ff ff    	mov    -0xf4(%rbp),%edx
    305b:	85 d2                	test   %edx,%edx
    305d:	0f 84 de 03 00 00    	je     3441 <__scif_accept+0x931>
			spin_lock_irqsave(&lep->lock, sflags);
    3063:	4c 89 ff             	mov    %r15,%rdi
    3066:	e8 00 00 00 00       	callq  306b <__scif_accept+0x55b>
    306b:	48 89 c6             	mov    %rax,%rsi
			goto retry_connection;
    306e:	e9 8a fb ff ff       	jmpq   2bfd <__scif_accept+0xed>
    3073:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

	micscif_dec_node_refcnt(cep->remote_dev, 1);
	if (err)
		goto scif_accept_error_map;
retry:
	err = wait_event_timeout(cep->conwq, 
    3078:	48 8b 85 00 ff ff ff 	mov    -0x100(%rbp),%rax
    307f:	48 c7 45 a0 00 00 00 	movq   $0x0,-0x60(%rbp)
    3086:	00 
    3087:	bb 2c 01 00 00       	mov    $0x12c,%ebx
    308c:	48 c7 45 b0 00 00 00 	movq   $0x0,-0x50(%rbp)
    3093:	00 
    3094:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    3098:	48 8b 85 38 ff ff ff 	mov    -0xc8(%rbp),%rax
    309f:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    30a3:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    30a7:	eb 1b                	jmp    30c4 <__scif_accept+0x5b4>
    30a9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    30b0:	48 89 df             	mov    %rbx,%rdi
    30b3:	e8 00 00 00 00       	callq  30b8 <__scif_accept+0x5a8>
    30b8:	48 85 c0             	test   %rax,%rax
    30bb:	48 89 c3             	mov    %rax,%rbx
    30be:	0f 84 9c 01 00 00    	je     3260 <__scif_accept+0x750>
    30c4:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    30c8:	ba 02 00 00 00       	mov    $0x2,%edx
    30cd:	4c 89 f7             	mov    %r14,%rdi
    30d0:	e8 00 00 00 00       	callq  30d5 <__scif_accept+0x5c5>
    30d5:	41 8b 04 24          	mov    (%r12),%eax
    30d9:	83 f8 05             	cmp    $0x5,%eax
    30dc:	74 d2                	je     30b0 <__scif_accept+0x5a0>
    30de:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    30e2:	4c 89 f7             	mov    %r14,%rdi
    30e5:	e8 00 00 00 00       	callq  30ea <__scif_accept+0x5da>
		(cep->state != SCIFEP_CONNECTING), NODE_ACCEPT_TIMEOUT);
	if (!err && scifdev_alive(cep))
    30ea:	85 db                	test   %ebx,%ebx
    30ec:	0f 85 0e ff ff ff    	jne    3000 <__scif_accept+0x4f0>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    30f2:	49 8b 84 24 48 01 00 	mov    0x148(%r12),%rax
    30f9:	00 
    30fa:	8b 40 04             	mov    0x4(%rax),%eax
    30fd:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    3100:	83 f8 01             	cmp    $0x1,%eax
    3103:	0f 86 3f 01 00 00    	jbe    3248 <__scif_accept+0x738>
		goto retry;

	if (!err) {
		err = -ENODEV;
    3109:	41 bb ed ff ff ff    	mov    $0xffffffed,%r11d
	spin_unlock_irqrestore(&cep->lock, sflags);
	return 0;

	// Error allocating or mapping resources
scif_accept_error_map:
	kfree(cep->qp_info.qp);
    310f:	49 8b 7c 24 10       	mov    0x10(%r12),%rdi
    3114:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    311b:	e8 00 00 00 00       	callq  3120 <__scif_accept+0x610>
    3120:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d

scif_accept_error_qpalloc:
	kfree(cep);
    3127:	4c 89 e7             	mov    %r12,%rdi
    312a:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    3131:	e8 00 00 00 00       	callq  3136 <__scif_accept+0x626>
    3136:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d

scif_accept_error_epalloc:
	micscif_inc_node_refcnt(&scif_dev[conreq->msg.src.node], 1);
    313d:	48 8b 85 f8 fe ff ff 	mov    -0x108(%rbp),%rax
    3144:	0f b7 58 d0          	movzwl -0x30(%rax),%ebx
    3148:	48 89 d8             	mov    %rbx,%rax
    314b:	49 89 dc             	mov    %rbx,%r12
    314e:	48 c1 e0 05          	shl    $0x5,%rax
    3152:	49 c1 e4 09          	shl    $0x9,%r12
    3156:	49 29 c4             	sub    %rax,%r12
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    3159:	4c 89 e0             	mov    %r12,%rax
    315c:	48 05 00 00 00 00    	add    $0x0,%rax
    3162:	0f 85 f8 03 00 00    	jne    3560 <__scif_accept+0xa50>
	// New reject the connection request due to lack of resources
	msg.uop = SCIF_CNCT_REJ;
	msg.dst.node = conreq->msg.src.node;
    3168:	48 8b 9d f8 fe ff ff 	mov    -0x108(%rbp),%rbx
    316f:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
	kfree(cep);

scif_accept_error_epalloc:
	micscif_inc_node_refcnt(&scif_dev[conreq->msg.src.node], 1);
	// New reject the connection request due to lack of resources
	msg.uop = SCIF_CNCT_REJ;
    3176:	c7 85 7c ff ff ff 09 	movl   $0x9,-0x84(%rbp)
    317d:	00 00 00 
	msg.dst.node = conreq->msg.src.node;
	msg.dst.port = conreq->msg.src.port;
	msg.payload[0] = conreq->msg.payload[0];
	msg.payload[1] = conreq->msg.payload[1];
	/* No error handling for Notification messages */
	micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, NULL);
    3180:	48 8b b5 e8 fe ff ff 	mov    -0x118(%rbp),%rsi

scif_accept_error_epalloc:
	micscif_inc_node_refcnt(&scif_dev[conreq->msg.src.node], 1);
	// New reject the connection request due to lack of resources
	msg.uop = SCIF_CNCT_REJ;
	msg.dst.node = conreq->msg.src.node;
    3187:	0f b7 43 d0          	movzwl -0x30(%rbx),%eax
    318b:	66 89 85 78 ff ff ff 	mov    %ax,-0x88(%rbp)
	msg.dst.port = conreq->msg.src.port;
    3192:	0f b7 43 d2          	movzwl -0x2e(%rbx),%eax
    3196:	66 89 85 7a ff ff ff 	mov    %ax,-0x86(%rbp)
	msg.payload[0] = conreq->msg.payload[0];
    319d:	48 8b 43 dc          	mov    -0x24(%rbx),%rax
    31a1:	48 89 45 80          	mov    %rax,-0x80(%rbp)
	msg.payload[1] = conreq->msg.payload[1];
    31a5:	48 8b 43 e4          	mov    -0x1c(%rbx),%rax
    31a9:	48 89 45 88          	mov    %rax,-0x78(%rbp)
	/* No error handling for Notification messages */
	micscif_nodeqp_send(&scif_dev[conreq->msg.src.node], &msg, NULL);
    31ad:	0f b7 43 d0          	movzwl -0x30(%rbx),%eax
    31b1:	48 89 c2             	mov    %rax,%rdx
    31b4:	48 c1 e0 09          	shl    $0x9,%rax
    31b8:	48 c1 e2 05          	shl    $0x5,%rdx
    31bc:	48 89 c7             	mov    %rax,%rdi
    31bf:	48 29 d7             	sub    %rdx,%rdi
    31c2:	31 d2                	xor    %edx,%edx
    31c4:	48 81 c7 00 00 00 00 	add    $0x0,%rdi
    31cb:	e8 00 00 00 00       	callq  31d0 <__scif_accept+0x6c0>
	micscif_dec_node_refcnt(&scif_dev[conreq->msg.src.node], 1);
    31d0:	0f b7 5b d0          	movzwl -0x30(%rbx),%ebx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    31d4:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    31db:	48 89 d8             	mov    %rbx,%rax
    31de:	48 c1 e3 09          	shl    $0x9,%rbx
    31e2:	48 c1 e0 05          	shl    $0x5,%rax
    31e6:	48 29 c3             	sub    %rax,%rbx
    31e9:	48 89 d9             	mov    %rbx,%rcx
    31ec:	48 81 c1 00 00 00 00 	add    $0x0,%rcx
    31f3:	74 28                	je     321d <__scif_accept+0x70d>
		if (unlikely((atomic_long_sub_return(cnt, 
    31f5:	48 81 c3 80 01 00 00 	add    $0x180,%rbx
    31fc:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    3203:	4c 8d a3 00 00 00 00 	lea    0x0(%rbx),%r12
    320a:	f0 48 0f c1 83 00 00 	lock xadd %rax,0x0(%rbx)
    3211:	00 00 
    3213:	48 83 e8 01          	sub    $0x1,%rax
    3217:	0f 88 c6 03 00 00    	js     35e3 <__scif_accept+0xad3>

	kfree(conreq);
    321d:	48 8b bd 28 ff ff ff 	mov    -0xd8(%rbp),%rdi
    3224:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    322b:	e8 00 00 00 00       	callq  3230 <__scif_accept+0x720>
	return err;
    3230:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    3237:	44 89 9d 30 ff ff ff 	mov    %r11d,-0xd0(%rbp)
    323e:	e9 58 fa ff ff       	jmpq   2c9b <__scif_accept+0x18b>
    3243:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    3248:	41 83 bc 24 5c 01 00 	cmpl   $0x2,0x15c(%r12)
    324f:	00 02 
    3251:	0f 85 b2 fe ff ff    	jne    3109 <__scif_accept+0x5f9>
    3257:	e9 9b fd ff ff       	jmpq   2ff7 <__scif_accept+0x4e7>
    325c:	0f 1f 40 00          	nopl   0x0(%rax)

	micscif_dec_node_refcnt(cep->remote_dev, 1);
	if (err)
		goto scif_accept_error_map;
retry:
	err = wait_event_timeout(cep->conwq, 
    3260:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    3264:	4c 89 f7             	mov    %r14,%rdi
    3267:	e8 00 00 00 00       	callq  326c <__scif_accept+0x75c>
    326c:	e9 81 fe ff ff       	jmpq   30f2 <__scif_accept+0x5e2>
	pr_debug("SCIFAPI accept: ep %p %s\n", lep, scif_ep_states[lep->state]);

	// Error if flags other than SCIF_ACCEPT_SYNC are set
	if (flags & ~SCIF_ACCEPT_SYNC) {
		pr_debug("SCIFAPI accept: ep %p invalid flags %x\n", lep, flags & ~SCIF_ACCEPT_SYNC);
		return -EINVAL;
    3271:	c7 85 30 ff ff ff ea 	movl   $0xffffffea,-0xd0(%rbp)
    3278:	ff ff ff 
    327b:	e9 1b fa ff ff       	jmpq   2c9b <__scif_accept+0x18b>

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
    3280:	41 bc 10 00 00 00    	mov    $0x10,%r12d
    3286:	e9 14 fb ff ff       	jmpq   2d9f <__scif_accept+0x28f>
	if ((err = micscif_reserve_dma_chan(cep))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto scif_accept_error_qpalloc;
	}

	cep->qp_info.qp = (struct micscif_qp *)kzalloc(sizeof(struct micscif_qp), GFP_KERNEL);
    328b:	49 c7 44 24 10 10 00 	movq   $0x10,0x10(%r12)
    3292:	00 00 
    3294:	b8 10 00 00 00       	mov    $0x10,%eax
    3299:	e9 ec fb ff ff       	jmpq   2e8a <__scif_accept+0x37a>

	spin_lock_irqsave(&cep->lock, sflags);

	if (cep->state == SCIFEP_CONNECTED) {
		// Connect sequence complete return new endpoint information
		*newepd = (scif_epd_t)cep;
    329e:	48 8b 8d f0 fe ff ff 	mov    -0x110(%rbp),%rcx
    32a5:	48 89 c6             	mov    %rax,%rsi
    32a8:	4c 89 f7             	mov    %r14,%rdi
    32ab:	4c 89 21             	mov    %r12,(%rcx)
    32ae:	e8 00 00 00 00       	callq  32b3 <__scif_accept+0x7a3>
		spin_unlock_irqrestore(&cep->lock, sflags);
		pr_debug("SCIFAPI accept: ep %p new %p returning new epnd point\n", lep, cep);
		return 0;
    32b3:	e9 e3 f9 ff ff       	jmpq   2c9b <__scif_accept+0x18b>
    32b8:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax

	// Create the connection endpoint
	cep = (struct endpt *)kzalloc(sizeof(struct endpt), GFP_KERNEL);
	if (!cep) {
		pr_debug("SCIFAPI accept: ep %p new end point allocation failed\n", lep);
		err = -ENOMEM;
    32bf:	41 bb f4 ff ff ff    	mov    $0xfffffff4,%r11d
    32c5:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    32cc:	e9 6c fe ff ff       	jmpq   313d <__scif_accept+0x62d>
    32d1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    32d8:	4c 89 ff             	mov    %r15,%rdi
    32db:	e8 00 00 00 00       	callq  32e0 <__scif_accept+0x7d0>
	might_sleep();
	spin_lock_irqsave(&lep->lock, sflags);
	if (lep->state != SCIFEP_LISTENING) {
		pr_debug("SCIFAPI accept: ep %p not listending\n", lep);
		spin_unlock_irqrestore(&lep->lock, sflags);
		return -EINVAL;
    32e0:	c7 85 30 ff ff ff ea 	movl   $0xffffffea,-0xd0(%rbp)
    32e7:	ff ff ff 
    32ea:	e9 ac f9 ff ff       	jmpq   2c9b <__scif_accept+0x18b>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    32ef:	49 8b 88 88 01 00 00 	mov    0x188(%r8),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    32f6:	4d 8d b0 88 01 00 00 	lea    0x188(%r8),%r14
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    32fd:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    3304:	00 00 80 
    3307:	48 39 f1             	cmp    %rsi,%rcx
    330a:	0f 84 f2 01 00 00    	je     3502 <__scif_accept+0x9f2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    3310:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    3314:	48 89 c8             	mov    %rcx,%rax
    3317:	f0 49 0f b1 90 88 01 	lock cmpxchg %rdx,0x188(%r8)
    331e:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    3320:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    3323:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    3326:	0f 85 b4 01 00 00    	jne    34e0 <__scif_accept+0x9d0>
    332c:	4d 8b 84 24 48 01 00 	mov    0x148(%r12),%r8
    3333:	00 
    3334:	e9 78 fb ff ff       	jmpq   2eb1 <__scif_accept+0x3a1>
	cep->remote_ep = conreq->msg.payload[0];
	cep->sd_state = SCIFDEV_RUNNING;

	if (!scifdev_alive(cep)) {
		err = -ENODEV;
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    3339:	b9 ed ff ff ff       	mov    $0xffffffed,%ecx
    333e:	ba 02 04 00 00       	mov    $0x402,%edx
    3343:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    334a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    3351:	31 c0                	xor    %eax,%eax
    3353:	e8 00 00 00 00       	callq  3358 <__scif_accept+0x848>
    3358:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
	cep->remote_dev = &scif_dev[peer->node];
	cep->remote_ep = conreq->msg.payload[0];
	cep->sd_state = SCIFDEV_RUNNING;

	if (!scifdev_alive(cep)) {
		err = -ENODEV;
    335f:	41 bb ed ff ff ff    	mov    $0xffffffed,%r11d
    3365:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto scif_accept_error_qpalloc;
    336c:	e9 b6 fd ff ff       	jmpq   3127 <__scif_accept+0x617>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    3371:	48 8b 45 08          	mov    0x8(%rbp),%rax
    3375:	4c 89 d1             	mov    %r10,%rcx
    3378:	ba a7 00 00 00       	mov    $0xa7,%edx
    337d:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    3384:	4d 8b 8a 88 01 00 00 	mov    0x188(%r10),%r9
    338b:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    3392:	44 89 9d e0 fe ff ff 	mov    %r11d,-0x120(%rbp)
    3399:	45 0f b7 02          	movzwl (%r10),%r8d
    339d:	4c 89 95 00 ff ff ff 	mov    %r10,-0x100(%rbp)
    33a4:	48 89 04 24          	mov    %rax,(%rsp)
    33a8:	31 c0                	xor    %eax,%eax
    33aa:	e8 00 00 00 00       	callq  33af <__scif_accept+0x89f>
    33af:	4c 8b 95 00 ff ff ff 	mov    -0x100(%rbp),%r10
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    33b6:	48 b9 00 00 00 00 00 	movabs $0x8000000000000000,%rcx
    33bd:	00 00 80 
    33c0:	44 8b 9d e0 fe ff ff 	mov    -0x120(%rbp),%r11d
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    33c7:	49 8b b2 88 01 00 00 	mov    0x188(%r10),%rsi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    33ce:	48 39 ce             	cmp    %rcx,%rsi
    33d1:	0f 84 00 fc ff ff    	je     2fd7 <__scif_accept+0x4c7>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    33d7:	48 8d 56 01          	lea    0x1(%rsi),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    33db:	48 89 f0             	mov    %rsi,%rax
    33de:	f0 49 0f b1 92 88 01 	lock cmpxchg %rdx,0x188(%r10)
    33e5:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    33e7:	48 39 c6             	cmp    %rax,%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    33ea:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    33ed:	0f 84 e4 fb ff ff    	je     2fd7 <__scif_accept+0x4c7>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    33f3:	48 39 ca             	cmp    %rcx,%rdx
    33f6:	48 8b b5 20 ff ff ff 	mov    -0xe0(%rbp),%rsi
    33fd:	0f 84 d4 fb ff ff    	je     2fd7 <__scif_accept+0x4c7>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    3403:	48 8d 7a 01          	lea    0x1(%rdx),%rdi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    3407:	48 89 d0             	mov    %rdx,%rax
    340a:	f0 48 0f b1 3e       	lock cmpxchg %rdi,(%rsi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    340f:	48 39 c2             	cmp    %rax,%rdx
    3412:	0f 84 bf fb ff ff    	je     2fd7 <__scif_accept+0x4c7>
    3418:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    341b:	48 39 ca             	cmp    %rcx,%rdx
    341e:	75 e3                	jne    3403 <__scif_accept+0x8f3>
    3420:	e9 b2 fb ff ff       	jmpq   2fd7 <__scif_accept+0x4c7>
    3425:	0f 1f 00             	nopl   (%rax)
    3428:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
	}

	if (micscif_rma_ep_init(cep) < 0) {
		pr_debug("SCIFAPI accept: ep %p new %p RMA EP init failed\n", lep, cep);
		err = -ENOMEM;
    342f:	41 bb f4 ff ff ff    	mov    $0xfffffff4,%r11d
    3435:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    343c:	e9 e6 fc ff ff       	jmpq   3127 <__scif_accept+0x617>
			spin_lock_irqsave(&lep->lock, sflags);
			goto retry_connection;
		}

		pr_debug("SCIFAPI accept: ep %p new %p remote failed to allocate resources\n", lep, cep);
		return -EAGAIN;
    3441:	c7 85 30 ff ff ff f5 	movl   $0xfffffff5,-0xd0(%rbp)
    3448:	ff ff ff 
    344b:	e9 4b f8 ff ff       	jmpq   2c9b <__scif_accept+0x18b>
	err = micscif_setup_qp_accept(cep->qp_info.qp, &cep->qp_info.qp_offset,
		conreq->msg.payload[1], ENDPT_QP_SIZE, cep->remote_dev);
	if (err) {
		pr_debug("SCIFAPI accept: ep %p new %p micscif_setup_qp_accept %d qp_offset 0x%llx\n", 
			    lep, cep, err, cep->qp_info.qp_offset);
		micscif_dec_node_refcnt(cep->remote_dev, 1);
    3450:	49 8b 8c 24 48 01 00 	mov    0x148(%r12),%rcx
    3457:	00 
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    3458:	48 85 c9             	test   %rcx,%rcx
    345b:	0f 84 6f 01 00 00    	je     35d0 <__scif_accept+0xac0>
		if (unlikely((atomic_long_sub_return(cnt, 
    3461:	48 8d 99 88 01 00 00 	lea    0x188(%rcx),%rbx
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    3468:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    346f:	f0 48 0f c1 81 88 01 	lock xadd %rax,0x188(%rcx)
    3476:	00 00 
    3478:	48 8d 95 74 ff ff ff 	lea    -0x8c(%rbp),%rdx
    347f:	48 83 e8 01          	sub    $0x1,%rax
    3483:	48 89 95 e8 fe ff ff 	mov    %rdx,-0x118(%rbp)
    348a:	0f 89 7f fc ff ff    	jns    310f <__scif_accept+0x5ff>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    3490:	48 8b 45 08          	mov    0x8(%rbp),%rax
    3494:	ba a7 00 00 00       	mov    $0xa7,%edx
    3499:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    34a0:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    34a7:	4c 8b 89 88 01 00 00 	mov    0x188(%rcx),%r9
    34ae:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    34b5:	44 0f b7 01          	movzwl (%rcx),%r8d
    34b9:	48 89 04 24          	mov    %rax,(%rsp)
    34bd:	31 c0                	xor    %eax,%eax
    34bf:	e8 00 00 00 00       	callq  34c4 <__scif_accept+0x9b4>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    34c4:	48 89 df             	mov    %rbx,%rdi
    34c7:	e8 94 cf ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    34cc:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    34d3:	e9 37 fc ff ff       	jmpq   310f <__scif_accept+0x5ff>
    34d8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    34df:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    34e0:	48 39 f2             	cmp    %rsi,%rdx
    34e3:	74 1d                	je     3502 <__scif_accept+0x9f2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    34e5:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    34e9:	48 89 d0             	mov    %rdx,%rax
    34ec:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    34f1:	48 39 c2             	cmp    %rax,%rdx
    34f4:	0f 84 32 fe ff ff    	je     332c <__scif_accept+0x81c>
    34fa:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    34fd:	48 39 f2             	cmp    %rsi,%rdx
    3500:	75 e3                	jne    34e5 <__scif_accept+0x9d5>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    3502:	49 8d 80 68 01 00 00 	lea    0x168(%r8),%rax
    3509:	4c 89 85 20 ff ff ff 	mov    %r8,-0xe0(%rbp)
    3510:	48 89 c7             	mov    %rax,%rdi
    3513:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    351a:	e8 00 00 00 00       	callq  351f <__scif_accept+0xa0f>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    351f:	4c 8b 85 20 ff ff ff 	mov    -0xe0(%rbp),%r8
    3526:	41 8b 40 04          	mov    0x4(%r8),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    352a:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    352d:	83 fa 01             	cmp    $0x1,%edx
    3530:	76 1d                	jbe    354f <__scif_accept+0xa3f>
    3532:	83 f8 01             	cmp    $0x1,%eax
    3535:	74 18                	je     354f <__scif_accept+0xa3f>
    3537:	49 8b 80 88 01 00 00 	mov    0x188(%r8),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    353e:	48 85 c0             	test   %rax,%rax
    3541:	0f 88 92 01 00 00    	js     36d9 <__scif_accept+0xbc9>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    3547:	4c 89 f7             	mov    %r14,%rdi
    354a:	e8 a1 cf ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    354f:	48 8b bd e8 fe ff ff 	mov    -0x118(%rbp),%rdi
    3556:	e8 00 00 00 00       	callq  355b <__scif_accept+0xa4b>
    355b:	e9 cc fd ff ff       	jmpq   332c <__scif_accept+0x81c>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    3560:	49 8b b4 24 00 00 00 	mov    0x0(%r12),%rsi
    3567:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    3568:	49 8d 94 24 80 01 00 	lea    0x180(%r12),%rdx
    356f:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    3570:	48 b9 00 00 00 00 00 	movabs $0x8000000000000000,%rcx
    3577:	00 00 80 
    357a:	4d 8d ac 24 00 00 00 	lea    0x0(%r12),%r13
    3581:	00 
    3582:	48 39 ce             	cmp    %rcx,%rsi
    3585:	0f 84 c9 01 00 00    	je     3754 <__scif_accept+0xc44>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    358b:	48 8d 7e 01          	lea    0x1(%rsi),%rdi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    358f:	48 89 f0             	mov    %rsi,%rax
    3592:	f0 48 0f b1 ba 00 00 	lock cmpxchg %rdi,0x0(%rdx)
    3599:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    359b:	48 39 c6             	cmp    %rax,%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    359e:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    35a1:	0f 84 c1 fb ff ff    	je     3168 <__scif_accept+0x658>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    35a7:	48 39 ca             	cmp    %rcx,%rdx
    35aa:	0f 84 a4 01 00 00    	je     3754 <__scif_accept+0xc44>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    35b0:	48 8d 72 01          	lea    0x1(%rdx),%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    35b4:	48 89 d0             	mov    %rdx,%rax
    35b7:	f0 49 0f b1 75 00    	lock cmpxchg %rsi,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    35bd:	48 39 c2             	cmp    %rax,%rdx
    35c0:	0f 84 a2 fb ff ff    	je     3168 <__scif_accept+0x658>
    35c6:	48 89 c2             	mov    %rax,%rdx
    35c9:	eb dc                	jmp    35a7 <__scif_accept+0xa97>
    35cb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    35d0:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
    35d7:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    35de:	e9 2c fb ff ff       	jmpq   310f <__scif_accept+0x5ff>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    35e3:	48 8b 45 08          	mov    0x8(%rbp),%rax
    35e7:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    35ee:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    35f5:	ba a7 00 00 00       	mov    $0xa7,%edx
    35fa:	44 0f b7 01          	movzwl (%rcx),%r8d
    35fe:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    3605:	4c 8b 8b 00 00 00 00 	mov    0x0(%rbx),%r9
    360c:	48 89 04 24          	mov    %rax,(%rsp)
    3610:	31 c0                	xor    %eax,%eax
    3612:	e8 00 00 00 00       	callq  3617 <__scif_accept+0xb07>
    3617:	48 8b b3 00 00 00 00 	mov    0x0(%rbx),%rsi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    361e:	48 b9 00 00 00 00 00 	movabs $0x8000000000000000,%rcx
    3625:	00 00 80 
    3628:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    362f:	48 39 ce             	cmp    %rcx,%rsi
    3632:	0f 84 e5 fb ff ff    	je     321d <__scif_accept+0x70d>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    3638:	48 8d 56 01          	lea    0x1(%rsi),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    363c:	48 89 f0             	mov    %rsi,%rax
    363f:	f0 48 0f b1 93 00 00 	lock cmpxchg %rdx,0x0(%rbx)
    3646:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    3648:	48 39 c6             	cmp    %rax,%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    364b:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    364e:	0f 84 c9 fb ff ff    	je     321d <__scif_accept+0x70d>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    3654:	48 39 ca             	cmp    %rcx,%rdx
    3657:	0f 84 c0 fb ff ff    	je     321d <__scif_accept+0x70d>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    365d:	48 8d 72 01          	lea    0x1(%rdx),%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    3661:	48 89 d0             	mov    %rdx,%rax
    3664:	f0 49 0f b1 34 24    	lock cmpxchg %rsi,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    366a:	48 39 c2             	cmp    %rax,%rdx
    366d:	0f 84 aa fb ff ff    	je     321d <__scif_accept+0x70d>
    3673:	48 89 c2             	mov    %rax,%rdx
    3676:	eb dc                	jmp    3654 <__scif_accept+0xb44>
		goto scif_accept_error_qpalloc;
	}

	cep->qp_info.qp = (struct micscif_qp *)kzalloc(sizeof(struct micscif_qp), GFP_KERNEL);
	if (!cep->qp_info.qp) {
		printk(KERN_ERR "Port Qp Allocation Failed\n");
    3678:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    367f:	e8 00 00 00 00       	callq  3684 <__scif_accept+0xb74>
    3684:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
		err = -ENOMEM;
    368b:	41 bb f4 ff ff ff    	mov    $0xfffffff4,%r11d
    3691:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
		goto scif_accept_error_qpalloc;
    3698:	e9 8a fa ff ff       	jmpq   3127 <__scif_accept+0x617>
		err = -ENOMEM;
		goto scif_accept_error_qpalloc;
	}

	if ((err = micscif_reserve_dma_chan(cep))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    369d:	89 c1                	mov    %eax,%ecx
    369f:	89 85 38 ff ff ff    	mov    %eax,-0xc8(%rbp)
    36a5:	31 c0                	xor    %eax,%eax
    36a7:	ba 0d 04 00 00       	mov    $0x40d,%edx
    36ac:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    36b3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    36ba:	e8 00 00 00 00       	callq  36bf <__scif_accept+0xbaf>
    36bf:	48 8d 85 74 ff ff ff 	lea    -0x8c(%rbp),%rax
		goto scif_accept_error_qpalloc;
    36c6:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    36cd:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    36d4:	e9 4e fa ff ff       	jmpq   3127 <__scif_accept+0x617>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    36d9:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 36df <__scif_accept+0xbcf>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    36df:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    36e3:	31 c9                	xor    %ecx,%ecx
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    36e5:	31 d2                	xor    %edx,%edx
    36e7:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    36ee:	4c 89 85 20 ff ff ff 	mov    %r8,-0xe0(%rbp)
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    36f5:	49 c7 80 b0 01 00 00 	movq   $0x2,0x1b0(%r8)
    36fc:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    3700:	c7 45 a8 2e 00 00 00 	movl   $0x2e,-0x58(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
    3707:	66 89 45 a0          	mov    %ax,-0x60(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
    370b:	66 89 4d a4          	mov    %cx,-0x5c(%rbp)
			notif_msg.payload[0] = dev->sd_node;
    370f:	41 0f b7 00          	movzwl (%r8),%eax
    3713:	48 89 45 ac          	mov    %rax,-0x54(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    3717:	e8 00 00 00 00       	callq  371c <__scif_accept+0xc0c>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    371c:	4c 8b 85 20 ff ff ff 	mov    -0xe0(%rbp),%r8
    3723:	49 8b 80 b0 01 00 00 	mov    0x1b0(%r8),%rax
    372a:	48 83 f8 02          	cmp    $0x2,%rax
    372e:	0f 84 b5 00 00 00    	je     37e9 <__scif_accept+0xcd9>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    3734:	48 83 f8 03          	cmp    $0x3,%rax
    3738:	0f 85 09 fe ff ff    	jne    3547 <__scif_accept+0xa37>
				dev->sd_state = SCIFDEV_RUNNING;
    373e:	41 c7 40 04 02 00 00 	movl   $0x2,0x4(%r8)
    3745:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    3746:	f0 41 80 a0 8f 01 00 	lock andb $0x7f,0x18f(%r8)
    374d:	00 7f 
    374f:	e9 f3 fd ff ff       	jmpq   3547 <__scif_accept+0xa37>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    3754:	48 89 d8             	mov    %rbx,%rax
    3757:	48 89 da             	mov    %rbx,%rdx
    375a:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    3761:	48 c1 e0 05          	shl    $0x5,%rax
    3765:	48 c1 e2 09          	shl    $0x9,%rdx
    3769:	48 29 c2             	sub    %rax,%rdx
    376c:	48 8d 82 00 00 00 00 	lea    0x0(%rdx),%rax
    3773:	49 89 d7             	mov    %rdx,%r15
    3776:	48 89 c7             	mov    %rax,%rdi
    3779:	48 89 85 30 ff ff ff 	mov    %rax,-0xd0(%rbp)
    3780:	e8 00 00 00 00       	callq  3785 <__scif_accept+0xc75>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    3785:	49 8d 8f 00 00 00 00 	lea    0x0(%r15),%rcx
    378c:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    3793:	8b 41 04             	mov    0x4(%rcx),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    3796:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    3799:	83 fa 01             	cmp    $0x1,%edx
    379c:	76 2c                	jbe    37ca <__scif_accept+0xcba>
    379e:	83 f8 01             	cmp    $0x1,%eax
    37a1:	74 27                	je     37ca <__scif_accept+0xcba>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    37a3:	49 8b 84 24 00 00 00 	mov    0x0(%r12),%rax
    37aa:	00 
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    37ab:	48 85 c0             	test   %rax,%rax
    37ae:	0f 88 f7 00 00 00    	js     38ab <__scif_accept+0xd9b>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    37b4:	4c 89 ef             	mov    %r13,%rdi
    37b7:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    37be:	e8 2d cd ff ff       	callq  4f0 <atomic_long_add.constprop.17>
    37c3:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
bail_out:
		mutex_unlock(&dev->sd_lock);
    37ca:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    37d1:	44 89 9d 38 ff ff ff 	mov    %r11d,-0xc8(%rbp)
    37d8:	e8 00 00 00 00       	callq  37dd <__scif_accept+0xccd>
    37dd:	44 8b 9d 38 ff ff ff 	mov    -0xc8(%rbp),%r11d
    37e4:	e9 7f f9 ff ff       	jmpq   3168 <__scif_accept+0x658>
    37e9:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    37f0:	00 00 
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    37f2:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    37f9:	48 8d 85 48 ff ff ff 	lea    -0xb8(%rbp),%rax
    3800:	48 89 85 00 ff ff ff 	mov    %rax,-0x100(%rbp)
    3807:	48 83 c0 18          	add    $0x18,%rax
    380b:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    3812:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    3819:	49 8d 80 98 01 00 00 	lea    0x198(%r8),%rax
    3820:	48 89 9d e0 fe ff ff 	mov    %rbx,-0x120(%rbp)
    3827:	4c 89 c3             	mov    %r8,%rbx
    382a:	48 c7 85 48 ff ff ff 	movq   $0x0,-0xb8(%rbp)
    3831:	00 00 00 00 
    3835:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
    383c:	00 00 00 00 
    3840:	48 89 85 20 ff ff ff 	mov    %rax,-0xe0(%rbp)
    3847:	eb 0c                	jmp    3855 <__scif_accept+0xd45>
    3849:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    3850:	e8 00 00 00 00       	callq  3855 <__scif_accept+0xd45>
    3855:	48 8b b5 00 ff ff ff 	mov    -0x100(%rbp),%rsi
    385c:	ba 02 00 00 00       	mov    $0x2,%edx
    3861:	48 8b bd 20 ff ff ff 	mov    -0xe0(%rbp),%rdi
    3868:	e8 00 00 00 00       	callq  386d <__scif_accept+0xd5d>
    386d:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    3874:	02 
    3875:	74 d9                	je     3850 <__scif_accept+0xd40>
    3877:	48 8b b5 00 ff ff ff 	mov    -0x100(%rbp),%rsi
    387e:	48 89 9d d8 fe ff ff 	mov    %rbx,-0x128(%rbp)
    3885:	48 8b bd 20 ff ff ff 	mov    -0xe0(%rbp),%rdi
    388c:	48 8b 9d e0 fe ff ff 	mov    -0x120(%rbp),%rbx
    3893:	e8 00 00 00 00       	callq  3898 <__scif_accept+0xd88>
    3898:	4c 8b 85 d8 fe ff ff 	mov    -0x128(%rbp),%r8
    389f:	49 8b 80 b0 01 00 00 	mov    0x1b0(%r8),%rax
    38a6:	e9 89 fe ff ff       	jmpq   3734 <__scif_accept+0xc24>
    38ab:	44 89 9d 20 ff ff ff 	mov    %r11d,-0xe0(%rbp)
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    38b2:	31 d2                	xor    %edx,%edx
    38b4:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    38bb:	48 c7 81 b0 01 00 00 	movq   $0x2,0x1b0(%rcx)
    38c2:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    38c6:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    38ca:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 38d0 <__scif_accept+0xdc0>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    38d0:	c7 45 a8 2e 00 00 00 	movl   $0x2e,-0x58(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
    38d7:	66 89 45 a0          	mov    %ax,-0x60(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
    38db:	31 c0                	xor    %eax,%eax
    38dd:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.payload[0] = dev->sd_node;
    38e1:	0f b7 01             	movzwl (%rcx),%eax
    38e4:	48 89 8d 38 ff ff ff 	mov    %rcx,-0xc8(%rbp)
    38eb:	48 89 45 ac          	mov    %rax,-0x54(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    38ef:	e8 00 00 00 00       	callq  38f4 <__scif_accept+0xde4>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    38f4:	48 8b 8d 38 ff ff ff 	mov    -0xc8(%rbp),%rcx
    38fb:	44 8b 9d 20 ff ff ff 	mov    -0xe0(%rbp),%r11d
    3902:	48 8b 81 b0 01 00 00 	mov    0x1b0(%rcx),%rax
    3909:	48 83 f8 02          	cmp    $0x2,%rax
    390d:	74 34                	je     3943 <__scif_accept+0xe33>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    390f:	48 83 f8 03          	cmp    $0x3,%rax
    3913:	0f 85 9b fe ff ff    	jne    37b4 <__scif_accept+0xca4>
				dev->sd_state = SCIFDEV_RUNNING;
    3919:	48 89 da             	mov    %rbx,%rdx
    391c:	48 89 d8             	mov    %rbx,%rax
    391f:	48 c1 e2 05          	shl    $0x5,%rdx
    3923:	48 c1 e0 09          	shl    $0x9,%rax
    3927:	48 29 d0             	sub    %rdx,%rax
    392a:	c7 80 00 00 00 00 02 	movl   $0x2,0x0(%rax)
    3931:	00 00 00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    3934:	f0 41 80 a4 24 00 00 	lock andb $0x7f,0x0(%r12)
    393b:	00 00 7f 
    393e:	e9 71 fe ff ff       	jmpq   37b4 <__scif_accept+0xca4>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    3943:	4c 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%r14
    394a:	48 c7 85 48 ff ff ff 	movq   $0x0,-0xb8(%rbp)
    3951:	00 00 00 00 
    3955:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    395c:	00 00 
    395e:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    3965:	49 8d 46 18          	lea    0x18(%r14),%rax
    3969:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    3970:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    3977:	49 8d 87 00 00 00 00 	lea    0x0(%r15),%rax
    397e:	49 89 df             	mov    %rbx,%r15
    3981:	48 89 cb             	mov    %rcx,%rbx
    3984:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
    398b:	00 00 00 00 
    398f:	48 89 85 38 ff ff ff 	mov    %rax,-0xc8(%rbp)
    3996:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    399d:	ba 02 00 00 00       	mov    $0x2,%edx
    39a2:	4c 89 f6             	mov    %r14,%rsi
    39a5:	e8 00 00 00 00       	callq  39aa <__scif_accept+0xe9a>
    39aa:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    39b1:	02 
    39b2:	48 8d 83 b0 01 00 00 	lea    0x1b0(%rbx),%rax
    39b9:	75 07                	jne    39c2 <__scif_accept+0xeb2>
    39bb:	e8 00 00 00 00       	callq  39c0 <__scif_accept+0xeb0>
    39c0:	eb d4                	jmp    3996 <__scif_accept+0xe86>
    39c2:	44 8b 9d 20 ff ff ff 	mov    -0xe0(%rbp),%r11d
    39c9:	4c 89 f6             	mov    %r14,%rsi
    39cc:	48 89 85 20 ff ff ff 	mov    %rax,-0xe0(%rbp)
    39d3:	4c 89 fb             	mov    %r15,%rbx
    39d6:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    39dd:	44 89 9d 18 ff ff ff 	mov    %r11d,-0xe8(%rbp)
    39e4:	e8 00 00 00 00       	callq  39e9 <__scif_accept+0xed9>
    39e9:	48 8b 85 20 ff ff ff 	mov    -0xe0(%rbp),%rax
    39f0:	44 8b 9d 18 ff ff ff 	mov    -0xe8(%rbp),%r11d
    39f7:	48 8b 00             	mov    (%rax),%rax
    39fa:	e9 10 ff ff ff       	jmpq   390f <__scif_accept+0xdff>
    39ff:	90                   	nop

0000000000003a00 <scif_accept>:
	return err;
}

int
scif_accept(scif_epd_t epd, struct scif_portID *peer, scif_epd_t *newepd, int flags)
{
    3a00:	55                   	push   %rbp
    3a01:	48 89 e5             	mov    %rsp,%rbp
    3a04:	41 57                	push   %r15
    3a06:	41 56                	push   %r14
    3a08:	41 55                	push   %r13
    3a0a:	41 54                	push   %r12
    3a0c:	53                   	push   %rbx
    3a0d:	48 83 ec 08          	sub    $0x8,%rsp
    3a11:	e8 00 00 00 00       	callq  3a16 <scif_accept+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    3a16:	4c 8d a7 70 01 00 00 	lea    0x170(%rdi),%r12
    3a1d:	48 89 fb             	mov    %rdi,%rbx
    3a20:	49 89 f6             	mov    %rsi,%r14
    3a23:	49 89 d5             	mov    %rdx,%r13
    3a26:	41 89 cf             	mov    %ecx,%r15d
    3a29:	4c 89 e7             	mov    %r12,%rdi
    3a2c:	e8 00 00 00 00       	callq  3a31 <scif_accept+0x31>
	int ret;
	get_kref_count(epd);
	ret = __scif_accept(epd, peer, newepd, flags);
    3a31:	48 89 df             	mov    %rbx,%rdi
    3a34:	44 89 f9             	mov    %r15d,%ecx
    3a37:	4c 89 ea             	mov    %r13,%rdx
    3a3a:	4c 89 f6             	mov    %r14,%rsi
    3a3d:	e8 00 00 00 00       	callq  3a42 <scif_accept+0x42>
	if (ret == 0) {
    3a42:	85 c0                	test   %eax,%eax
int
scif_accept(scif_epd_t epd, struct scif_portID *peer, scif_epd_t *newepd, int flags)
{
	int ret;
	get_kref_count(epd);
	ret = __scif_accept(epd, peer, newepd, flags);
    3a44:	89 c3                	mov    %eax,%ebx
	if (ret == 0) {
    3a46:	75 10                	jne    3a58 <scif_accept+0x58>
		kref_init(&((*newepd)->ref_count));
    3a48:	49 8b 7d 00          	mov    0x0(%r13),%rdi
    3a4c:	48 81 c7 70 01 00 00 	add    $0x170,%rdi
    3a53:	e8 00 00 00 00       	callq  3a58 <scif_accept+0x58>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    3a58:	4c 89 e7             	mov    %r12,%rdi
    3a5b:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    3a62:	e8 00 00 00 00       	callq  3a67 <scif_accept+0x67>
	}
	put_kref_count(epd);
	return ret;
}
    3a67:	48 83 c4 08          	add    $0x8,%rsp
    3a6b:	89 d8                	mov    %ebx,%eax
    3a6d:	5b                   	pop    %rbx
    3a6e:	41 5c                	pop    %r12
    3a70:	41 5d                	pop    %r13
    3a72:	41 5e                	pop    %r14
    3a74:	41 5f                	pop    %r15
    3a76:	5d                   	pop    %rbp
    3a77:	c3                   	retq   
    3a78:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    3a7f:	00 

0000000000003a80 <_scif_send>:
 *
 * This function may be interrupted by a signal and will return -EINTR.
 */
int
_scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
    3a80:	55                   	push   %rbp
    3a81:	48 89 e5             	mov    %rsp,%rbp
    3a84:	41 57                	push   %r15
    3a86:	41 56                	push   %r14
    3a88:	41 55                	push   %r13
    3a8a:	41 54                	push   %r12
    3a8c:	53                   	push   %rbx
    3a8d:	48 81 ec 98 00 00 00 	sub    $0x98,%rsp
    3a94:	e8 00 00 00 00       	callq  3a99 <_scif_send+0x19>
	int ret;
#ifdef SCIF_BLAST
	int tl;
#endif

	if (flags & SCIF_SEND_BLOCK)
    3a99:	83 e1 01             	and    $0x1,%ecx
 *
 * This function may be interrupted by a signal and will return -EINTR.
 */
int
_scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
    3a9c:	49 89 fe             	mov    %rdi,%r14
    3a9f:	48 89 b5 58 ff ff ff 	mov    %rsi,-0xa8(%rbp)
    3aa6:	89 95 60 ff ff ff    	mov    %edx,-0xa0(%rbp)
	int ret;
#ifdef SCIF_BLAST
	int tl;
#endif

	if (flags & SCIF_SEND_BLOCK)
    3aac:	89 8d 64 ff ff ff    	mov    %ecx,-0x9c(%rbp)
    3ab2:	0f 85 a8 02 00 00    	jne    3d60 <_scif_send+0x2e0>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    3ab8:	4d 8d 66 04          	lea    0x4(%r14),%r12
{
	struct endpt *ep = (struct endpt *)epd;
	struct nodemsg notif_msg;
	unsigned long sflags;
	size_t curr_xfer_len = 0;
	size_t sent_len = 0;
    3abc:	31 db                	xor    %ebx,%ebx
	} else {
		tl = 1;
		spin_lock_irqsave(&ep->lock, sflags);
	}
#else
	spin_lock_irqsave(&ep->lock, sflags);
    3abe:	4c 89 e7             	mov    %r12,%rdi
    3ac1:	e8 00 00 00 00       	callq  3ac6 <_scif_send+0x46>
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3ac6:	48 8d 8d 78 ff ff ff 	lea    -0x88(%rbp),%rcx
	} else {
		tl = 1;
		spin_lock_irqsave(&ep->lock, sflags);
	}
#else
	spin_lock_irqsave(&ep->lock, sflags);
    3acd:	49 89 c5             	mov    %rax,%r13
    3ad0:	48 63 85 60 ff ff ff 	movslq -0xa0(%rbp),%rax
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3ad7:	48 83 c1 18          	add    $0x18,%rcx
    3adb:	48 89 8d 40 ff ff ff 	mov    %rcx,-0xc0(%rbp)
    3ae2:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    3ae9:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    3af0:	00 00 
    3af2:	48 89 85 48 ff ff ff 	mov    %rax,-0xb8(%rbp)
    3af9:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
	}
#else
	spin_lock_irqsave(&ep->lock, sflags);
#endif

	while (sent_len != len) {
    3b00:	48 3b 9d 68 ff ff ff 	cmp    -0x98(%rbp),%rbx
    3b07:	0f 84 63 02 00 00    	je     3d70 <_scif_send+0x2f0>
		if (ep->state == SCIFEP_DISCONNECTED) {
    3b0d:	41 8b 16             	mov    (%r14),%edx
    3b10:	83 fa 09             	cmp    $0x9,%edx
    3b13:	0f 84 67 02 00 00    	je     3d80 <_scif_send+0x300>
			ret = (int)(sent_len ? sent_len : -ECONNRESET);
			goto unlock_dec_return;
		}
		if (ep->state != SCIFEP_CONNECTED) {
    3b19:	41 8b 16             	mov    (%r14),%edx
    3b1c:	83 fa 04             	cmp    $0x4,%edx
    3b1f:	0f 85 73 02 00 00    	jne    3d98 <_scif_send+0x318>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    3b25:	49 8b 86 48 01 00 00 	mov    0x148(%r14),%rax
    3b2c:	8b 50 04             	mov    0x4(%rax),%edx
    3b2f:	83 ea 02             	sub    $0x2,%edx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    3b32:	83 fa 01             	cmp    $0x1,%edx
    3b35:	76 39                	jbe    3b70 <_scif_send+0xf0>
			ret = (int)(sent_len ? sent_len : -ENOTCONN);
			goto unlock_dec_return;
		}
		if (!scifdev_alive(ep)) {
			ret = (int) (sent_len ? sent_len : -ENODEV);
    3b37:	ba ed ff ff ff       	mov    $0xffffffed,%edx
    3b3c:	48 85 db             	test   %rbx,%rbx
    3b3f:	89 d0                	mov    %edx,%eax
    3b41:	0f 45 c3             	cmovne %ebx,%eax
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    3b44:	4c 89 ee             	mov    %r13,%rsi
    3b47:	4c 89 e7             	mov    %r12,%rdi
    3b4a:	89 85 68 ff ff ff    	mov    %eax,-0x98(%rbp)
    3b50:	e8 00 00 00 00       	callq  3b55 <_scif_send+0xd5>
    3b55:	8b 85 68 ff ff ff    	mov    -0x98(%rbp),%eax
	if (tl)
#endif
	spin_unlock_irqrestore(&ep->lock, sflags);
dec_return:
	return ret;
}
    3b5b:	48 81 c4 98 00 00 00 	add    $0x98,%rsp
    3b62:	5b                   	pop    %rbx
    3b63:	41 5c                	pop    %r12
    3b65:	41 5d                	pop    %r13
    3b67:	41 5e                	pop    %r14
    3b69:	41 5f                	pop    %r15
    3b6b:	5d                   	pop    %rbp
    3b6c:	c3                   	retq   
    3b6d:	0f 1f 00             	nopl   (%rax)
    3b70:	41 83 be 5c 01 00 00 	cmpl   $0x2,0x15c(%r14)
    3b77:	02 
    3b78:	75 bd                	jne    3b37 <_scif_send+0xb7>
		}
		if (!scifdev_alive(ep)) {
			ret = (int) (sent_len ? sent_len : -ENODEV);
			goto unlock_dec_return;
		}
		write_count = micscif_rb_space(&ep->qp_info.qp->outbound_q);
    3b7a:	49 8b 46 10          	mov    0x10(%r14),%rax
    3b7e:	48 8d 78 18          	lea    0x18(%rax),%rdi
    3b82:	e8 00 00 00 00       	callq  3b87 <_scif_send+0x107>
    3b87:	48 98                	cltq   
		if (write_count) {
    3b89:	48 85 c0             	test   %rax,%rax
    3b8c:	75 3a                	jne    3bc8 <_scif_send+0x148>
		}
		curr_xfer_len = min(len - sent_len, (size_t)(ENDPT_QP_SIZE - 1));
		/*
		 * Not enough space in the RB. Return in the Non Blocking case.
		 */
		if (!(flags & SCIF_SEND_BLOCK)) {
    3b8e:	8b 85 64 ff ff ff    	mov    -0x9c(%rbp),%eax
    3b94:	85 c0                	test   %eax,%eax
    3b96:	0f 84 14 02 00 00    	je     3db0 <_scif_send+0x330>
    3b9c:	4c 89 ee             	mov    %r13,%rsi
    3b9f:	4c 89 e7             	mov    %r12,%rdi
    3ba2:	e8 00 00 00 00       	callq  3ba7 <_scif_send+0x127>
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3ba7:	41 8b 06             	mov    (%r14),%eax
    3baa:	83 f8 04             	cmp    $0x4,%eax
    3bad:	0f 84 bd 00 00 00    	je     3c70 <_scif_send+0x1f0>
			(micscif_rb_space(&ep->qp_info.qp->outbound_q)
				>= curr_xfer_len) || (!scifdev_alive(ep))))) {
			ret = (int) (sent_len ? sent_len : ret);
			goto dec_return;
		}
		spin_lock_irqsave(&ep->lock, sflags);
    3bb3:	4c 89 e7             	mov    %r12,%rdi
    3bb6:	e8 00 00 00 00       	callq  3bbb <_scif_send+0x13b>
    3bbb:	49 89 c5             	mov    %rax,%r13
    3bbe:	e9 3d ff ff ff       	jmpq   3b00 <_scif_send+0x80>
    3bc3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
			/*
			 * Best effort to send as much data as there
			 * is space in the RB particularly important for the
			 * Non Blocking case.
			 */
			curr_xfer_len = min(len - sent_len, write_count);
    3bc8:	48 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%rdx
			ret = micscif_rb_write(&ep->qp_info.qp->outbound_q, msg,
    3bcf:	48 8b b5 58 ff ff ff 	mov    -0xa8(%rbp),%rsi
			/*
			 * Best effort to send as much data as there
			 * is space in the RB particularly important for the
			 * Non Blocking case.
			 */
			curr_xfer_len = min(len - sent_len, write_count);
    3bd6:	48 29 da             	sub    %rbx,%rdx
    3bd9:	48 39 d0             	cmp    %rdx,%rax
    3bdc:	48 0f 46 d0          	cmovbe %rax,%rdx
			ret = micscif_rb_write(&ep->qp_info.qp->outbound_q, msg,
    3be0:	49 8b 46 10          	mov    0x10(%r14),%rax
			/*
			 * Best effort to send as much data as there
			 * is space in the RB particularly important for the
			 * Non Blocking case.
			 */
			curr_xfer_len = min(len - sent_len, write_count);
    3be4:	49 89 d7             	mov    %rdx,%r15
			ret = micscif_rb_write(&ep->qp_info.qp->outbound_q, msg,
    3be7:	48 8d 78 18          	lea    0x18(%rax),%rdi
    3beb:	e8 00 00 00 00       	callq  3bf0 <_scif_send+0x170>
						(uint32_t)curr_xfer_len);
			if (ret < 0) {
    3bf0:	85 c0                	test   %eax,%eax
    3bf2:	0f 88 f3 01 00 00    	js     3deb <_scif_send+0x36b>
				ret = -EFAULT;
				goto unlock_dec_return;
			}
			if (ret) {
    3bf8:	74 16                	je     3c10 <_scif_send+0x190>
    3bfa:	4c 89 ee             	mov    %r13,%rsi
    3bfd:	4c 89 e7             	mov    %r12,%rdi
    3c00:	e8 00 00 00 00       	callq  3c05 <_scif_send+0x185>
				 * If there is space in the RB and we have the
				 * EP lock held then writing to the RB should
				 * succeed. Releasing spin lock before asserting
				 * to avoid deadlocking the system.
				 */
				BUG_ON(ret);
    3c05:	0f 0b                	ud2    
    3c07:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    3c0e:	00 00 
			}
			/*
			 * Success. Update write pointer.
			 */
			micscif_rb_commit(&ep->qp_info.qp->outbound_q);
    3c10:	49 8b 46 10          	mov    0x10(%r14),%rax
    3c14:	48 8d 78 18          	lea    0x18(%rax),%rdi
    3c18:	e8 00 00 00 00       	callq  3c1d <_scif_send+0x19d>
#else
			/*
			 * Send a notification to the peer about the
			 * produced data message.
			 */
			notif_msg.src = ep->port;
    3c1d:	41 8b 46 06          	mov    0x6(%r14),%eax
			notif_msg.uop = SCIF_CLIENT_SENT;
			notif_msg.payload[0] = ep->remote_ep;
			if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
    3c21:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    3c25:	4c 89 f2             	mov    %r14,%rdx
    3c28:	49 8b be 48 01 00 00 	mov    0x148(%r14),%rdi
			/*
			 * Send a notification to the peer about the
			 * produced data message.
			 */
			notif_msg.src = ep->port;
			notif_msg.uop = SCIF_CLIENT_SENT;
    3c2f:	c7 45 ac 19 00 00 00 	movl   $0x19,-0x54(%rbp)
#else
			/*
			 * Send a notification to the peer about the
			 * produced data message.
			 */
			notif_msg.src = ep->port;
    3c36:	89 45 a4             	mov    %eax,-0x5c(%rbp)
			notif_msg.uop = SCIF_CLIENT_SENT;
			notif_msg.payload[0] = ep->remote_ep;
    3c39:	49 8b 86 50 01 00 00 	mov    0x150(%r14),%rax
    3c40:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
    3c44:	e8 00 00 00 00       	callq  3c49 <_scif_send+0x1c9>
    3c49:	85 c0                	test   %eax,%eax
    3c4b:	74 13                	je     3c60 <_scif_send+0x1e0>
				ret = (int)(sent_len ? sent_len : ret);
    3c4d:	48 85 db             	test   %rbx,%rbx
    3c50:	0f 45 c3             	cmovne %ebx,%eax
    3c53:	e9 ec fe ff ff       	jmpq   3b44 <_scif_send+0xc4>
    3c58:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    3c5f:	00 
				goto unlock_dec_return;
			}
#endif
			sent_len += curr_xfer_len;
			msg = (char *)msg + curr_xfer_len;
    3c60:	4c 01 bd 58 ff ff ff 	add    %r15,-0xa8(%rbp)
			if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
				ret = (int)(sent_len ? sent_len : ret);
				goto unlock_dec_return;
			}
#endif
			sent_len += curr_xfer_len;
    3c67:	4c 01 fb             	add    %r15,%rbx
			msg = (char *)msg + curr_xfer_len;
			continue;
    3c6a:	e9 91 fe ff ff       	jmpq   3b00 <_scif_send+0x80>
    3c6f:	90                   	nop
		}
		curr_xfer_len = min(len - sent_len, (size_t)(ENDPT_QP_SIZE - 1));
    3c70:	4c 8b ad 68 ff ff ff 	mov    -0x98(%rbp),%r13
    3c77:	b8 ff 0f 00 00       	mov    $0xfff,%eax
    3c7c:	49 29 dd             	sub    %rbx,%r13
    3c7f:	49 81 fd ff 0f 00 00 	cmp    $0xfff,%r13
    3c86:	4c 0f 47 e8          	cmova  %rax,%r13
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3c8a:	49 8b 46 10          	mov    0x10(%r14),%rax
    3c8e:	48 8d 78 18          	lea    0x18(%rax),%rdi
    3c92:	e8 00 00 00 00       	callq  3c97 <_scif_send+0x217>
    3c97:	48 98                	cltq   
    3c99:	49 39 c5             	cmp    %rax,%r13
    3c9c:	0f 86 11 ff ff ff    	jbe    3bb3 <_scif_send+0x133>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    3ca2:	49 8b 86 48 01 00 00 	mov    0x148(%r14),%rax
    3ca9:	8b 40 04             	mov    0x4(%rax),%eax
    3cac:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    3caf:	83 f8 01             	cmp    $0x1,%eax
    3cb2:	0f 87 fb fe ff ff    	ja     3bb3 <_scif_send+0x133>
    3cb8:	41 83 be 5c 01 00 00 	cmpl   $0x2,0x15c(%r14)
    3cbf:	02 
    3cc0:	0f 85 ed fe ff ff    	jne    3bb3 <_scif_send+0x133>
    3cc6:	48 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%rax
    3ccd:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    3cd4:	00 
    3cd5:	4d 8d be d0 01 00 00 	lea    0x1d0(%r14),%r15
    3cdc:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    3ce3:	00 00 00 00 
    3ce7:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    3ceb:	48 8b 85 40 ff ff ff 	mov    -0xc0(%rbp),%rax
    3cf2:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    3cf6:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    3cfa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    3d00:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    3d07:	ba 01 00 00 00       	mov    $0x1,%edx
    3d0c:	4c 89 ff             	mov    %r15,%rdi
    3d0f:	e8 00 00 00 00       	callq  3d14 <_scif_send+0x294>
    3d14:	41 8b 06             	mov    (%r14),%eax
    3d17:	83 f8 04             	cmp    $0x4,%eax
    3d1a:	75 26                	jne    3d42 <_scif_send+0x2c2>
    3d1c:	49 8b 46 10          	mov    0x10(%r14),%rax
    3d20:	48 8d 78 18          	lea    0x18(%rax),%rdi
    3d24:	e8 00 00 00 00       	callq  3d29 <_scif_send+0x2a9>
    3d29:	48 98                	cltq   
    3d2b:	49 39 c5             	cmp    %rax,%r13
    3d2e:	76 12                	jbe    3d42 <_scif_send+0x2c2>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    3d30:	49 8b 86 48 01 00 00 	mov    0x148(%r14),%rax
    3d37:	8b 40 04             	mov    0x4(%rax),%eax
    3d3a:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    3d3d:	83 f8 01             	cmp    $0x1,%eax
    3d40:	76 7e                	jbe    3dc0 <_scif_send+0x340>
    3d42:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    3d49:	4c 89 ff             	mov    %r15,%rdi
    3d4c:	e8 00 00 00 00       	callq  3d51 <_scif_send+0x2d1>
    3d51:	e9 5d fe ff ff       	jmpq   3bb3 <_scif_send+0x133>
    3d56:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    3d5d:	00 00 00 
#ifdef SCIF_BLAST
	int tl;
#endif

	if (flags & SCIF_SEND_BLOCK)
		might_sleep();
    3d60:	e8 00 00 00 00       	callq  3d65 <_scif_send+0x2e5>
    3d65:	e9 4e fd ff ff       	jmpq   3ab8 <_scif_send+0x38>
    3d6a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
			ret = (int) (sent_len ? sent_len : ret);
			goto dec_return;
		}
		spin_lock_irqsave(&ep->lock, sflags);
	}
	ret = len;
    3d70:	8b 85 60 ff ff ff    	mov    -0xa0(%rbp),%eax
    3d76:	e9 c9 fd ff ff       	jmpq   3b44 <_scif_send+0xc4>
    3d7b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	spin_lock_irqsave(&ep->lock, sflags);
#endif

	while (sent_len != len) {
		if (ep->state == SCIFEP_DISCONNECTED) {
			ret = (int)(sent_len ? sent_len : -ECONNRESET);
    3d80:	ba 98 ff ff ff       	mov    $0xffffff98,%edx
    3d85:	48 85 db             	test   %rbx,%rbx
    3d88:	89 d0                	mov    %edx,%eax
    3d8a:	0f 45 c3             	cmovne %ebx,%eax
    3d8d:	e9 b2 fd ff ff       	jmpq   3b44 <_scif_send+0xc4>
    3d92:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
			goto unlock_dec_return;
		}
		if (ep->state != SCIFEP_CONNECTED) {
			ret = (int)(sent_len ? sent_len : -ENOTCONN);
    3d98:	ba 95 ff ff ff       	mov    $0xffffff95,%edx
    3d9d:	48 85 db             	test   %rbx,%rbx
    3da0:	89 d0                	mov    %edx,%eax
    3da2:	0f 45 c3             	cmovne %ebx,%eax
    3da5:	e9 9a fd ff ff       	jmpq   3b44 <_scif_send+0xc4>
    3daa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		curr_xfer_len = min(len - sent_len, (size_t)(ENDPT_QP_SIZE - 1));
		/*
		 * Not enough space in the RB. Return in the Non Blocking case.
		 */
		if (!(flags & SCIF_SEND_BLOCK)) {
			ret = (int)sent_len;
    3db0:	89 d8                	mov    %ebx,%eax
			goto unlock_dec_return;
    3db2:	e9 8d fd ff ff       	jmpq   3b44 <_scif_send+0xc4>
    3db7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    3dbe:	00 00 
    3dc0:	41 83 be 5c 01 00 00 	cmpl   $0x2,0x15c(%r14)
    3dc7:	02 
    3dc8:	0f 85 74 ff ff ff    	jne    3d42 <_scif_send+0x2c2>
    3dce:	48 8b 85 50 ff ff ff 	mov    -0xb0(%rbp),%rax
    3dd5:	48 8b 40 08          	mov    0x8(%rax),%rax
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    3dd9:	48 8b 40 10          	mov    0x10(%rax),%rax
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3ddd:	a8 04                	test   $0x4,%al
    3ddf:	75 14                	jne    3df5 <_scif_send+0x375>
    3de1:	e8 00 00 00 00       	callq  3de6 <_scif_send+0x366>
    3de6:	e9 15 ff ff ff       	jmpq   3d00 <_scif_send+0x280>
			 */
			curr_xfer_len = min(len - sent_len, write_count);
			ret = micscif_rb_write(&ep->qp_info.qp->outbound_q, msg,
						(uint32_t)curr_xfer_len);
			if (ret < 0) {
				ret = -EFAULT;
    3deb:	b8 f2 ff ff ff       	mov    $0xfffffff2,%eax
    3df0:	e9 4f fd ff ff       	jmpq   3b44 <_scif_send+0xc4>
#endif
		spin_unlock_irqrestore(&ep->lock, sflags);
		/*
		 * Wait for a message now in the Blocking case.
		 */
		if ((ret = wait_event_interruptible(ep->sendwq, 
    3df5:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    3dfc:	4c 89 ff             	mov    %r15,%rdi
    3dff:	e8 00 00 00 00       	callq  3e04 <_scif_send+0x384>
    3e04:	ba 00 fe ff ff       	mov    $0xfffffe00,%edx
    3e09:	48 85 db             	test   %rbx,%rbx
    3e0c:	89 d0                	mov    %edx,%eax
    3e0e:	0f 45 c3             	cmovne %ebx,%eax
    3e11:	e9 45 fd ff ff       	jmpq   3b5b <_scif_send+0xdb>
    3e16:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    3e1d:	00 00 00 

0000000000003e20 <_scif_recv>:
 *
 * This function may be interrupted by a signal and will return -EINTR.
 */
int
_scif_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    3e20:	55                   	push   %rbp
    3e21:	48 89 e5             	mov    %rsp,%rbp
    3e24:	41 57                	push   %r15
    3e26:	41 56                	push   %r14
    3e28:	41 55                	push   %r13
    3e2a:	41 54                	push   %r12
    3e2c:	53                   	push   %rbx
    3e2d:	48 81 ec e8 00 00 00 	sub    $0xe8,%rsp
    3e34:	e8 00 00 00 00       	callq  3e39 <_scif_recv+0x19>
    3e39:	48 63 c2             	movslq %edx,%rax
	size_t curr_recv_len = 0;
	size_t remaining_len = len;
	size_t read_count;
	int ret;

	if (flags & SCIF_RECV_BLOCK)
    3e3c:	83 e1 01             	and    $0x1,%ecx
 *
 * This function may be interrupted by a signal and will return -EINTR.
 */
int
_scif_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    3e3f:	49 89 fc             	mov    %rdi,%r12
    3e42:	48 89 b5 28 ff ff ff 	mov    %rsi,-0xd8(%rbp)
    3e49:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
	int read_size;
	struct endpt *ep = (struct endpt *)epd;
	unsigned long sflags;
	struct nodemsg notif_msg;
	size_t curr_recv_len = 0;
	size_t remaining_len = len;
    3e4f:	48 89 85 20 ff ff ff 	mov    %rax,-0xe0(%rbp)
	size_t read_count;
	int ret;

	if (flags & SCIF_RECV_BLOCK)
    3e56:	89 8d 34 ff ff ff    	mov    %ecx,-0xcc(%rbp)
    3e5c:	0f 85 86 04 00 00    	jne    42e8 <_scif_recv+0x4c8>
		might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    3e62:	4d 8b ac 24 48 01 00 	mov    0x148(%r12),%r13
    3e69:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    3e6a:	4d 85 ed             	test   %r13,%r13
    3e6d:	0f 85 8b 04 00 00    	jne    42fe <_scif_recv+0x4de>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    3e73:	49 8d 44 24 04       	lea    0x4(%r12),%rax
	spin_lock_irqsave(&ep->lock, sflags);
    3e78:	48 89 c7             	mov    %rax,%rdi
    3e7b:	48 89 85 38 ff ff ff 	mov    %rax,-0xc8(%rbp)
    3e82:	e8 00 00 00 00       	callq  3e87 <_scif_recv+0x67>
    3e87:	49 89 c5             	mov    %rax,%r13
	while (remaining_len) {
    3e8a:	48 8b 85 20 ff ff ff 	mov    -0xe0(%rbp),%rax
    3e91:	48 85 c0             	test   %rax,%rax
    3e94:	0f 84 16 01 00 00    	je     3fb0 <_scif_recv+0x190>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    3e9a:	48 8d 8d 48 ff ff ff 	lea    -0xb8(%rbp),%rcx
    3ea1:	48 89 c3             	mov    %rax,%rbx
    3ea4:	48 83 c1 18          	add    $0x18,%rcx
    3ea8:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    3eaf:	00 00 
    3eb1:	48 89 8d 08 ff ff ff 	mov    %rcx,-0xf8(%rbp)
    3eb8:	48 89 85 10 ff ff ff 	mov    %rax,-0xf0(%rbp)
    3ebf:	48 89 85 18 ff ff ff 	mov    %rax,-0xe8(%rbp)
		if (ep->state != SCIFEP_CONNECTED &&
    3ec6:	41 8b 04 24          	mov    (%r12),%eax
    3eca:	83 f8 04             	cmp    $0x4,%eax
    3ecd:	74 0d                	je     3edc <_scif_recv+0xbc>
			ep->state != SCIFEP_DISCONNECTED) {
    3ecf:	41 8b 04 24          	mov    (%r12),%eax
		might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);
	while (remaining_len) {
		if (ep->state != SCIFEP_CONNECTED &&
    3ed3:	83 f8 09             	cmp    $0x9,%eax
    3ed6:	0f 85 ba 00 00 00    	jne    3f96 <_scif_recv+0x176>
			ep->state != SCIFEP_DISCONNECTED) {
			ret = (int) (len - remaining_len) ?
				(int) (len - remaining_len) : -ENOTCONN;
			goto unlock_dec_return;
		}
		read_count = micscif_rb_count(&ep->qp_info.qp->inbound_q,
    3edc:	49 8b 44 24 10       	mov    0x10(%r12),%rax
    3ee1:	89 de                	mov    %ebx,%esi
    3ee3:	48 8d 78 48          	lea    0x48(%rax),%rdi
    3ee7:	e8 00 00 00 00       	callq  3eec <_scif_recv+0xcc>
    3eec:	89 c2                	mov    %eax,%edx
					(int) remaining_len);
		if (read_count) {
    3eee:	48 85 d2             	test   %rdx,%rdx
    3ef1:	0f 85 11 01 00 00    	jne    4008 <_scif_recv+0x1e8>
		curr_recv_len = min(remaining_len, (size_t)(ENDPT_QP_SIZE - 1));
		/*
		 * Bail out now if the EP is in SCIFEP_DISCONNECTED state else
		 * we will keep looping forever.
		 */
		if (ep->state == SCIFEP_DISCONNECTED) {
    3ef7:	41 8b 04 24          	mov    (%r12),%eax
    3efb:	83 f8 09             	cmp    $0x9,%eax
    3efe:	0f 84 9c 04 00 00    	je     43a0 <_scif_recv+0x580>
		}
		/*
		 * Return in the Non Blocking case if there is no data
		 * to read in this iteration.
		 */
		if (!(flags & SCIF_RECV_BLOCK)) {
    3f04:	8b 95 34 ff ff ff    	mov    -0xcc(%rbp),%edx
    3f0a:	85 d2                	test   %edx,%edx
    3f0c:	0f 84 ae 04 00 00    	je     43c0 <_scif_recv+0x5a0>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    3f12:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    3f19:	4c 89 ee             	mov    %r13,%rsi
    3f1c:	e8 00 00 00 00       	callq  3f21 <_scif_recv+0x101>
			ret = len - (int)remaining_len;
			goto unlock_dec_return;
		}
		spin_unlock_irqrestore(&ep->lock, sflags);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    3f21:	4d 8b ac 24 48 01 00 	mov    0x148(%r12),%r13
    3f28:	00 
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    3f29:	4d 85 ed             	test   %r13,%r13
    3f2c:	74 21                	je     3f4f <_scif_recv+0x12f>
		if (unlikely((atomic_long_sub_return(cnt, 
    3f2e:	4d 8d b5 88 01 00 00 	lea    0x188(%r13),%r14
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    3f35:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    3f3c:	f0 49 0f c1 85 88 01 	lock xadd %rax,0x188(%r13)
    3f43:	00 00 
    3f45:	48 83 e8 01          	sub    $0x1,%rax
    3f49:	0f 88 91 02 00 00    	js     41e0 <_scif_recv+0x3c0>
		/*
		 * Wait for a message now in the Blocking case.
		 * or until other side disconnects.
		 */
		if ((ret = wait_event_interruptible(ep->recvwq, 
    3f4f:	41 8b 04 24          	mov    (%r12),%eax
    3f53:	83 f8 04             	cmp    $0x4,%eax
    3f56:	0f 84 1c 01 00 00    	je     4078 <_scif_recv+0x258>
    3f5c:	4d 8b bc 24 48 01 00 	mov    0x148(%r12),%r15
    3f63:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    3f64:	4d 85 ff             	test   %r15,%r15
    3f67:	0f 85 0b 02 00 00    	jne    4178 <_scif_recv+0x358>
			ret = (len - remaining_len) ?
				(len - (int)remaining_len) : ret;
			goto dec_return;
		}
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		spin_lock_irqsave(&ep->lock, sflags);
    3f6d:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    3f74:	e8 00 00 00 00       	callq  3f79 <_scif_recv+0x159>
    3f79:	49 89 c5             	mov    %rax,%r13
		might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);
	while (remaining_len) {
		if (ep->state != SCIFEP_CONNECTED &&
    3f7c:	41 8b 04 24          	mov    (%r12),%eax
    3f80:	83 f8 04             	cmp    $0x4,%eax
    3f83:	0f 84 53 ff ff ff    	je     3edc <_scif_recv+0xbc>
			ep->state != SCIFEP_DISCONNECTED) {
    3f89:	41 8b 04 24          	mov    (%r12),%eax
		might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);
	while (remaining_len) {
		if (ep->state != SCIFEP_CONNECTED &&
    3f8d:	83 f8 09             	cmp    $0x9,%eax
    3f90:	0f 84 46 ff ff ff    	je     3edc <_scif_recv+0xbc>
			ep->state != SCIFEP_DISCONNECTED) {
			ret = (int) (len - remaining_len) ?
				(int) (len - remaining_len) : -ENOTCONN;
    3f96:	8b 85 30 ff ff ff    	mov    -0xd0(%rbp),%eax
    3f9c:	89 c2                	mov    %eax,%edx
    3f9e:	29 da                	sub    %ebx,%edx
    3fa0:	39 d8                	cmp    %ebx,%eax
    3fa2:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
    3fa7:	0f 45 c2             	cmovne %edx,%eax
    3faa:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    3fb0:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    3fb7:	4c 89 ee             	mov    %r13,%rsi
    3fba:	e8 00 00 00 00       	callq  3fbf <_scif_recv+0x19f>
		spin_lock_irqsave(&ep->lock, sflags);
	}
	ret = len;
unlock_dec_return:
	spin_unlock_irqrestore(&ep->lock, sflags);
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    3fbf:	49 8b 9c 24 48 01 00 	mov    0x148(%r12),%rbx
    3fc6:	00 
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    3fc7:	48 85 db             	test   %rbx,%rbx
    3fca:	74 21                	je     3fed <_scif_recv+0x1cd>
		if (unlikely((atomic_long_sub_return(cnt, 
    3fcc:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
    3fd3:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    3fda:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    3fe1:	00 00 
    3fe3:	48 83 e8 01          	sub    $0x1,%rax
    3fe7:	0f 88 20 04 00 00    	js     440d <_scif_recv+0x5ed>
dec_return:
	return ret;
}
    3fed:	8b 85 30 ff ff ff    	mov    -0xd0(%rbp),%eax
    3ff3:	48 81 c4 e8 00 00 00 	add    $0xe8,%rsp
    3ffa:	5b                   	pop    %rbx
    3ffb:	41 5c                	pop    %r12
    3ffd:	41 5d                	pop    %r13
    3fff:	41 5e                	pop    %r14
    4001:	41 5f                	pop    %r15
    4003:	5d                   	pop    %rbp
    4004:	c3                   	retq   
    4005:	0f 1f 00             	nopl   (%rax)
			 * Best effort to recv as much data as there
			 * are bytes to read in the RB particularly
			 * important for the Non Blocking case.
			 */
			curr_recv_len = min(remaining_len, read_count);
			read_size = micscif_rb_get_next(
    4008:	49 8b 44 24 10       	mov    0x10(%r12),%rax
			/*
			 * Best effort to recv as much data as there
			 * are bytes to read in the RB particularly
			 * important for the Non Blocking case.
			 */
			curr_recv_len = min(remaining_len, read_count);
    400d:	48 39 da             	cmp    %rbx,%rdx
    4010:	48 0f 47 d3          	cmova  %rbx,%rdx
			read_size = micscif_rb_get_next(
    4014:	48 8b b5 28 ff ff ff 	mov    -0xd8(%rbp),%rsi
			/*
			 * Best effort to recv as much data as there
			 * are bytes to read in the RB particularly
			 * important for the Non Blocking case.
			 */
			curr_recv_len = min(remaining_len, read_count);
    401b:	49 89 d6             	mov    %rdx,%r14
			read_size = micscif_rb_get_next(
    401e:	48 8d 78 48          	lea    0x48(%rax),%rdi
    4022:	e8 00 00 00 00       	callq  4027 <_scif_recv+0x207>
					&ep->qp_info.qp->inbound_q,
					msg, (int) curr_recv_len);
			if (read_size < 0){
    4027:	85 c0                	test   %eax,%eax
    4029:	0f 88 9d 04 00 00    	js     44cc <_scif_recv+0x6ac>
				/* only could happen when copy to USER buffer
				*/
				ret = -EFAULT;
				goto unlock_dec_return;
			}
			if (read_size != curr_recv_len) {
    402f:	48 98                	cltq   
    4031:	49 39 c6             	cmp    %rax,%r14
    4034:	74 1a                	je     4050 <_scif_recv+0x230>
    4036:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    403d:	4c 89 ee             	mov    %r13,%rsi
    4040:	e8 00 00 00 00       	callq  4045 <_scif_recv+0x225>
				 * If there are bytes to be read from the RB and
				 * we have the EP lock held then reading from
				 * RB should succeed. Releasing spin lock before
				 * asserting to avoid deadlocking the system.
				 */
				BUG_ON(read_size != curr_recv_len);
    4045:	0f 0b                	ud2    
    4047:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    404e:	00 00 
			}
			if (ep->state == SCIFEP_CONNECTED) {
    4050:	41 8b 04 24          	mov    (%r12),%eax
    4054:	83 f8 04             	cmp    $0x4,%eax
    4057:	0f 84 1b 02 00 00    	je     4278 <_scif_recv+0x458>
						(len - (int)remaining_len) : ret;
					goto unlock_dec_return;
				}
			}
			remaining_len -= curr_recv_len;
			msg = (char *)msg + curr_recv_len;
    405d:	4c 01 b5 28 ff ff ff 	add    %r14,-0xd8(%rbp)
	if (flags & SCIF_RECV_BLOCK)
		might_sleep();

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);
	while (remaining_len) {
    4064:	4c 29 f3             	sub    %r14,%rbx
    4067:	0f 85 59 fe ff ff    	jne    3ec6 <_scif_recv+0xa6>
    406d:	e9 3e ff ff ff       	jmpq   3fb0 <_scif_recv+0x190>
    4072:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		/*
		 * Wait for a message now in the Blocking case.
		 * or until other side disconnects.
		 */
		if ((ret = wait_event_interruptible(ep->recvwq, 
    4078:	49 8b 44 24 10       	mov    0x10(%r12),%rax
			}
			remaining_len -= curr_recv_len;
			msg = (char *)msg + curr_recv_len;
			continue;
		}
		curr_recv_len = min(remaining_len, (size_t)(ENDPT_QP_SIZE - 1));
    407d:	48 81 fb ff 0f 00 00 	cmp    $0xfff,%rbx
    4084:	41 bd ff 0f 00 00    	mov    $0xfff,%r13d
    408a:	4c 0f 46 eb          	cmovbe %rbx,%r13
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		/*
		 * Wait for a message now in the Blocking case.
		 * or until other side disconnects.
		 */
		if ((ret = wait_event_interruptible(ep->recvwq, 
    408e:	44 89 ee             	mov    %r13d,%esi
    4091:	45 89 ee             	mov    %r13d,%r14d
    4094:	48 8d 78 48          	lea    0x48(%rax),%rdi
    4098:	e8 00 00 00 00       	callq  409d <_scif_recv+0x27d>
    409d:	89 c0                	mov    %eax,%eax
    409f:	49 39 c5             	cmp    %rax,%r13
    40a2:	0f 86 b4 fe ff ff    	jbe    3f5c <_scif_recv+0x13c>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    40a8:	4d 8b bc 24 48 01 00 	mov    0x148(%r12),%r15
    40af:	00 
    40b0:	41 8b 47 04          	mov    0x4(%r15),%eax
    40b4:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    40b7:	83 f8 01             	cmp    $0x1,%eax
    40ba:	0f 87 a4 fe ff ff    	ja     3f64 <_scif_recv+0x144>
    40c0:	41 83 bc 24 5c 01 00 	cmpl   $0x2,0x15c(%r12)
    40c7:	00 02 
    40c9:	0f 85 95 fe ff ff    	jne    3f64 <_scif_recv+0x144>
    40cf:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    40d6:	48 c7 45 a0 00 00 00 	movq   $0x0,-0x60(%rbp)
    40dd:	00 
    40de:	4d 8d bc 24 e8 01 00 	lea    0x1e8(%r12),%r15
    40e5:	00 
    40e6:	48 c7 45 b0 00 00 00 	movq   $0x0,-0x50(%rbp)
    40ed:	00 
    40ee:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    40f2:	48 8d 45 a0          	lea    -0x60(%rbp),%rax
    40f6:	48 83 c0 18          	add    $0x18,%rax
    40fa:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    40fe:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    4102:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    4109:	00 00 
    410b:	48 89 85 00 ff ff ff 	mov    %rax,-0x100(%rbp)
    4112:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    4118:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    411c:	ba 01 00 00 00       	mov    $0x1,%edx
    4121:	4c 89 ff             	mov    %r15,%rdi
    4124:	e8 00 00 00 00       	callq  4129 <_scif_recv+0x309>
    4129:	41 8b 04 24          	mov    (%r12),%eax
    412d:	83 f8 04             	cmp    $0x4,%eax
    4130:	75 2f                	jne    4161 <_scif_recv+0x341>
    4132:	49 8b 44 24 10       	mov    0x10(%r12),%rax
    4137:	44 89 f6             	mov    %r14d,%esi
    413a:	48 8d 78 48          	lea    0x48(%rax),%rdi
    413e:	e8 00 00 00 00       	callq  4143 <_scif_recv+0x323>
    4143:	89 c0                	mov    %eax,%eax
    4145:	49 39 c5             	cmp    %rax,%r13
    4148:	76 17                	jbe    4161 <_scif_recv+0x341>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    414a:	49 8b 84 24 48 01 00 	mov    0x148(%r12),%rax
    4151:	00 
    4152:	8b 40 04             	mov    0x4(%rax),%eax
    4155:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    4158:	83 f8 01             	cmp    $0x1,%eax
    415b:	0f 86 3f 03 00 00    	jbe    44a0 <_scif_recv+0x680>
    4161:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    4165:	4c 89 ff             	mov    %r15,%rdi
    4168:	e8 00 00 00 00       	callq  416d <_scif_recv+0x34d>
    416d:	e9 ea fd ff ff       	jmpq   3f5c <_scif_recv+0x13c>
    4172:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4178:	49 8b 8f 88 01 00 00 	mov    0x188(%r15),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    417f:	4d 8d b7 88 01 00 00 	lea    0x188(%r15),%r14
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4186:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    418d:	00 00 80 
    4190:	48 39 f1             	cmp    %rsi,%rcx
    4193:	0f 84 32 02 00 00    	je     43cb <_scif_recv+0x5ab>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4199:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    419d:	48 89 c8             	mov    %rcx,%rax
    41a0:	f0 49 0f b1 97 88 01 	lock cmpxchg %rdx,0x188(%r15)
    41a7:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    41a9:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    41ac:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    41af:	0f 84 b8 fd ff ff    	je     3f6d <_scif_recv+0x14d>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    41b5:	48 39 f2             	cmp    %rsi,%rdx
    41b8:	0f 84 0d 02 00 00    	je     43cb <_scif_recv+0x5ab>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    41be:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    41c2:	48 89 d0             	mov    %rdx,%rax
    41c5:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    41ca:	48 39 c2             	cmp    %rax,%rdx
    41cd:	0f 84 9a fd ff ff    	je     3f6d <_scif_recv+0x14d>
    41d3:	48 89 c2             	mov    %rax,%rdx
    41d6:	eb dd                	jmp    41b5 <_scif_recv+0x395>
    41d8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    41df:	00 
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    41e0:	48 8b 45 08          	mov    0x8(%rbp),%rax
    41e4:	4c 89 e9             	mov    %r13,%rcx
    41e7:	ba a7 00 00 00       	mov    $0xa7,%edx
    41ec:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    41f3:	4d 8b 8d 88 01 00 00 	mov    0x188(%r13),%r9
    41fa:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    4201:	45 0f b7 45 00       	movzwl 0x0(%r13),%r8d
    4206:	48 89 04 24          	mov    %rax,(%rsp)
    420a:	31 c0                	xor    %eax,%eax
    420c:	e8 00 00 00 00       	callq  4211 <_scif_recv+0x3f1>
    4211:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4218:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    421f:	00 00 80 
    4222:	48 39 f1             	cmp    %rsi,%rcx
    4225:	0f 84 24 fd ff ff    	je     3f4f <_scif_recv+0x12f>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    422b:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    422f:	48 89 c8             	mov    %rcx,%rax
    4232:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    4239:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    423b:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    423e:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4241:	0f 84 08 fd ff ff    	je     3f4f <_scif_recv+0x12f>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4247:	48 39 f2             	cmp    %rsi,%rdx
    424a:	0f 84 ff fc ff ff    	je     3f4f <_scif_recv+0x12f>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4250:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4254:	48 89 d0             	mov    %rdx,%rax
    4257:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    425c:	48 39 c2             	cmp    %rax,%rdx
    425f:	0f 84 ea fc ff ff    	je     3f4f <_scif_recv+0x12f>
    4265:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4268:	48 39 f2             	cmp    %rsi,%rdx
    426b:	75 e3                	jne    4250 <_scif_recv+0x430>
    426d:	e9 dd fc ff ff       	jmpq   3f4f <_scif_recv+0x12f>
    4272:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
				/*
				 * Update the read pointer only if the endpoint is
				 * still connected else the read pointer might no
				 * longer exist since the peer has freed resources!
				 */
				micscif_rb_update_read_ptr(&ep->qp_info.qp->inbound_q);
    4278:	49 8b 44 24 10       	mov    0x10(%r12),%rax
    427d:	48 8d 78 48          	lea    0x48(%rax),%rdi
    4281:	e8 00 00 00 00       	callq  4286 <_scif_recv+0x466>
				/*
				 * Send a notification to the peer about the
				 * consumed data message only if the EP is in
				 * SCIFEP_CONNECTED state.
				 */
				notif_msg.src = ep->port;
    4286:	41 8b 44 24 06       	mov    0x6(%r12),%eax
				notif_msg.uop = SCIF_CLIENT_RCVD;
				notif_msg.payload[0] = ep->remote_ep;
				if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
    428b:	4c 89 e2             	mov    %r12,%rdx
				 * Send a notification to the peer about the
				 * consumed data message only if the EP is in
				 * SCIFEP_CONNECTED state.
				 */
				notif_msg.src = ep->port;
				notif_msg.uop = SCIF_CLIENT_RCVD;
    428e:	c7 85 78 ff ff ff 1a 	movl   $0x1a,-0x88(%rbp)
    4295:	00 00 00 
				notif_msg.payload[0] = ep->remote_ep;
				if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
    4298:	49 8b bc 24 48 01 00 	mov    0x148(%r12),%rdi
    429f:	00 
    42a0:	48 8d b5 70 ff ff ff 	lea    -0x90(%rbp),%rsi
				/*
				 * Send a notification to the peer about the
				 * consumed data message only if the EP is in
				 * SCIFEP_CONNECTED state.
				 */
				notif_msg.src = ep->port;
    42a7:	89 85 70 ff ff ff    	mov    %eax,-0x90(%rbp)
				notif_msg.uop = SCIF_CLIENT_RCVD;
				notif_msg.payload[0] = ep->remote_ep;
    42ad:	49 8b 84 24 50 01 00 	mov    0x150(%r12),%rax
    42b4:	00 
    42b5:	48 89 85 7c ff ff ff 	mov    %rax,-0x84(%rbp)
				if ((ret = micscif_nodeqp_send(ep->remote_dev, &notif_msg, ep))) {
    42bc:	e8 00 00 00 00       	callq  42c1 <_scif_recv+0x4a1>
    42c1:	85 c0                	test   %eax,%eax
    42c3:	0f 84 94 fd ff ff    	je     405d <_scif_recv+0x23d>
					ret = (len - (int)remaining_len) ?
						(len - (int)remaining_len) : ret;
    42c9:	8b 8d 30 ff ff ff    	mov    -0xd0(%rbp),%ecx
    42cf:	89 ca                	mov    %ecx,%edx
    42d1:	29 da                	sub    %ebx,%edx
    42d3:	39 d9                	cmp    %ebx,%ecx
    42d5:	0f 45 c2             	cmovne %edx,%eax
    42d8:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    42de:	e9 cd fc ff ff       	jmpq   3fb0 <_scif_recv+0x190>
    42e3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	size_t remaining_len = len;
	size_t read_count;
	int ret;

	if (flags & SCIF_RECV_BLOCK)
		might_sleep();
    42e8:	e8 00 00 00 00       	callq  42ed <_scif_recv+0x4cd>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    42ed:	4d 8b ac 24 48 01 00 	mov    0x148(%r12),%r13
    42f4:	00 
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    42f5:	4d 85 ed             	test   %r13,%r13
    42f8:	0f 84 75 fb ff ff    	je     3e73 <_scif_recv+0x53>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    42fe:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
    4305:	49 8d bd 88 01 00 00 	lea    0x188(%r13),%rdi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    430c:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    4313:	00 00 80 
    4316:	48 39 f1             	cmp    %rsi,%rcx
    4319:	74 3e                	je     4359 <_scif_recv+0x539>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    431b:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    431f:	48 89 c8             	mov    %rcx,%rax
    4322:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    4329:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    432b:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    432e:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4331:	0f 84 3c fb ff ff    	je     3e73 <_scif_recv+0x53>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4337:	48 39 f2             	cmp    %rsi,%rdx
    433a:	74 1d                	je     4359 <_scif_recv+0x539>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    433c:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4340:	48 89 d0             	mov    %rdx,%rax
    4343:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4348:	48 39 d0             	cmp    %rdx,%rax
    434b:	0f 84 22 fb ff ff    	je     3e73 <_scif_recv+0x53>
    4351:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4354:	48 39 f2             	cmp    %rsi,%rdx
    4357:	75 e3                	jne    433c <_scif_recv+0x51c>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    4359:	4d 8d b5 68 01 00 00 	lea    0x168(%r13),%r14
    4360:	4c 89 f7             	mov    %r14,%rdi
    4363:	e8 00 00 00 00       	callq  4368 <_scif_recv+0x548>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    4368:	41 8b 45 04          	mov    0x4(%r13),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    436c:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    436f:	83 fa 01             	cmp    $0x1,%edx
    4372:	76 1e                	jbe    4392 <_scif_recv+0x572>
    4374:	83 f8 01             	cmp    $0x1,%eax
    4377:	74 19                	je     4392 <_scif_recv+0x572>
    4379:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    4380:	48 85 c0             	test   %rax,%rax
    4383:	0f 88 78 02 00 00    	js     4601 <_scif_recv+0x7e1>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
    4389:	f0 49 83 85 88 01 00 	lock addq $0x1,0x188(%r13)
    4390:	00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    4392:	4c 89 f7             	mov    %r14,%rdi
    4395:	e8 00 00 00 00       	callq  439a <_scif_recv+0x57a>
    439a:	e9 d4 fa ff ff       	jmpq   3e73 <_scif_recv+0x53>
    439f:	90                   	nop
		 * Bail out now if the EP is in SCIFEP_DISCONNECTED state else
		 * we will keep looping forever.
		 */
		if (ep->state == SCIFEP_DISCONNECTED) {
			ret = (len - (int)remaining_len) ?
				(len - (int)remaining_len) : -ECONNRESET;
    43a0:	8b 85 30 ff ff ff    	mov    -0xd0(%rbp),%eax
    43a6:	89 c2                	mov    %eax,%edx
    43a8:	29 da                	sub    %ebx,%edx
    43aa:	39 d8                	cmp    %ebx,%eax
    43ac:	b8 98 ff ff ff       	mov    $0xffffff98,%eax
    43b1:	0f 45 c2             	cmovne %edx,%eax
    43b4:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    43ba:	e9 f1 fb ff ff       	jmpq   3fb0 <_scif_recv+0x190>
    43bf:	90                   	nop
		/*
		 * Return in the Non Blocking case if there is no data
		 * to read in this iteration.
		 */
		if (!(flags & SCIF_RECV_BLOCK)) {
			ret = len - (int)remaining_len;
    43c0:	29 9d 30 ff ff ff    	sub    %ebx,-0xd0(%rbp)
			goto unlock_dec_return;
    43c6:	e9 e5 fb ff ff       	jmpq   3fb0 <_scif_recv+0x190>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    43cb:	4d 8d af 68 01 00 00 	lea    0x168(%r15),%r13
    43d2:	4c 89 ef             	mov    %r13,%rdi
    43d5:	e8 00 00 00 00       	callq  43da <_scif_recv+0x5ba>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    43da:	41 8b 47 04          	mov    0x4(%r15),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    43de:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    43e1:	83 fa 01             	cmp    $0x1,%edx
    43e4:	76 1a                	jbe    4400 <_scif_recv+0x5e0>
    43e6:	83 f8 01             	cmp    $0x1,%eax
    43e9:	74 15                	je     4400 <_scif_recv+0x5e0>
    43eb:	49 8b 87 88 01 00 00 	mov    0x188(%r15),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    43f2:	48 85 c0             	test   %rax,%rax
    43f5:	0f 88 0e 01 00 00    	js     4509 <_scif_recv+0x6e9>
    43fb:	f0 49 83 06 01       	lock addq $0x1,(%r14)
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    4400:	4c 89 ef             	mov    %r13,%rdi
    4403:	e8 00 00 00 00       	callq  4408 <_scif_recv+0x5e8>
    4408:	e9 60 fb ff ff       	jmpq   3f6d <_scif_recv+0x14d>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    440d:	48 8b 45 08          	mov    0x8(%rbp),%rax
    4411:	48 89 d9             	mov    %rbx,%rcx
    4414:	ba a7 00 00 00       	mov    $0xa7,%edx
    4419:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4420:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    4427:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    442e:	44 0f b7 03          	movzwl (%rbx),%r8d
    4432:	48 89 04 24          	mov    %rax,(%rsp)
    4436:	31 c0                	xor    %eax,%eax
    4438:	e8 00 00 00 00       	callq  443d <_scif_recv+0x61d>
    443d:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4444:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    444b:	00 00 80 
    444e:	48 39 f1             	cmp    %rsi,%rcx
    4451:	0f 84 96 fb ff ff    	je     3fed <_scif_recv+0x1cd>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4457:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    445b:	48 89 c8             	mov    %rcx,%rax
    445e:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    4465:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4467:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    446a:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    446d:	0f 84 7a fb ff ff    	je     3fed <_scif_recv+0x1cd>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4473:	48 39 f2             	cmp    %rsi,%rdx
    4476:	0f 84 71 fb ff ff    	je     3fed <_scif_recv+0x1cd>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    447c:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4480:	48 89 d0             	mov    %rdx,%rax
    4483:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4489:	48 39 c2             	cmp    %rax,%rdx
    448c:	0f 84 5b fb ff ff    	je     3fed <_scif_recv+0x1cd>
    4492:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4495:	48 39 f2             	cmp    %rsi,%rdx
    4498:	75 e2                	jne    447c <_scif_recv+0x65c>
    449a:	e9 4e fb ff ff       	jmpq   3fed <_scif_recv+0x1cd>
    449f:	90                   	nop
    44a0:	41 83 bc 24 5c 01 00 	cmpl   $0x2,0x15c(%r12)
    44a7:	00 02 
    44a9:	0f 85 b2 fc ff ff    	jne    4161 <_scif_recv+0x341>
    44af:	48 8b 85 00 ff ff ff 	mov    -0x100(%rbp),%rax
    44b6:	48 8b 40 08          	mov    0x8(%rax),%rax
    44ba:	48 8b 40 10          	mov    0x10(%rax),%rax
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		/*
		 * Wait for a message now in the Blocking case.
		 * or until other side disconnects.
		 */
		if ((ret = wait_event_interruptible(ep->recvwq, 
    44be:	a8 04                	test   $0x4,%al
    44c0:	75 19                	jne    44db <_scif_recv+0x6bb>
    44c2:	e8 00 00 00 00       	callq  44c7 <_scif_recv+0x6a7>
    44c7:	e9 4c fc ff ff       	jmpq   4118 <_scif_recv+0x2f8>
					&ep->qp_info.qp->inbound_q,
					msg, (int) curr_recv_len);
			if (read_size < 0){
				/* only could happen when copy to USER buffer
				*/
				ret = -EFAULT;
    44cc:	c7 85 30 ff ff ff f2 	movl   $0xfffffff2,-0xd0(%rbp)
    44d3:	ff ff ff 
    44d6:	e9 d5 fa ff ff       	jmpq   3fb0 <_scif_recv+0x190>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		/*
		 * Wait for a message now in the Blocking case.
		 * or until other side disconnects.
		 */
		if ((ret = wait_event_interruptible(ep->recvwq, 
    44db:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
    44df:	4c 89 ff             	mov    %r15,%rdi
    44e2:	e8 00 00 00 00       	callq  44e7 <_scif_recv+0x6c7>
    44e7:	8b 95 30 ff ff ff    	mov    -0xd0(%rbp),%edx
    44ed:	b8 00 fe ff ff       	mov    $0xfffffe00,%eax
    44f2:	29 da                	sub    %ebx,%edx
    44f4:	48 39 9d 20 ff ff ff 	cmp    %rbx,-0xe0(%rbp)
    44fb:	0f 45 c2             	cmovne %edx,%eax
    44fe:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    4504:	e9 e4 fa ff ff       	jmpq   3fed <_scif_recv+0x1cd>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    4509:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 450f <_scif_recv+0x6ef>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    450f:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    4511:	49 c7 87 b0 01 00 00 	movq   $0x2,0x1b0(%r15)
    4518:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    451c:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    4523:	c7 45 a8 2e 00 00 00 	movl   $0x2e,-0x58(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    452a:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    452e:	66 89 45 a0          	mov    %ax,-0x60(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
    4532:	31 c0                	xor    %eax,%eax
    4534:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.payload[0] = dev->sd_node;
    4538:	41 0f b7 07          	movzwl (%r15),%eax
    453c:	48 89 45 ac          	mov    %rax,-0x54(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4540:	e8 00 00 00 00       	callq  4545 <_scif_recv+0x725>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    4545:	49 8b 87 b0 01 00 00 	mov    0x1b0(%r15),%rax
    454c:	48 83 f8 02          	cmp    $0x2,%rax
    4550:	74 20                	je     4572 <_scif_recv+0x752>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    4552:	48 83 f8 03          	cmp    $0x3,%rax
    4556:	0f 85 9f fe ff ff    	jne    43fb <_scif_recv+0x5db>
				dev->sd_state = SCIFDEV_RUNNING;
    455c:	41 c7 47 04 02 00 00 	movl   $0x2,0x4(%r15)
    4563:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    4564:	f0 41 80 a7 8f 01 00 	lock andb $0x7f,0x18f(%r15)
    456b:	00 7f 
    456d:	e9 89 fe ff ff       	jmpq   43fb <_scif_recv+0x5db>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    4572:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    4579:	48 c7 85 48 ff ff ff 	movq   $0x0,-0xb8(%rbp)
    4580:	00 00 00 00 
    4584:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
    458b:	00 00 00 00 
    458f:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    4596:	48 8b 85 08 ff ff ff 	mov    -0xf8(%rbp),%rax
    459d:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    45a4:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    45ab:	49 8d 87 98 01 00 00 	lea    0x198(%r15),%rax
    45b2:	48 89 85 00 ff ff ff 	mov    %rax,-0x100(%rbp)
    45b9:	eb 05                	jmp    45c0 <_scif_recv+0x7a0>
    45bb:	e8 00 00 00 00       	callq  45c0 <_scif_recv+0x7a0>
    45c0:	48 8b bd 00 ff ff ff 	mov    -0x100(%rbp),%rdi
    45c7:	ba 02 00 00 00       	mov    $0x2,%edx
    45cc:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    45d3:	e8 00 00 00 00       	callq  45d8 <_scif_recv+0x7b8>
    45d8:	49 83 bf b0 01 00 00 	cmpq   $0x2,0x1b0(%r15)
    45df:	02 
    45e0:	74 d9                	je     45bb <_scif_recv+0x79b>
    45e2:	48 8b bd 00 ff ff ff 	mov    -0x100(%rbp),%rdi
    45e9:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    45f0:	e8 00 00 00 00       	callq  45f5 <_scif_recv+0x7d5>
    45f5:	49 8b 87 b0 01 00 00 	mov    0x1b0(%r15),%rax
    45fc:	e9 51 ff ff ff       	jmpq   4552 <_scif_recv+0x732>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    4601:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 4607 <_scif_recv+0x7e7>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4607:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    460b:	31 c9                	xor    %ecx,%ecx
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    460d:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    460f:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    4616:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    461a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    4621:	c7 45 a8 2e 00 00 00 	movl   $0x2e,-0x58(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    4628:	66 89 4d a4          	mov    %cx,-0x5c(%rbp)
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    462c:	66 89 45 a0          	mov    %ax,-0x60(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    4630:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    4635:	48 89 45 ac          	mov    %rax,-0x54(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4639:	e8 00 00 00 00       	callq  463e <_scif_recv+0x81e>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    463e:	49 8b 85 b0 01 00 00 	mov    0x1b0(%r13),%rax
    4645:	48 83 f8 02          	cmp    $0x2,%rax
    4649:	74 20                	je     466b <_scif_recv+0x84b>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    464b:	48 83 f8 03          	cmp    $0x3,%rax
    464f:	0f 85 34 fd ff ff    	jne    4389 <_scif_recv+0x569>
				dev->sd_state = SCIFDEV_RUNNING;
    4655:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    465c:	00 
    465d:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    4664:	00 7f 
    4666:	e9 1e fd ff ff       	jmpq   4389 <_scif_recv+0x569>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    466b:	48 8d bd 70 ff ff ff 	lea    -0x90(%rbp),%rdi
    4672:	30 c0                	xor    %al,%al
    4674:	b9 0a 00 00 00       	mov    $0xa,%ecx
    4679:	f3 ab                	rep stos %eax,%es:(%rdi)
    467b:	49 8d 9d 98 01 00 00 	lea    0x198(%r13),%rbx
    4682:	48 c7 45 80 00 00 00 	movq   $0x0,-0x80(%rbp)
    4689:	00 
    468a:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    4691:	00 00 
    4693:	48 89 85 78 ff ff ff 	mov    %rax,-0x88(%rbp)
    469a:	48 8d 85 70 ff ff ff 	lea    -0x90(%rbp),%rax
    46a1:	48 83 c0 18          	add    $0x18,%rax
    46a5:	48 89 45 88          	mov    %rax,-0x78(%rbp)
    46a9:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    46ad:	eb 06                	jmp    46b5 <_scif_recv+0x895>
    46af:	90                   	nop
    46b0:	e8 00 00 00 00       	callq  46b5 <_scif_recv+0x895>
    46b5:	48 8d b5 70 ff ff ff 	lea    -0x90(%rbp),%rsi
    46bc:	ba 02 00 00 00       	mov    $0x2,%edx
    46c1:	48 89 df             	mov    %rbx,%rdi
    46c4:	e8 00 00 00 00       	callq  46c9 <_scif_recv+0x8a9>
    46c9:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    46d0:	02 
    46d1:	74 dd                	je     46b0 <_scif_recv+0x890>
    46d3:	48 8d b5 70 ff ff ff 	lea    -0x90(%rbp),%rsi
    46da:	48 89 df             	mov    %rbx,%rdi
    46dd:	e8 00 00 00 00       	callq  46e2 <_scif_recv+0x8c2>
    46e2:	49 8b 85 b0 01 00 00 	mov    0x1b0(%r13),%rax
    46e9:	e9 5d ff ff ff       	jmpq   464b <_scif_recv+0x82b>
    46ee:	66 90                	xchg   %ax,%ax

00000000000046f0 <scif_user_send>:
 * This function is called from the driver IOCTL entry point
 * only and is a wrapper for _scif_send().
 */
int
scif_user_send(scif_epd_t epd, void *msg, int len, int flags)
{
    46f0:	55                   	push   %rbp
    46f1:	48 89 e5             	mov    %rsp,%rbp
    46f4:	41 57                	push   %r15
    46f6:	41 56                	push   %r14
    46f8:	41 55                	push   %r13
    46fa:	41 54                	push   %r12
    46fc:	53                   	push   %rbx
    46fd:	48 81 ec a8 00 00 00 	sub    $0xa8,%rsp
    4704:	e8 00 00 00 00       	callq  4709 <scif_user_send+0x19>
    4709:	48 89 f8             	mov    %rdi,%rax
    470c:	48 89 bd 60 ff ff ff 	mov    %rdi,-0xa0(%rbp)
    4713:	49 89 f5             	mov    %rsi,%r13
    4716:	41 89 d6             	mov    %edx,%r14d
	int err = 0;
	int sent_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI send (U): ep %p %s\n", ep, scif_ep_states[ep->state]);
    4719:	8b 00                	mov    (%rax),%eax

	if (!len)
		return 0;
    471b:	31 c0                	xor    %eax,%eax
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI send (U): ep %p %s\n", ep, scif_ep_states[ep->state]);

	if (!len)
    471d:	85 d2                	test   %edx,%edx
 * This function is called from the driver IOCTL entry point
 * only and is a wrapper for _scif_send().
 */
int
scif_user_send(scif_epd_t epd, void *msg, int len, int flags)
{
    471f:	89 8d 5c ff ff ff    	mov    %ecx,-0xa4(%rbp)
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI send (U): ep %p %s\n", ep, scif_ep_states[ep->state]);

	if (!len)
    4725:	0f 84 58 01 00 00    	je     4883 <scif_user_send+0x193>
static inline int
scif_msg_param_check(scif_epd_t epd, int len, int flags)
{
	int ret = -EINVAL;

	if (len < 0)
    472b:	0f 88 67 01 00 00    	js     4898 <scif_user_send+0x1a8>
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int sent_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
    4731:	81 fa 00 00 40 00    	cmp    $0x400000,%edx
    4737:	41 bf 00 00 40 00    	mov    $0x400000,%r15d
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    473d:	89 c8                	mov    %ecx,%eax
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int sent_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
    473f:	44 0f 4e fa          	cmovle %edx,%r15d
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4743:	85 c9                	test   %ecx,%ecx
    4745:	74 08                	je     474f <scif_user_send+0x5f>
    4747:	a8 01                	test   $0x1,%al
    4749:	0f 84 49 01 00 00    	je     4898 <scif_user_send+0x1a8>
		return 0;

	if ((err = scif_msg_param_check(epd, len, flags)))
		goto send_err;

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
    474f:	49 63 ff             	movslq %r15d,%rdi

			return kmem_cache_alloc_trace(s, flags, size);
		}
	}
	return __kmalloc(size, flags);
    4752:	be d0 00 00 00       	mov    $0xd0,%esi
    4757:	e8 00 00 00 00       	callq  475c <scif_user_send+0x6c>
    475c:	48 85 c0             	test   %rax,%rax
    475f:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    4766:	0f 84 14 03 00 00    	je     4a80 <scif_user_send+0x390>
		err = -ENOMEM;
		goto send_err;
	}
	err = 0;
	micscif_inc_node_refcnt(ep->remote_dev, 1);
    476c:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
    4773:	48 8b 98 48 01 00 00 	mov    0x148(%rax),%rbx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    477a:	48 85 db             	test   %rbx,%rbx
    477d:	0f 85 f1 01 00 00    	jne    4974 <scif_user_send+0x284>
	/*
	 * Grabbing the lock before breaking up the transfer in
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);
    4783:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
int
scif_user_send(scif_epd_t epd, void *msg, int len, int flags)
{
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int sent_len = 0;
    478a:	45 31 e4             	xor    %r12d,%r12d

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
		err = -ENOMEM;
		goto send_err;
	}
	err = 0;
    478d:	31 db                	xor    %ebx,%ebx
	/*
	 * Grabbing the lock before breaking up the transfer in
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);
    478f:	48 05 00 02 00 00    	add    $0x200,%rax
    4795:	48 89 c7             	mov    %rax,%rdi
    4798:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    479f:	e8 00 00 00 00       	callq  47a4 <scif_user_send+0xb4>
    47a4:	0f 1f 40 00          	nopl   0x0(%rax)
	
	while (sent_len != len) {
		msg = (void *)((char *)msg + err);
    47a8:	48 63 db             	movslq %ebx,%rbx
    47ab:	49 01 dd             	add    %rbx,%r13
		loop_len = len - sent_len;
    47ae:	44 89 f3             	mov    %r14d,%ebx
    47b1:	44 29 e3             	sub    %r12d,%ebx
		loop_len = min(chunk_len, loop_len);
    47b4:	44 39 fb             	cmp    %r15d,%ebx
    47b7:	41 0f 4f df          	cmovg  %r15d,%ebx
#ifdef CONFIG_PROVE_LOCKING
void might_fault(void);
#else
static inline void might_fault(void)
{
	might_sleep();
    47bb:	e8 00 00 00 00       	callq  47c0 <scif_user_send+0xd0>
{
	int sz = __compiletime_object_size(to);

	might_fault();
	if (likely(sz == -1 || sz >= n))
		n = _copy_from_user(to, from, n);
    47c0:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    47c7:	4c 89 ee             	mov    %r13,%rsi
    47ca:	89 da                	mov    %ebx,%edx
    47cc:	e8 00 00 00 00       	callq  47d1 <scif_user_send+0xe1>
		if (copy_from_user(tmp, msg, loop_len)) {
    47d1:	48 85 c0             	test   %rax,%rax
    47d4:	0f 85 ce 00 00 00    	jne    48a8 <scif_user_send+0x1b8>
			err = -EFAULT;
			goto send_free_err;
		}
		err = _scif_send(epd, (void *)tmp, loop_len, flags);
    47da:	8b 8d 5c ff ff ff    	mov    -0xa4(%rbp),%ecx
    47e0:	89 da                	mov    %ebx,%edx
    47e2:	48 8b b5 68 ff ff ff 	mov    -0x98(%rbp),%rsi
    47e9:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    47f0:	e8 00 00 00 00       	callq  47f5 <scif_user_send+0x105>
		if (err < 0) {
    47f5:	85 c0                	test   %eax,%eax
		loop_len = min(chunk_len, loop_len);
		if (copy_from_user(tmp, msg, loop_len)) {
			err = -EFAULT;
			goto send_free_err;
		}
		err = _scif_send(epd, (void *)tmp, loop_len, flags);
    47f7:	41 89 c2             	mov    %eax,%r10d
		if (err < 0) {
    47fa:	0f 88 ae 00 00 00    	js     48ae <scif_user_send+0x1be>
			goto send_free_err;
		}
		sent_len += err;
    4800:	41 01 c4             	add    %eax,%r12d
		if (err !=loop_len) {
    4803:	39 c3                	cmp    %eax,%ebx
    4805:	0f 85 f5 00 00 00    	jne    4900 <scif_user_send+0x210>
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);
	
	while (sent_len != len) {
    480b:	45 39 e6             	cmp    %r12d,%r14d
    480e:	75 98                	jne    47a8 <scif_user_send+0xb8>
		if (err !=loop_len) {
			goto send_free_err;
		}
	}
send_free_err:
	mutex_unlock(&ep->sendlock);
    4810:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    4817:	89 9d 5c ff ff ff    	mov    %ebx,-0xa4(%rbp)
    481d:	e8 00 00 00 00       	callq  4822 <scif_user_send+0x132>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    4822:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    4829:	44 8b 95 5c ff ff ff 	mov    -0xa4(%rbp),%r10d
    4830:	48 8b 98 48 01 00 00 	mov    0x148(%rax),%rbx
    4837:	48 85 db             	test   %rbx,%rbx
    483a:	0f 84 20 01 00 00    	je     4960 <scif_user_send+0x270>
		if (unlikely((atomic_long_sub_return(cnt, 
    4840:	4c 8d ab 88 01 00 00 	lea    0x188(%rbx),%r13
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    4847:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    484e:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    4855:	00 00 
    4857:	48 83 e8 01          	sub    $0x1,%rax
    485b:	0f 88 77 01 00 00    	js     49d8 <scif_user_send+0x2e8>
	kfree(tmp);
    4861:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    4868:	44 89 95 60 ff ff ff 	mov    %r10d,-0xa0(%rbp)
    486f:	e8 00 00 00 00       	callq  4874 <scif_user_send+0x184>
send_err:
	return err < 0 ? err : sent_len;
    4874:	44 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10d
    487b:	45 85 d2             	test   %r10d,%r10d
    487e:	78 1e                	js     489e <scif_user_send+0x1ae>
		}
		err = _scif_send(epd, (void *)tmp, loop_len, flags);
		if (err < 0) {
			goto send_free_err;
		}
		sent_len += err;
    4880:	44 89 e0             	mov    %r12d,%eax
	mutex_unlock(&ep->sendlock);
	micscif_dec_node_refcnt(ep->remote_dev, 1);
	kfree(tmp);
send_err:
	return err < 0 ? err : sent_len;
}
    4883:	48 81 c4 a8 00 00 00 	add    $0xa8,%rsp
    488a:	5b                   	pop    %rbx
    488b:	41 5c                	pop    %r12
    488d:	41 5d                	pop    %r13
    488f:	41 5e                	pop    %r14
    4891:	41 5f                	pop    %r15
    4893:	5d                   	pop    %rbp
    4894:	c3                   	retq   
    4895:	0f 1f 00             	nopl   (%rax)
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4898:	41 ba ea ff ff ff    	mov    $0xffffffea,%r10d
    489e:	44 89 d0             	mov    %r10d,%eax
    48a1:	eb e0                	jmp    4883 <scif_user_send+0x193>
    48a3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	while (sent_len != len) {
		msg = (void *)((char *)msg + err);
		loop_len = len - sent_len;
		loop_len = min(chunk_len, loop_len);
		if (copy_from_user(tmp, msg, loop_len)) {
			err = -EFAULT;
    48a8:	41 ba f2 ff ff ff    	mov    $0xfffffff2,%r10d
		if (err !=loop_len) {
			goto send_free_err;
		}
	}
send_free_err:
	mutex_unlock(&ep->sendlock);
    48ae:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    48b5:	44 89 95 5c ff ff ff 	mov    %r10d,-0xa4(%rbp)
    48bc:	e8 00 00 00 00       	callq  48c1 <scif_user_send+0x1d1>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    48c1:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    48c8:	44 8b 95 5c ff ff ff 	mov    -0xa4(%rbp),%r10d
    48cf:	48 8b 98 48 01 00 00 	mov    0x148(%rax),%rbx
    48d6:	48 85 db             	test   %rbx,%rbx
    48d9:	0f 85 61 ff ff ff    	jne    4840 <scif_user_send+0x150>
	kfree(tmp);
    48df:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    48e6:	44 89 95 60 ff ff ff 	mov    %r10d,-0xa0(%rbp)
    48ed:	e8 00 00 00 00       	callq  48f2 <scif_user_send+0x202>
    48f2:	44 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10d
    48f9:	eb a3                	jmp    489e <scif_user_send+0x1ae>
    48fb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
		if (err !=loop_len) {
			goto send_free_err;
		}
	}
send_free_err:
	mutex_unlock(&ep->sendlock);
    4900:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    4907:	89 85 5c ff ff ff    	mov    %eax,-0xa4(%rbp)
    490d:	e8 00 00 00 00       	callq  4912 <scif_user_send+0x222>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    4912:	48 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%rax
    4919:	44 8b 95 5c ff ff ff 	mov    -0xa4(%rbp),%r10d
    4920:	48 8b 98 48 01 00 00 	mov    0x148(%rax),%rbx
    4927:	48 85 db             	test   %rbx,%rbx
    492a:	74 21                	je     494d <scif_user_send+0x25d>
		if (unlikely((atomic_long_sub_return(cnt, 
    492c:	4c 8d ab 88 01 00 00 	lea    0x188(%rbx),%r13
    4933:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    493a:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    4941:	00 00 
    4943:	48 83 e8 01          	sub    $0x1,%rax
    4947:	0f 88 3e 01 00 00    	js     4a8b <scif_user_send+0x39b>
	kfree(tmp);
    494d:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    4954:	e8 00 00 00 00       	callq  4959 <scif_user_send+0x269>
    4959:	e9 22 ff ff ff       	jmpq   4880 <scif_user_send+0x190>
    495e:	66 90                	xchg   %ax,%ax
    4960:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    4967:	e8 00 00 00 00       	callq  496c <scif_user_send+0x27c>
		}
		err = _scif_send(epd, (void *)tmp, loop_len, flags);
		if (err < 0) {
			goto send_free_err;
		}
		sent_len += err;
    496c:	44 89 f0             	mov    %r14d,%eax
    496f:	e9 0f ff ff ff       	jmpq   4883 <scif_user_send+0x193>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4974:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    497b:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4982:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    4989:	00 00 80 
    498c:	48 39 f1             	cmp    %rsi,%rcx
    498f:	0f 84 53 01 00 00    	je     4ae8 <scif_user_send+0x3f8>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4995:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4999:	48 89 c8             	mov    %rcx,%rax
    499c:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    49a3:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    49a5:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    49a8:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    49ab:	0f 84 d2 fd ff ff    	je     4783 <scif_user_send+0x93>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    49b1:	48 39 f2             	cmp    %rsi,%rdx
    49b4:	0f 84 2e 01 00 00    	je     4ae8 <scif_user_send+0x3f8>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    49ba:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    49be:	48 89 d0             	mov    %rdx,%rax
    49c1:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    49c7:	48 39 d0             	cmp    %rdx,%rax
    49ca:	0f 84 b3 fd ff ff    	je     4783 <scif_user_send+0x93>
    49d0:	48 89 c2             	mov    %rax,%rdx
    49d3:	eb dc                	jmp    49b1 <scif_user_send+0x2c1>
    49d5:	0f 1f 00             	nopl   (%rax)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    49d8:	48 8b 45 08          	mov    0x8(%rbp),%rax
    49dc:	48 89 d9             	mov    %rbx,%rcx
    49df:	ba a7 00 00 00       	mov    $0xa7,%edx
    49e4:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    49eb:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    49f2:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    49f9:	44 89 95 60 ff ff ff 	mov    %r10d,-0xa0(%rbp)
    4a00:	44 0f b7 03          	movzwl (%rbx),%r8d
    4a04:	48 89 04 24          	mov    %rax,(%rsp)
    4a08:	31 c0                	xor    %eax,%eax
    4a0a:	e8 00 00 00 00       	callq  4a0f <scif_user_send+0x31f>
    4a0f:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4a16:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    4a1d:	00 00 80 
    4a20:	44 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10d
    4a27:	48 39 c1             	cmp    %rax,%rcx
    4a2a:	0f 84 31 fe ff ff    	je     4861 <scif_user_send+0x171>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4a30:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4a34:	48 89 c8             	mov    %rcx,%rax
    4a37:	f0 49 0f b1 55 00    	lock cmpxchg %rdx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4a3d:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4a40:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4a43:	0f 84 18 fe ff ff    	je     4861 <scif_user_send+0x171>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4a49:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    4a50:	00 00 80 
    4a53:	48 39 f2             	cmp    %rsi,%rdx
    4a56:	0f 84 05 fe ff ff    	je     4861 <scif_user_send+0x171>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4a5c:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4a60:	48 89 d0             	mov    %rdx,%rax
    4a63:	f0 49 0f b1 4d 00    	lock cmpxchg %rcx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4a69:	48 39 d0             	cmp    %rdx,%rax
    4a6c:	0f 84 ef fd ff ff    	je     4861 <scif_user_send+0x171>
    4a72:	48 89 c2             	mov    %rax,%rdx
    4a75:	eb dc                	jmp    4a53 <scif_user_send+0x363>
    4a77:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    4a7e:	00 00 

	if ((err = scif_msg_param_check(epd, len, flags)))
		goto send_err;

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
		err = -ENOMEM;
    4a80:	41 ba f4 ff ff ff    	mov    $0xfffffff4,%r10d
    4a86:	e9 13 fe ff ff       	jmpq   489e <scif_user_send+0x1ae>
    4a8b:	48 8b 45 08          	mov    0x8(%rbp),%rax
    4a8f:	48 89 d9             	mov    %rbx,%rcx
    4a92:	ba a7 00 00 00       	mov    $0xa7,%edx
    4a97:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4a9e:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    4aa5:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    4aac:	44 89 95 60 ff ff ff 	mov    %r10d,-0xa0(%rbp)
    4ab3:	44 0f b7 03          	movzwl (%rbx),%r8d
    4ab7:	48 89 04 24          	mov    %rax,(%rsp)
    4abb:	31 c0                	xor    %eax,%eax
    4abd:	e8 00 00 00 00       	callq  4ac2 <scif_user_send+0x3d2>
    4ac2:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4ac9:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    4ad0:	00 00 80 
    4ad3:	44 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10d
    4ada:	48 39 c1             	cmp    %rax,%rcx
    4add:	0f 85 4d ff ff ff    	jne    4a30 <scif_user_send+0x340>
    4ae3:	e9 65 fe ff ff       	jmpq   494d <scif_user_send+0x25d>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    4ae8:	48 8d 83 68 01 00 00 	lea    0x168(%rbx),%rax
    4aef:	48 89 c7             	mov    %rax,%rdi
    4af2:	48 89 85 48 ff ff ff 	mov    %rax,-0xb8(%rbp)
    4af9:	e8 00 00 00 00       	callq  4afe <scif_user_send+0x40e>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    4afe:	8b 43 04             	mov    0x4(%rbx),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    4b01:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    4b04:	83 fa 01             	cmp    $0x1,%edx
    4b07:	76 19                	jbe    4b22 <scif_user_send+0x432>
    4b09:	83 f8 01             	cmp    $0x1,%eax
    4b0c:	74 14                	je     4b22 <scif_user_send+0x432>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    4b0e:	48 8b 83 88 01 00 00 	mov    0x188(%rbx),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    4b15:	48 85 c0             	test   %rax,%rax
    4b18:	78 19                	js     4b33 <scif_user_send+0x443>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    4b1a:	4c 89 e7             	mov    %r12,%rdi
    4b1d:	e8 ce b9 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    4b22:	48 8b bd 48 ff ff ff 	mov    -0xb8(%rbp),%rdi
    4b29:	e8 00 00 00 00       	callq  4b2e <scif_user_send+0x43e>
    4b2e:	e9 50 fc ff ff       	jmpq   4783 <scif_user_send+0x93>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    4b33:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 4b39 <scif_user_send+0x449>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    4b39:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4b40:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    4b44:	31 d2                	xor    %edx,%edx
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    4b46:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4b4c:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    4b53:	48 c7 83 b0 01 00 00 	movq   $0x2,0x1b0(%rbx)
    4b5a:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    4b5e:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    4b62:	0f b7 03             	movzwl (%rbx),%eax
    4b65:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    4b69:	e8 00 00 00 00       	callq  4b6e <scif_user_send+0x47e>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    4b6e:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    4b75:	02 
    4b76:	74 1b                	je     4b93 <scif_user_send+0x4a3>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    4b78:	48 83 bb b0 01 00 00 	cmpq   $0x3,0x1b0(%rbx)
    4b7f:	03 
    4b80:	75 98                	jne    4b1a <scif_user_send+0x42a>
				dev->sd_state = SCIFDEV_RUNNING;
    4b82:	c7 43 04 02 00 00 00 	movl   $0x2,0x4(%rbx)
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    4b89:	f0 80 a3 8f 01 00 00 	lock andb $0x7f,0x18f(%rbx)
    4b90:	7f 
    4b91:	eb 87                	jmp    4b1a <scif_user_send+0x42a>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    4b93:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    4b9a:	31 c0                	xor    %eax,%eax
    4b9c:	b9 0a 00 00 00       	mov    $0xa,%ecx
    4ba1:	f3 ab                	rep stos %eax,%es:(%rdi)
    4ba3:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    4baa:	00 
    4bab:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    4bb2:	00 00 
    4bb4:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    4bb8:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    4bbf:	48 83 c0 18          	add    $0x18,%rax
    4bc3:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    4bc7:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    4bcb:	48 8d 83 98 01 00 00 	lea    0x198(%rbx),%rax
    4bd2:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    4bd9:	eb 0a                	jmp    4be5 <scif_user_send+0x4f5>
    4bdb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    4be0:	e8 00 00 00 00       	callq  4be5 <scif_user_send+0x4f5>
    4be5:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    4bec:	ba 02 00 00 00       	mov    $0x2,%edx
    4bf1:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    4bf8:	e8 00 00 00 00       	callq  4bfd <scif_user_send+0x50d>
    4bfd:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    4c04:	02 
    4c05:	74 d9                	je     4be0 <scif_user_send+0x4f0>
    4c07:	48 8b bd 50 ff ff ff 	mov    -0xb0(%rbp),%rdi
    4c0e:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    4c15:	e8 00 00 00 00       	callq  4c1a <scif_user_send+0x52a>
    4c1a:	e9 59 ff ff ff       	jmpq   4b78 <scif_user_send+0x488>
    4c1f:	90                   	nop

0000000000004c20 <scif_user_recv>:
 * This function is called from the driver IOCTL entry point
 * only and is a wrapper for _scif_recv().
 */
int
scif_user_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    4c20:	55                   	push   %rbp
    4c21:	48 89 e5             	mov    %rsp,%rbp
    4c24:	41 57                	push   %r15
    4c26:	41 56                	push   %r14
    4c28:	41 55                	push   %r13
    4c2a:	41 54                	push   %r12
    4c2c:	53                   	push   %rbx
    4c2d:	48 83 ec 28          	sub    $0x28,%rsp
    4c31:	e8 00 00 00 00       	callq  4c36 <scif_user_recv+0x16>
    4c36:	48 89 f8             	mov    %rdi,%rax
    4c39:	48 89 7d b8          	mov    %rdi,-0x48(%rbp)
    4c3d:	49 89 f5             	mov    %rsi,%r13
    4c40:	41 89 d6             	mov    %edx,%r14d
	int err = 0;
	int recv_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI recv (U): ep %p %s\n", ep, scif_ep_states[ep->state]);
    4c43:	8b 00                	mov    (%rax),%eax

	if (!len)
		return 0;
    4c45:	31 c0                	xor    %eax,%eax
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI recv (U): ep %p %s\n", ep, scif_ep_states[ep->state]);

	if (!len)
    4c47:	85 d2                	test   %edx,%edx
 * This function is called from the driver IOCTL entry point
 * only and is a wrapper for _scif_recv().
 */
int
scif_user_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    4c49:	89 4d c4             	mov    %ecx,-0x3c(%rbp)
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
	pr_debug("SCIFAPI recv (U): ep %p %s\n", ep, scif_ep_states[ep->state]);

	if (!len)
    4c4c:	0f 84 ce 00 00 00    	je     4d20 <scif_user_recv+0x100>
static inline int
scif_msg_param_check(scif_epd_t epd, int len, int flags)
{
	int ret = -EINVAL;

	if (len < 0)
    4c52:	0f 88 d8 00 00 00    	js     4d30 <scif_user_recv+0x110>
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int recv_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
    4c58:	81 fa 00 00 40 00    	cmp    $0x400000,%edx
    4c5e:	41 bf 00 00 40 00    	mov    $0x400000,%r15d
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4c64:	89 c8                	mov    %ecx,%eax
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int recv_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
    4c66:	44 0f 4e fa          	cmovle %edx,%r15d
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4c6a:	85 c9                	test   %ecx,%ecx
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int recv_len = 0;
	char *tmp;
	int loop_len;
	int chunk_len = min(len, (1 << (MAX_ORDER + PAGE_SHIFT - 1)));;
    4c6c:	44 89 7d c0          	mov    %r15d,-0x40(%rbp)
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4c70:	74 08                	je     4c7a <scif_user_recv+0x5a>
    4c72:	a8 01                	test   $0x1,%al
    4c74:	0f 84 b6 00 00 00    	je     4d30 <scif_user_recv+0x110>
		return 0;

	if ((err = scif_msg_param_check(epd, len, flags)))
		goto recv_err;

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
    4c7a:	48 63 7d c0          	movslq -0x40(%rbp),%rdi
    4c7e:	be d0 00 00 00       	mov    $0xd0,%esi
    4c83:	e8 00 00 00 00       	callq  4c88 <scif_user_recv+0x68>
    4c88:	48 85 c0             	test   %rax,%rax
    4c8b:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
    4c8f:	0f 84 fb 00 00 00    	je     4d90 <scif_user_recv+0x170>
	/*
	 * Grabbing the lock before breaking up the transfer in
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);
    4c95:	48 8b 45 b8          	mov    -0x48(%rbp),%rax
int
scif_user_recv(scif_epd_t epd, void *msg, int len, int flags)
{
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;
	int recv_len = 0;
    4c99:	45 31 e4             	xor    %r12d,%r12d

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
		err = -ENOMEM;
		goto recv_err;
	}
	err = 0;
    4c9c:	31 db                	xor    %ebx,%ebx
	/*
	 * Grabbing the lock before breaking up the transfer in
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);
    4c9e:	48 05 00 02 00 00    	add    $0x200,%rax
    4ca4:	48 89 c7             	mov    %rax,%rdi
    4ca7:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    4cab:	e8 00 00 00 00       	callq  4cb0 <scif_user_recv+0x90>

	while (recv_len != len) {
		msg = (void *)((char *)msg + err);
		loop_len = len - recv_len;
		loop_len = min(chunk_len, loop_len);
    4cb0:	8b 45 c0             	mov    -0x40(%rbp),%eax
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);

	while (recv_len != len) {
		msg = (void *)((char *)msg + err);
    4cb3:	48 63 db             	movslq %ebx,%rbx
    4cb6:	49 01 dd             	add    %rbx,%r13
		loop_len = len - recv_len;
    4cb9:	44 89 f3             	mov    %r14d,%ebx
		loop_len = min(chunk_len, loop_len);
		if ((err = _scif_recv(epd, tmp, loop_len, flags)) < 0)
    4cbc:	8b 4d c4             	mov    -0x3c(%rbp),%ecx
	 */
	mutex_lock(&ep->sendlock);

	while (recv_len != len) {
		msg = (void *)((char *)msg + err);
		loop_len = len - recv_len;
    4cbf:	44 29 e3             	sub    %r12d,%ebx
		loop_len = min(chunk_len, loop_len);
		if ((err = _scif_recv(epd, tmp, loop_len, flags)) < 0)
    4cc2:	48 8b 75 c8          	mov    -0x38(%rbp),%rsi
    4cc6:	48 8b 7d b8          	mov    -0x48(%rbp),%rdi
	mutex_lock(&ep->sendlock);

	while (recv_len != len) {
		msg = (void *)((char *)msg + err);
		loop_len = len - recv_len;
		loop_len = min(chunk_len, loop_len);
    4cca:	39 c3                	cmp    %eax,%ebx
    4ccc:	0f 4f d8             	cmovg  %eax,%ebx
		if ((err = _scif_recv(epd, tmp, loop_len, flags)) < 0)
    4ccf:	89 da                	mov    %ebx,%edx
    4cd1:	e8 00 00 00 00       	callq  4cd6 <scif_user_recv+0xb6>
    4cd6:	85 c0                	test   %eax,%eax
    4cd8:	41 89 c7             	mov    %eax,%r15d
    4cdb:	78 79                	js     4d56 <scif_user_recv+0x136>
    4cdd:	e8 00 00 00 00       	callq  4ce2 <scif_user_recv+0xc2>
static __always_inline __must_check
int copy_to_user(void __user *dst, const void *src, unsigned size)
{
	might_fault();

	return _copy_to_user(dst, src, size);
    4ce2:	48 8b 75 c8          	mov    -0x38(%rbp),%rsi
    4ce6:	44 89 fa             	mov    %r15d,%edx
    4ce9:	4c 89 ef             	mov    %r13,%rdi
    4cec:	e8 00 00 00 00       	callq  4cf1 <scif_user_recv+0xd1>
			goto recv_free_err;
		if (copy_to_user(msg, tmp, err)) {
    4cf1:	85 c0                	test   %eax,%eax
    4cf3:	75 5b                	jne    4d50 <scif_user_recv+0x130>
			err = -EFAULT;
			goto recv_free_err;
		}
		recv_len += err;
    4cf5:	45 01 fc             	add    %r15d,%r12d
		if (err !=loop_len) {
    4cf8:	44 39 fb             	cmp    %r15d,%ebx
    4cfb:	75 73                	jne    4d70 <scif_user_recv+0x150>
	 * multiple chunks is required to ensure that messages do
	 * not get fragmented and reordered.
	 */
	mutex_lock(&ep->sendlock);

	while (recv_len != len) {
    4cfd:	45 39 e6             	cmp    %r12d,%r14d
    4d00:	75 ae                	jne    4cb0 <scif_user_recv+0x90>
		if (err !=loop_len) {
			goto recv_free_err;
		}
	}
recv_free_err:
	mutex_unlock(&ep->sendlock);
    4d02:	48 8b 7d b0          	mov    -0x50(%rbp),%rdi
    4d06:	e8 00 00 00 00       	callq  4d0b <scif_user_recv+0xeb>
	kfree(tmp);
    4d0b:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
    4d0f:	e8 00 00 00 00       	callq  4d14 <scif_user_recv+0xf4>
			goto recv_free_err;
		if (copy_to_user(msg, tmp, err)) {
			err = -EFAULT;
			goto recv_free_err;
		}
		recv_len += err;
    4d14:	44 89 f0             	mov    %r14d,%eax
    4d17:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    4d1e:	00 00 
recv_free_err:
	mutex_unlock(&ep->sendlock);
	kfree(tmp);
recv_err:
	return err < 0 ? err : recv_len;
}
    4d20:	48 83 c4 28          	add    $0x28,%rsp
    4d24:	5b                   	pop    %rbx
    4d25:	41 5c                	pop    %r12
    4d27:	41 5d                	pop    %r13
    4d29:	41 5e                	pop    %r14
    4d2b:	41 5f                	pop    %r15
    4d2d:	5d                   	pop    %rbp
    4d2e:	c3                   	retq   
    4d2f:	90                   	nop
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4d30:	41 bf ea ff ff ff    	mov    $0xffffffea,%r15d
recv_free_err:
	mutex_unlock(&ep->sendlock);
	kfree(tmp);
recv_err:
	return err < 0 ? err : recv_len;
}
    4d36:	48 83 c4 28          	add    $0x28,%rsp
			goto recv_free_err;
		if (copy_to_user(msg, tmp, err)) {
			err = -EFAULT;
			goto recv_free_err;
		}
		recv_len += err;
    4d3a:	44 89 f8             	mov    %r15d,%eax
recv_free_err:
	mutex_unlock(&ep->sendlock);
	kfree(tmp);
recv_err:
	return err < 0 ? err : recv_len;
}
    4d3d:	5b                   	pop    %rbx
    4d3e:	41 5c                	pop    %r12
    4d40:	41 5d                	pop    %r13
    4d42:	41 5e                	pop    %r14
    4d44:	41 5f                	pop    %r15
    4d46:	5d                   	pop    %rbp
    4d47:	c3                   	retq   
    4d48:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4d4f:	00 
		loop_len = len - recv_len;
		loop_len = min(chunk_len, loop_len);
		if ((err = _scif_recv(epd, tmp, loop_len, flags)) < 0)
			goto recv_free_err;
		if (copy_to_user(msg, tmp, err)) {
			err = -EFAULT;
    4d50:	41 bf f2 ff ff ff    	mov    $0xfffffff2,%r15d
		if (err !=loop_len) {
			goto recv_free_err;
		}
	}
recv_free_err:
	mutex_unlock(&ep->sendlock);
    4d56:	48 8b 7d b0          	mov    -0x50(%rbp),%rdi
    4d5a:	e8 00 00 00 00       	callq  4d5f <scif_user_recv+0x13f>
	kfree(tmp);
    4d5f:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
    4d63:	e8 00 00 00 00       	callq  4d68 <scif_user_recv+0x148>
    4d68:	eb cc                	jmp    4d36 <scif_user_recv+0x116>
    4d6a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		if (err !=loop_len) {
			goto recv_free_err;
		}
	}
recv_free_err:
	mutex_unlock(&ep->sendlock);
    4d70:	48 8b 7d b0          	mov    -0x50(%rbp),%rdi
    4d74:	e8 00 00 00 00       	callq  4d79 <scif_user_recv+0x159>
	kfree(tmp);
    4d79:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
    4d7d:	e8 00 00 00 00       	callq  4d82 <scif_user_recv+0x162>
			goto recv_free_err;
		if (copy_to_user(msg, tmp, err)) {
			err = -EFAULT;
			goto recv_free_err;
		}
		recv_len += err;
    4d82:	44 89 e0             	mov    %r12d,%eax
    4d85:	eb 99                	jmp    4d20 <scif_user_recv+0x100>
    4d87:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    4d8e:	00 00 

	if ((err = scif_msg_param_check(epd, len, flags)))
		goto recv_err;

	if (!(tmp = kmalloc(chunk_len, GFP_KERNEL))) {
		err = -ENOMEM;
    4d90:	41 bf f4 ff ff ff    	mov    $0xfffffff4,%r15d
    4d96:	eb 9e                	jmp    4d36 <scif_user_recv+0x116>
    4d98:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4d9f:	00 

0000000000004da0 <__scif_send>:
 * This function is called from the kernel mode only and is
 * a wrapper for _scif_send().
 */
int
__scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
    4da0:	55                   	push   %rbp
    4da1:	48 89 e5             	mov    %rsp,%rbp
    4da4:	41 57                	push   %r15
    4da6:	41 56                	push   %r14
    4da8:	41 55                	push   %r13
    4daa:	41 54                	push   %r12
    4dac:	53                   	push   %rbx
    4dad:	48 81 ec 98 00 00 00 	sub    $0x98,%rsp
    4db4:	e8 00 00 00 00       	callq  4db9 <__scif_send+0x19>
	struct endpt *ep = (struct endpt *)epd;
	int ret;

	pr_debug("SCIFAPI send (K): ep %p %s\n", ep, scif_ep_states[ep->state]);
	if (!len)
		return 0;
    4db9:	45 31 ed             	xor    %r13d,%r13d
__scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
	struct endpt *ep = (struct endpt *)epd;
	int ret;

	pr_debug("SCIFAPI send (K): ep %p %s\n", ep, scif_ep_states[ep->state]);
    4dbc:	8b 07                	mov    (%rdi),%eax
	if (!len)
    4dbe:	85 d2                	test   %edx,%edx
    4dc0:	74 6e                	je     4e30 <__scif_send+0x90>
static inline int
scif_msg_param_check(scif_epd_t epd, int len, int flags)
{
	int ret = -EINVAL;

	if (len < 0)
    4dc2:	41 bd ea ff ff ff    	mov    $0xffffffea,%r13d
    4dc8:	78 66                	js     4e30 <__scif_send+0x90>
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    4dca:	85 c9                	test   %ecx,%ecx
    4dcc:	41 89 ce             	mov    %ecx,%r14d
    4dcf:	41 89 d4             	mov    %edx,%r12d
    4dd2:	49 89 f7             	mov    %rsi,%r15
    4dd5:	48 89 fb             	mov    %rdi,%rbx
    4dd8:	74 6e                	je     4e48 <__scif_send+0xa8>
    4dda:	f6 c1 01             	test   $0x1,%cl
    4ddd:	74 51                	je     4e30 <__scif_send+0x90>
	if (!(flags & SCIF_SEND_BLOCK) && ep->remote_dev &&
		SCIF_NODE_IDLE == atomic_long_read(
			&ep->remote_dev->scif_ref_cnt))
		return -ENODEV;

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    4ddf:	4c 8b af 48 01 00 00 	mov    0x148(%rdi),%r13
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    4de6:	4d 85 ed             	test   %r13,%r13
    4de9:	0f 85 a6 03 00 00    	jne    5195 <__scif_send+0x3f5>
	 * to ensure messages do not get fragmented/reordered.
	 * The non blocking mode is protected using spin locks
	 * in _scif_send().
	 */
	if (flags & SCIF_SEND_BLOCK)
		mutex_lock(&ep->sendlock);
    4def:	4c 8d ab 00 02 00 00 	lea    0x200(%rbx),%r13
    4df6:	4c 89 ef             	mov    %r13,%rdi
    4df9:	e8 00 00 00 00       	callq  4dfe <__scif_send+0x5e>

	ret = _scif_send(epd, msg, len, flags);
    4dfe:	44 89 e2             	mov    %r12d,%edx
    4e01:	48 89 df             	mov    %rbx,%rdi
    4e04:	44 89 f1             	mov    %r14d,%ecx
    4e07:	4c 89 fe             	mov    %r15,%rsi
    4e0a:	e8 00 00 00 00       	callq  4e0f <__scif_send+0x6f>

	if (flags & SCIF_SEND_BLOCK)
		mutex_unlock(&ep->sendlock);
    4e0f:	4c 89 ef             	mov    %r13,%rdi
	 * in _scif_send().
	 */
	if (flags & SCIF_SEND_BLOCK)
		mutex_lock(&ep->sendlock);

	ret = _scif_send(epd, msg, len, flags);
    4e12:	41 89 c4             	mov    %eax,%r12d

	if (flags & SCIF_SEND_BLOCK)
		mutex_unlock(&ep->sendlock);
    4e15:	e8 00 00 00 00       	callq  4e1a <__scif_send+0x7a>

	micscif_dec_node_refcnt(ep->remote_dev, 1);
    4e1a:	48 8b 9b 48 01 00 00 	mov    0x148(%rbx),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    4e21:	45 89 e5             	mov    %r12d,%r13d
    4e24:	48 85 db             	test   %rbx,%rbx
    4e27:	0f 85 b1 00 00 00    	jne    4ede <__scif_send+0x13e>
    4e2d:	0f 1f 00             	nopl   (%rax)
	return ret;
}
    4e30:	48 81 c4 98 00 00 00 	add    $0x98,%rsp
    4e37:	44 89 e8             	mov    %r13d,%eax
    4e3a:	5b                   	pop    %rbx
    4e3b:	41 5c                	pop    %r12
    4e3d:	41 5d                	pop    %r13
    4e3f:	41 5e                	pop    %r14
    4e41:	41 5f                	pop    %r15
    4e43:	5d                   	pop    %rbp
    4e44:	c3                   	retq   
    4e45:	0f 1f 00             	nopl   (%rax)
	/*
	 * Cannot block while waiting for node to wake up
	 * if non blocking messaging mode is requested. Return
	 * ENODEV if the remote node is idle.
	 */
	if (!(flags & SCIF_SEND_BLOCK) && ep->remote_dev &&
    4e48:	4c 8b af 48 01 00 00 	mov    0x148(%rdi),%r13
    4e4f:	4d 85 ed             	test   %r13,%r13
    4e52:	74 63                	je     4eb7 <__scif_send+0x117>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4e54:	49 8b 95 88 01 00 00 	mov    0x188(%r13),%rdx
    4e5b:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    4e62:	00 00 80 
    4e65:	48 39 c2             	cmp    %rax,%rdx
    4e68:	0f 84 c0 01 00 00    	je     502e <__scif_send+0x28e>
    4e6e:	45 31 d2             	xor    %r10d,%r10d
    4e71:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    4e78:	4d 8d 9d 88 01 00 00 	lea    0x188(%r13),%r11
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4e7f:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    4e86:	00 00 80 
    4e89:	48 39 f1             	cmp    %rsi,%rcx
    4e8c:	0f 84 f5 00 00 00    	je     4f87 <__scif_send+0x1e7>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4e92:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4e96:	48 89 c8             	mov    %rcx,%rax
    4e99:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    4ea0:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4ea2:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4ea5:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4ea8:	0f 85 5d 01 00 00    	jne    500b <__scif_send+0x26b>
	 * Grab the mutex lock in the blocking case only
	 * to ensure messages do not get fragmented/reordered.
	 * The non blocking mode is protected using spin locks
	 * in _scif_send().
	 */
	if (flags & SCIF_SEND_BLOCK)
    4eae:	45 85 d2             	test   %r10d,%r10d
    4eb1:	0f 85 38 ff ff ff    	jne    4def <__scif_send+0x4f>
		mutex_lock(&ep->sendlock);

	ret = _scif_send(epd, msg, len, flags);
    4eb7:	44 89 e2             	mov    %r12d,%edx
    4eba:	48 89 df             	mov    %rbx,%rdi
    4ebd:	44 89 f1             	mov    %r14d,%ecx
    4ec0:	4c 89 fe             	mov    %r15,%rsi
    4ec3:	e8 00 00 00 00       	callq  4ec8 <__scif_send+0x128>

	if (flags & SCIF_SEND_BLOCK)
		mutex_unlock(&ep->sendlock);

	micscif_dec_node_refcnt(ep->remote_dev, 1);
    4ec8:	48 8b 9b 48 01 00 00 	mov    0x148(%rbx),%rbx
	 * in _scif_send().
	 */
	if (flags & SCIF_SEND_BLOCK)
		mutex_lock(&ep->sendlock);

	ret = _scif_send(epd, msg, len, flags);
    4ecf:	41 89 c4             	mov    %eax,%r12d
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    4ed2:	45 89 e5             	mov    %r12d,%r13d
    4ed5:	48 85 db             	test   %rbx,%rbx
    4ed8:	0f 84 52 ff ff ff    	je     4e30 <__scif_send+0x90>
		if (unlikely((atomic_long_sub_return(cnt, 
    4ede:	4c 8d b3 88 01 00 00 	lea    0x188(%rbx),%r14
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    4ee5:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    4eec:	f0 48 0f c1 93 88 01 	lock xadd %rdx,0x188(%rbx)
    4ef3:	00 00 
    4ef5:	48 83 ea 01          	sub    $0x1,%rdx
    4ef9:	0f 89 31 ff ff ff    	jns    4e30 <__scif_send+0x90>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    4eff:	48 8b 45 08          	mov    0x8(%rbp),%rax
    4f03:	48 89 d9             	mov    %rbx,%rcx
    4f06:	ba a7 00 00 00       	mov    $0xa7,%edx
    4f0b:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    4f12:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    4f19:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    4f20:	44 0f b7 03          	movzwl (%rbx),%r8d
    4f24:	48 89 04 24          	mov    %rax,(%rsp)
    4f28:	31 c0                	xor    %eax,%eax
    4f2a:	e8 00 00 00 00       	callq  4f2f <__scif_send+0x18f>
    4f2f:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4f36:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    4f3d:	00 00 80 
    4f40:	48 39 f1             	cmp    %rsi,%rcx
    4f43:	0f 84 e7 fe ff ff    	je     4e30 <__scif_send+0x90>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4f49:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4f4d:	48 89 c8             	mov    %rcx,%rax
    4f50:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    4f57:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4f59:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4f5c:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4f5f:	0f 84 cb fe ff ff    	je     4e30 <__scif_send+0x90>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    4f65:	48 39 f2             	cmp    %rsi,%rdx
    4f68:	74 15                	je     4f7f <__scif_send+0x1df>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    4f6a:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    4f6e:	48 89 d0             	mov    %rdx,%rax
    4f71:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    4f76:	48 39 c2             	cmp    %rax,%rdx
    4f79:	0f 85 41 01 00 00    	jne    50c0 <__scif_send+0x320>
    4f7f:	45 89 e5             	mov    %r12d,%r13d
    4f82:	e9 a9 fe ff ff       	jmpq   4e30 <__scif_send+0x90>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    4f87:	49 8d 85 68 01 00 00 	lea    0x168(%r13),%rax
    4f8e:	44 89 95 68 ff ff ff 	mov    %r10d,-0x98(%rbp)
    4f95:	48 89 c7             	mov    %rax,%rdi
    4f98:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    4f9f:	4c 89 9d 58 ff ff ff 	mov    %r11,-0xa8(%rbp)
    4fa6:	e8 00 00 00 00       	callq  4fab <__scif_send+0x20b>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    4fab:	41 8b 45 04          	mov    0x4(%r13),%eax
    4faf:	44 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10d
			SCIFDEV_STOPPING == dev->sd_state ||
    4fb6:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    4fb9:	83 fa 01             	cmp    $0x1,%edx
    4fbc:	76 2e                	jbe    4fec <__scif_send+0x24c>
    4fbe:	83 f8 01             	cmp    $0x1,%eax
    4fc1:	4c 8b 9d 58 ff ff ff 	mov    -0xa8(%rbp),%r11
    4fc8:	74 22                	je     4fec <__scif_send+0x24c>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    4fca:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    4fd1:	48 85 c0             	test   %rax,%rax
    4fd4:	78 63                	js     5039 <__scif_send+0x299>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    4fd6:	4c 89 df             	mov    %r11,%rdi
    4fd9:	44 89 95 68 ff ff ff 	mov    %r10d,-0x98(%rbp)
    4fe0:	e8 0b b5 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
    4fe5:	44 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10d
bail_out:
		mutex_unlock(&dev->sd_lock);
    4fec:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    4ff3:	44 89 95 68 ff ff ff 	mov    %r10d,-0x98(%rbp)
    4ffa:	e8 00 00 00 00       	callq  4fff <__scif_send+0x25f>
    4fff:	44 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10d
    5006:	e9 a3 fe ff ff       	jmpq   4eae <__scif_send+0x10e>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    500b:	48 39 f2             	cmp    %rsi,%rdx
    500e:	0f 84 73 ff ff ff    	je     4f87 <__scif_send+0x1e7>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5014:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5018:	48 89 d0             	mov    %rdx,%rax
    501b:	f0 49 0f b1 0b       	lock cmpxchg %rcx,(%r11)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5020:	48 39 d0             	cmp    %rdx,%rax
    5023:	0f 84 85 fe ff ff    	je     4eae <__scif_send+0x10e>
    5029:	48 89 c2             	mov    %rax,%rdx
    502c:	eb dd                	jmp    500b <__scif_send+0x26b>
	 * ENODEV if the remote node is idle.
	 */
	if (!(flags & SCIF_SEND_BLOCK) && ep->remote_dev &&
		SCIF_NODE_IDLE == atomic_long_read(
			&ep->remote_dev->scif_ref_cnt))
		return -ENODEV;
    502e:	41 bd ed ff ff ff    	mov    $0xffffffed,%r13d
    5034:	e9 f7 fd ff ff       	jmpq   4e30 <__scif_send+0x90>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    5039:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 503f <__scif_send+0x29f>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    503f:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5046:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    504a:	31 d2                	xor    %edx,%edx
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    504c:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5052:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    5059:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    5060:	02 00 00 00 
    5064:	4c 89 9d 58 ff ff ff 	mov    %r11,-0xa8(%rbp)
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    506b:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    506f:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    5074:	44 89 95 68 ff ff ff 	mov    %r10d,-0x98(%rbp)
    507b:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    507f:	e8 00 00 00 00       	callq  5084 <__scif_send+0x2e4>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    5084:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    508b:	02 
    508c:	44 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10d
    5093:	4c 8b 9d 58 ff ff ff 	mov    -0xa8(%rbp),%r11
    509a:	74 2c                	je     50c8 <__scif_send+0x328>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    509c:	49 83 bd b0 01 00 00 	cmpq   $0x3,0x1b0(%r13)
    50a3:	03 
    50a4:	0f 85 2c ff ff ff    	jne    4fd6 <__scif_send+0x236>
				dev->sd_state = SCIFDEV_RUNNING;
    50aa:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    50b1:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    50b2:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    50b9:	00 7f 
    50bb:	e9 16 ff ff ff       	jmpq   4fd6 <__scif_send+0x236>
    50c0:	48 89 c2             	mov    %rax,%rdx
    50c3:	e9 9d fe ff ff       	jmpq   4f65 <__scif_send+0x1c5>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    50c8:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    50cf:	31 c0                	xor    %eax,%eax
    50d1:	b9 0a 00 00 00       	mov    $0xa,%ecx
    50d6:	f3 ab                	rep stos %eax,%es:(%rdi)
    50d8:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    50df:	00 
    50e0:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    50e7:	00 00 
    50e9:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    50ed:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    50f4:	48 83 c0 18          	add    $0x18,%rax
    50f8:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    50fc:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    5100:	49 8d 85 98 01 00 00 	lea    0x198(%r13),%rax
    5107:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    510e:	eb 13                	jmp    5123 <__scif_send+0x383>
    5110:	e8 00 00 00 00       	callq  5115 <__scif_send+0x375>
    5115:	44 8b 95 58 ff ff ff 	mov    -0xa8(%rbp),%r10d
    511c:	4c 8b 9d 50 ff ff ff 	mov    -0xb0(%rbp),%r11
    5123:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    512a:	ba 02 00 00 00       	mov    $0x2,%edx
    512f:	4c 89 9d 50 ff ff ff 	mov    %r11,-0xb0(%rbp)
    5136:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    513d:	44 89 95 58 ff ff ff 	mov    %r10d,-0xa8(%rbp)
    5144:	e8 00 00 00 00       	callq  5149 <__scif_send+0x3a9>
    5149:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    5150:	02 
    5151:	44 8b 95 58 ff ff ff 	mov    -0xa8(%rbp),%r10d
    5158:	4c 8b 9d 50 ff ff ff 	mov    -0xb0(%rbp),%r11
    515f:	74 af                	je     5110 <__scif_send+0x370>
    5161:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    5168:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    516f:	4c 89 9d 50 ff ff ff 	mov    %r11,-0xb0(%rbp)
    5176:	44 89 95 58 ff ff ff 	mov    %r10d,-0xa8(%rbp)
    517d:	e8 00 00 00 00       	callq  5182 <__scif_send+0x3e2>
    5182:	4c 8b 9d 50 ff ff ff 	mov    -0xb0(%rbp),%r11
    5189:	44 8b 95 58 ff ff ff 	mov    -0xa8(%rbp),%r10d
    5190:	e9 07 ff ff ff       	jmpq   509c <__scif_send+0x2fc>
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    5195:	41 ba 01 00 00 00    	mov    $0x1,%r10d
    519b:	e9 d1 fc ff ff       	jmpq   4e71 <__scif_send+0xd1>

00000000000051a0 <scif_send>:
	return ret;
}

int
scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
    51a0:	55                   	push   %rbp
    51a1:	48 89 e5             	mov    %rsp,%rbp
    51a4:	41 57                	push   %r15
    51a6:	41 56                	push   %r14
    51a8:	41 55                	push   %r13
    51aa:	41 54                	push   %r12
    51ac:	53                   	push   %rbx
    51ad:	48 83 ec 08          	sub    $0x8,%rsp
    51b1:	e8 00 00 00 00       	callq  51b6 <scif_send+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    51b6:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    51bd:	49 89 fc             	mov    %rdi,%r12
    51c0:	49 89 f5             	mov    %rsi,%r13
    51c3:	41 89 d6             	mov    %edx,%r14d
    51c6:	41 89 cf             	mov    %ecx,%r15d
    51c9:	48 89 df             	mov    %rbx,%rdi
    51cc:	e8 00 00 00 00       	callq  51d1 <scif_send+0x31>
	int ret;
	get_kref_count(epd);
	ret = __scif_send(epd, msg, len, flags);
    51d1:	44 89 f9             	mov    %r15d,%ecx
    51d4:	44 89 f2             	mov    %r14d,%edx
    51d7:	4c 89 ee             	mov    %r13,%rsi
    51da:	4c 89 e7             	mov    %r12,%rdi
    51dd:	e8 00 00 00 00       	callq  51e2 <scif_send+0x42>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    51e2:	48 89 df             	mov    %rbx,%rdi
    51e5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    51ec:	41 89 c4             	mov    %eax,%r12d
    51ef:	e8 00 00 00 00       	callq  51f4 <scif_send+0x54>
	put_kref_count(epd);
	return ret;
}
    51f4:	48 83 c4 08          	add    $0x8,%rsp
    51f8:	44 89 e0             	mov    %r12d,%eax
    51fb:	5b                   	pop    %rbx
    51fc:	41 5c                	pop    %r12
    51fe:	41 5d                	pop    %r13
    5200:	41 5e                	pop    %r14
    5202:	41 5f                	pop    %r15
    5204:	5d                   	pop    %rbp
    5205:	c3                   	retq   
    5206:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    520d:	00 00 00 

0000000000005210 <__scif_recv>:
 * This function is called from the kernel mode only and is
 * a wrapper for _scif_recv().
 */
int
__scif_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    5210:	55                   	push   %rbp
    5211:	48 89 e5             	mov    %rsp,%rbp
    5214:	41 55                	push   %r13
    5216:	41 54                	push   %r12
    5218:	53                   	push   %rbx
    5219:	48 83 ec 18          	sub    $0x18,%rsp
    521d:	e8 00 00 00 00       	callq  5222 <__scif_recv+0x12>
	struct endpt *ep = (struct endpt *)epd;
	int ret;

	pr_debug("SCIFAPI recv (K): ep %p %s\n", ep, scif_ep_states[ep->state]);
    5222:	8b 07                	mov    (%rdi),%eax

	if (!len)
		return 0;
    5224:	31 c0                	xor    %eax,%eax
	struct endpt *ep = (struct endpt *)epd;
	int ret;

	pr_debug("SCIFAPI recv (K): ep %p %s\n", ep, scif_ep_states[ep->state]);

	if (!len)
    5226:	85 d2                	test   %edx,%edx
    5228:	74 3d                	je     5267 <__scif_recv+0x57>
static inline int
scif_msg_param_check(scif_epd_t epd, int len, int flags)
{
	int ret = -EINVAL;

	if (len < 0)
    522a:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    522f:	78 36                	js     5267 <__scif_recv+0x57>
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    5231:	85 c9                	test   %ecx,%ecx
    5233:	41 89 cd             	mov    %ecx,%r13d
    5236:	48 89 fb             	mov    %rdi,%rbx
    5239:	75 3d                	jne    5278 <__scif_recv+0x68>
	/*
	 * Cannot block while waiting for node to wake up
	 * if non blocking messaging mode is requested. Return
	 * ENODEV if the remote node is idle.
	 */
	if (!flags && ep->remote_dev &&
    523b:	48 8b 87 48 01 00 00 	mov    0x148(%rdi),%rax
    5242:	48 85 c0             	test   %rax,%rax
    5245:	74 16                	je     525d <__scif_recv+0x4d>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    5247:	48 8b 88 88 01 00 00 	mov    0x188(%rax),%rcx
    524e:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    5255:	00 00 80 
    5258:	48 39 c1             	cmp    %rax,%rcx
    525b:	74 5f                	je     52bc <__scif_recv+0xac>
	 * in _scif_send().
	 */
	if (flags & SCIF_RECV_BLOCK)
		mutex_lock(&ep->recvlock);

	ret = _scif_recv(epd, msg, len, flags);
    525d:	31 c9                	xor    %ecx,%ecx
    525f:	48 89 df             	mov    %rbx,%rdi
    5262:	e8 00 00 00 00       	callq  5267 <__scif_recv+0x57>

	if (flags & SCIF_RECV_BLOCK)
		mutex_unlock(&ep->recvlock);

	return ret;
}
    5267:	48 83 c4 18          	add    $0x18,%rsp
    526b:	5b                   	pop    %rbx
    526c:	41 5c                	pop    %r12
    526e:	41 5d                	pop    %r13
    5270:	5d                   	pop    %rbp
    5271:	c3                   	retq   
    5272:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	int ret = -EINVAL;

	if (len < 0)
		goto err_ret;

	if (flags && (!(flags & SCIF_RECV_BLOCK)))
    5278:	f6 c1 01             	test   $0x1,%cl
    527b:	74 ea                	je     5267 <__scif_recv+0x57>
	 * to ensure messages do not get fragmented/reordered.
	 * The non blocking mode is protected using spin locks
	 * in _scif_send().
	 */
	if (flags & SCIF_RECV_BLOCK)
		mutex_lock(&ep->recvlock);
    527d:	4c 8d a7 20 02 00 00 	lea    0x220(%rdi),%r12
    5284:	89 55 d4             	mov    %edx,-0x2c(%rbp)
    5287:	4c 89 e7             	mov    %r12,%rdi
    528a:	48 89 75 d8          	mov    %rsi,-0x28(%rbp)
    528e:	e8 00 00 00 00       	callq  5293 <__scif_recv+0x83>

	ret = _scif_recv(epd, msg, len, flags);
    5293:	8b 55 d4             	mov    -0x2c(%rbp),%edx
    5296:	44 89 e9             	mov    %r13d,%ecx
    5299:	48 89 df             	mov    %rbx,%rdi
    529c:	48 8b 75 d8          	mov    -0x28(%rbp),%rsi
    52a0:	e8 00 00 00 00       	callq  52a5 <__scif_recv+0x95>

	if (flags & SCIF_RECV_BLOCK)
		mutex_unlock(&ep->recvlock);
    52a5:	4c 89 e7             	mov    %r12,%rdi
	 * in _scif_send().
	 */
	if (flags & SCIF_RECV_BLOCK)
		mutex_lock(&ep->recvlock);

	ret = _scif_recv(epd, msg, len, flags);
    52a8:	89 c3                	mov    %eax,%ebx

	if (flags & SCIF_RECV_BLOCK)
		mutex_unlock(&ep->recvlock);
    52aa:	e8 00 00 00 00       	callq  52af <__scif_recv+0x9f>

	return ret;
}
    52af:	48 83 c4 18          	add    $0x18,%rsp
	 * in _scif_send().
	 */
	if (flags & SCIF_RECV_BLOCK)
		mutex_lock(&ep->recvlock);

	ret = _scif_recv(epd, msg, len, flags);
    52b3:	89 d8                	mov    %ebx,%eax

	if (flags & SCIF_RECV_BLOCK)
		mutex_unlock(&ep->recvlock);

	return ret;
}
    52b5:	5b                   	pop    %rbx
    52b6:	41 5c                	pop    %r12
    52b8:	41 5d                	pop    %r13
    52ba:	5d                   	pop    %rbp
    52bb:	c3                   	retq   
	 * ENODEV if the remote node is idle.
	 */
	if (!flags && ep->remote_dev &&
		SCIF_NODE_IDLE == atomic_long_read(
			&ep->remote_dev->scif_ref_cnt))
		return -ENODEV;
    52bc:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
    52c1:	eb a4                	jmp    5267 <__scif_recv+0x57>
    52c3:	66 66 66 66 2e 0f 1f 	data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    52ca:	84 00 00 00 00 00 

00000000000052d0 <scif_recv>:
	return ret;
}

int
scif_recv(scif_epd_t epd, void *msg, int len, int flags)
{
    52d0:	55                   	push   %rbp
    52d1:	48 89 e5             	mov    %rsp,%rbp
    52d4:	41 57                	push   %r15
    52d6:	41 56                	push   %r14
    52d8:	41 55                	push   %r13
    52da:	41 54                	push   %r12
    52dc:	53                   	push   %rbx
    52dd:	48 83 ec 08          	sub    $0x8,%rsp
    52e1:	e8 00 00 00 00       	callq  52e6 <scif_recv+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    52e6:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    52ed:	49 89 fc             	mov    %rdi,%r12
    52f0:	49 89 f5             	mov    %rsi,%r13
    52f3:	41 89 d6             	mov    %edx,%r14d
    52f6:	41 89 cf             	mov    %ecx,%r15d
    52f9:	48 89 df             	mov    %rbx,%rdi
    52fc:	e8 00 00 00 00       	callq  5301 <scif_recv+0x31>
	int ret;
	get_kref_count(epd);
	ret = __scif_recv(epd, msg, len, flags);
    5301:	44 89 f9             	mov    %r15d,%ecx
    5304:	44 89 f2             	mov    %r14d,%edx
    5307:	4c 89 ee             	mov    %r13,%rsi
    530a:	4c 89 e7             	mov    %r12,%rdi
    530d:	e8 00 00 00 00       	callq  5312 <scif_recv+0x42>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    5312:	48 89 df             	mov    %rbx,%rdi
    5315:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    531c:	41 89 c4             	mov    %eax,%r12d
    531f:	e8 00 00 00 00       	callq  5324 <scif_recv+0x54>
	put_kref_count(epd);
	return ret;
}
    5324:	48 83 c4 08          	add    $0x8,%rsp
    5328:	44 89 e0             	mov    %r12d,%eax
    532b:	5b                   	pop    %rbx
    532c:	41 5c                	pop    %r12
    532e:	41 5d                	pop    %r13
    5330:	41 5e                	pop    %r14
    5332:	41 5f                	pop    %r15
    5334:	5d                   	pop    %rbp
    5335:	c3                   	retq   
    5336:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    533d:	00 00 00 

0000000000005340 <__scif_pin_pages>:
 * reference via out_prot.
 */
int
__scif_pin_pages(void *addr, size_t len, int *out_prot,
		int map_flags, scif_pinned_pages_t *pages)
{
    5340:	55                   	push   %rbp
    5341:	48 89 e5             	mov    %rsp,%rbp
    5344:	41 57                	push   %r15
    5346:	41 56                	push   %r14
    5348:	41 55                	push   %r13
    534a:	41 54                	push   %r12
    534c:	53                   	push   %rbx
    534d:	48 83 ec 78          	sub    $0x78,%rsp
    5351:	e8 00 00 00 00       	callq  5356 <__scif_pin_pages+0x16>
	struct scif_pinned_pages *pinned_pages;
	int nr_pages, err = 0, i;
	bool vmalloc_addr = false;
	bool try_upgrade = false;
	int prot = *out_prot;
    5356:	44 8b 22             	mov    (%rdx),%r12d
	int ulimit = 0;
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
    5359:	89 c8                	mov    %ecx,%eax
 * reference via out_prot.
 */
int
__scif_pin_pages(void *addr, size_t len, int *out_prot,
		int map_flags, scif_pinned_pages_t *pages)
{
    535b:	49 89 fe             	mov    %rdi,%r14
    535e:	48 89 f3             	mov    %rsi,%rbx
    5361:	48 89 95 78 ff ff ff 	mov    %rdx,-0x88(%rbp)
	int prot = *out_prot;
	int ulimit = 0;
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
    5368:	83 e0 9f             	and    $0xffffff9f,%eax
 * reference via out_prot.
 */
int
__scif_pin_pages(void *addr, size_t len, int *out_prot,
		int map_flags, scif_pinned_pages_t *pages)
{
    536b:	41 89 cd             	mov    %ecx,%r13d
    536e:	4c 89 45 90          	mov    %r8,-0x70(%rbp)
	int prot = *out_prot;
	int ulimit = 0;
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
    5372:	89 45 c0             	mov    %eax,-0x40(%rbp)
    5375:	0f 85 75 02 00 00    	jne    55f0 <__scif_pin_pages+0x2b0>
	/* Unsupported protection requested */
	if (prot & ~(SCIF_PROT_READ | SCIF_PROT_WRITE))
		return -EINVAL;

	/* addr/len must be page aligned. len should be non zero */
	if ((!len) ||
    537b:	41 f7 c4 fc ff ff ff 	test   $0xfffffffc,%r12d
    5382:	0f 85 68 02 00 00    	jne    55f0 <__scif_pin_pages+0x2b0>
    5388:	48 85 f6             	test   %rsi,%rsi
    538b:	0f 84 5f 02 00 00    	je     55f0 <__scif_pin_pages+0x2b0>
	wait_queue_head_t       wq;
};

static inline uint64_t align_low(uint64_t data, uint32_t granularity)
{
	return ALIGN(data - (granularity - 1), granularity);
    5391:	48 89 f8             	mov    %rdi,%rax
    5394:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
    539a:	48 39 c7             	cmp    %rax,%rdi
    539d:	0f 85 4d 02 00 00    	jne    55f0 <__scif_pin_pages+0x2b0>
    53a3:	48 89 f0             	mov    %rsi,%rax
    53a6:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
		(align_low((uint64_t)addr, PAGE_SIZE) != (uint64_t)addr) ||
    53ac:	48 39 c6             	cmp    %rax,%rsi
    53af:	0f 85 3b 02 00 00    	jne    55f0 <__scif_pin_pages+0x2b0>
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	might_sleep();
    53b5:	e8 00 00 00 00       	callq  53ba <__scif_pin_pages+0x7a>

	nr_pages = (int)(len >> PAGE_SHIFT);
    53ba:	48 c1 eb 0c          	shr    $0xc,%rbx

	/* Allocate a set of pinned pages */
	if (!(pinned_pages = micscif_create_pinned_pages(nr_pages, prot)))
    53be:	44 89 e6             	mov    %r12d,%esi
    53c1:	89 df                	mov    %ebx,%edi
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	might_sleep();

	nr_pages = (int)(len >> PAGE_SHIFT);
    53c3:	48 89 5d 88          	mov    %rbx,-0x78(%rbp)
    53c7:	89 5d c4             	mov    %ebx,-0x3c(%rbp)

	/* Allocate a set of pinned pages */
	if (!(pinned_pages = micscif_create_pinned_pages(nr_pages, prot)))
    53ca:	e8 00 00 00 00       	callq  53cf <__scif_pin_pages+0x8f>
    53cf:	48 85 c0             	test   %rax,%rax
    53d2:	49 89 c7             	mov    %rax,%r15
    53d5:	0f 84 e1 03 00 00    	je     57bc <__scif_pin_pages+0x47c>
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
		return -EINVAL;
	ulimit = !!(map_flags & SCIF_MAP_ULIMIT);
    53db:	44 89 e8             	mov    %r13d,%eax
    53de:	c1 e8 06             	shr    $0x6,%eax
    53e1:	83 e0 01             	and    $0x1,%eax

	/* Allocate a set of pinned pages */
	if (!(pinned_pages = micscif_create_pinned_pages(nr_pages, prot)))
		return -ENOMEM;

	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
    53e4:	41 83 e5 20          	and    $0x20,%r13d
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
		return -EINVAL;
	ulimit = !!(map_flags & SCIF_MAP_ULIMIT);
    53e8:	88 45 87             	mov    %al,-0x79(%rbp)

	/* Allocate a set of pinned pages */
	if (!(pinned_pages = micscif_create_pinned_pages(nr_pages, prot)))
		return -ENOMEM;

	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
    53eb:	0f 85 1e 03 00 00    	jne    570f <__scif_pin_pages+0x3cf>
    53f1:	48 63 45 88          	movslq -0x78(%rbp),%rax
    53f5:	4c 89 75 a8          	mov    %r14,-0x58(%rbp)
			pinned_pages->nr_contig_chunks++;
		}
		pinned_pages->nr_pages = nr_pages;
		pinned_pages->map_flags = SCIF_MAP_KERNEL;
	} else {
		if (prot == SCIF_PROT_READ)
    53f9:	41 83 fc 01          	cmp    $0x1,%r12d
    53fd:	0f 94 45 c8          	sete   -0x38(%rbp)
static inline int __scif_check_inc_pinned_vm(struct mm_struct *mm,
					     int64_t nr_pages)
{
	if (mm && mic_ulimit_check && nr_pages) {
		unsigned long locked, lock_limit;
		locked = nr_pages;
    5401:	48 63 4d c4          	movslq -0x3c(%rbp),%rcx
			try_upgrade = true;
		prot |= SCIF_PROT_WRITE;
    5405:	41 83 cc 02          	or     $0x2,%r12d
    5409:	44 0f b6 75 87       	movzbl -0x79(%rbp),%r14d
    540e:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    5412:	48 89 4d 98          	mov    %rcx,-0x68(%rbp)
    5416:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    541d:	00 00 
    541f:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    5423:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
    5427:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    542e:	00 00 
retry:
		mm = current->mm;
    5430:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
    5434:	48 8b 98 50 02 00 00 	mov    0x250(%rax),%rbx
		down_write(&mm->mmap_sem);
    543b:	4c 8d 6b 60          	lea    0x60(%rbx),%r13
    543f:	4c 89 ef             	mov    %r13,%rdi
    5442:	e8 00 00 00 00       	callq  5447 <__scif_pin_pages+0x107>
		if (ulimit) {
    5447:	45 84 f6             	test   %r14b,%r14b
    544a:	74 4c                	je     5498 <__scif_pin_pages+0x158>
}

static inline int __scif_check_inc_pinned_vm(struct mm_struct *mm,
					     int64_t nr_pages)
{
	if (mm && mic_ulimit_check && nr_pages) {
    544c:	48 85 db             	test   %rbx,%rbx
    544f:	74 47                	je     5498 <__scif_pin_pages+0x158>
    5451:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 5458 <__scif_pin_pages+0x118>
    5458:	74 3e                	je     5498 <__scif_pin_pages+0x158>
    545a:	8b 45 c4             	mov    -0x3c(%rbp),%eax
    545d:	85 c0                	test   %eax,%eax
    545f:	74 37                	je     5498 <__scif_pin_pages+0x158>
#endif /* CONFIG_MM_OWNER */

static inline unsigned long task_rlimit(const struct task_struct *tsk,
		unsigned int limit)
{
	return ACCESS_ONCE(tsk->signal->rlim[limit].rlim_cur);
    5461:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
		unsigned long locked, lock_limit;
		locked = nr_pages;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
		locked += mm->pinned_vm;
    5465:	48 8b 55 98          	mov    -0x68(%rbp),%rdx
    5469:	48 03 93 b0 00 00 00 	add    0xb0(%rbx),%rdx
    5470:	48 8b 80 38 05 00 00 	mov    0x538(%rax),%rax
    5477:	48 8b 80 00 03 00 00 	mov    0x300(%rax),%rax
#else
		locked += mm->locked_vm;
#endif
		lock_limit = rlimit(RLIMIT_MEMLOCK) >> PAGE_SHIFT;
    547e:	48 c1 e8 0c          	shr    $0xc,%rax
		if ((locked > lock_limit) && !capable(CAP_IPC_LOCK)) {
    5482:	48 39 c2             	cmp    %rax,%rdx
    5485:	0f 87 35 01 00 00    	ja     55c0 <__scif_pin_pages+0x280>
			pr_debug("locked(%lu) > lock_limit(%lu)\n", 
				    locked, lock_limit);
			return -ENOMEM;
		} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
			mm->pinned_vm = locked;
    548b:	48 89 93 b0 00 00 00 	mov    %rdx,0xb0(%rbx)
    5492:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
				pinned_pages->nr_pages = 0;
				goto error_unmap;
			}
		}

		pinned_pages->nr_pages = get_user_pages(
    5498:	49 8b 47 38          	mov    0x38(%r15),%rax
    549c:	45 89 e0             	mov    %r12d,%r8d
    549f:	45 31 c9             	xor    %r9d,%r9d
    54a2:	48 89 de             	mov    %rbx,%rsi
    54a5:	8b 4d c4             	mov    -0x3c(%rbp),%ecx
    54a8:	41 d1 e8             	shr    %r8d
    54ab:	48 8b 55 a8          	mov    -0x58(%rbp),%rdx
    54af:	41 83 e0 01          	and    $0x1,%r8d
    54b3:	48 8b 7d a0          	mov    -0x60(%rbp),%rdi
    54b7:	48 89 44 24 08       	mov    %rax,0x8(%rsp)
    54bc:	49 8b 47 20          	mov    0x20(%r15),%rax
    54c0:	48 89 04 24          	mov    %rax,(%rsp)
    54c4:	e8 00 00 00 00       	callq  54c9 <__scif_pin_pages+0x189>
				nr_pages,
				!!(prot & SCIF_PROT_WRITE),
				0,
				pinned_pages->pages,
				pinned_pages->vma);
		up_write(&mm->mmap_sem);
    54c9:	4c 89 ef             	mov    %r13,%rdi
				pinned_pages->nr_pages = 0;
				goto error_unmap;
			}
		}

		pinned_pages->nr_pages = get_user_pages(
    54cc:	48 98                	cltq   
    54ce:	49 89 07             	mov    %rax,(%r15)
				nr_pages,
				!!(prot & SCIF_PROT_WRITE),
				0,
				pinned_pages->pages,
				pinned_pages->vma);
		up_write(&mm->mmap_sem);
    54d1:	e8 00 00 00 00       	callq  54d6 <__scif_pin_pages+0x196>
		if (nr_pages == pinned_pages->nr_pages) {
    54d6:	49 8b 07             	mov    (%r15),%rax
    54d9:	48 3b 45 b8          	cmp    -0x48(%rbp),%rax
    54dd:	0f 84 16 01 00 00    	je     55f9 <__scif_pin_pages+0x2b9>
#ifdef RMA_DEBUG
			atomic_long_add_return(nr_pages, &ms_info.rma_pin_cnt);
#endif
			micscif_detect_large_page(pinned_pages, addr);
		} else {
			if (try_upgrade) {
    54e3:	80 7d c8 00          	cmpb   $0x0,-0x38(%rbp)
    54e7:	74 7f                	je     5568 <__scif_pin_pages+0x228>
				if (ulimit)
    54e9:	45 84 f6             	test   %r14b,%r14b
    54ec:	75 42                	jne    5530 <__scif_pin_pages+0x1f0>
#ifdef RMA_DEBUG
				WARN_ON(atomic_long_sub_return(1,
						&ms_info.rma_mm_cnt) < 0);
#endif
				/* Roll back any pinned pages */
				for (i = 0; i < pinned_pages->nr_pages; i++) {
    54ee:	31 db                	xor    %ebx,%ebx
    54f0:	31 d2                	xor    %edx,%edx
    54f2:	48 85 c0             	test   %rax,%rax
    54f5:	7e 26                	jle    551d <__scif_pin_pages+0x1dd>
    54f7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    54fe:	00 00 
					if (pinned_pages->pages[i])
    5500:	49 8b 47 20          	mov    0x20(%r15),%rax
    5504:	48 8b 3c d0          	mov    (%rax,%rdx,8),%rdi
    5508:	48 85 ff             	test   %rdi,%rdi
    550b:	74 05                	je     5512 <__scif_pin_pages+0x1d2>
						page_cache_release(pinned_pages->pages[i]);
    550d:	e8 00 00 00 00       	callq  5512 <__scif_pin_pages+0x1d2>
#ifdef RMA_DEBUG
				WARN_ON(atomic_long_sub_return(1,
						&ms_info.rma_mm_cnt) < 0);
#endif
				/* Roll back any pinned pages */
				for (i = 0; i < pinned_pages->nr_pages; i++) {
    5512:	83 c3 01             	add    $0x1,%ebx
    5515:	48 63 d3             	movslq %ebx,%rdx
    5518:	49 3b 17             	cmp    (%r15),%rdx
    551b:	7c e3                	jl     5500 <__scif_pin_pages+0x1c0>
					if (pinned_pages->pages[i])
						page_cache_release(pinned_pages->pages[i]);
				}
				prot &= ~SCIF_PROT_WRITE;
    551d:	41 83 e4 fd          	and    $0xfffffffd,%r12d
				try_upgrade = false;
    5521:	c6 45 c8 00          	movb   $0x0,-0x38(%rbp)
				goto retry;
    5525:	e9 06 ff ff ff       	jmpq   5430 <__scif_pin_pages+0xf0>
    552a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
}

static inline int __scif_dec_pinned_vm_lock(struct mm_struct *mm,
					int64_t nr_pages, bool try_lock)
{
	if (mm && nr_pages && mic_ulimit_check) {
    5530:	48 85 db             	test   %rbx,%rbx
    5533:	74 b9                	je     54ee <__scif_pin_pages+0x1ae>
    5535:	48 83 7d b8 00       	cmpq   $0x0,-0x48(%rbp)
    553a:	74 b2                	je     54ee <__scif_pin_pages+0x1ae>
    553c:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 5543 <__scif_pin_pages+0x203>
    5543:	74 a9                	je     54ee <__scif_pin_pages+0x1ae>
		if (try_lock) {
			if (!down_write_trylock(&mm->mmap_sem)) {
				return -1;
			}
		} else {
			down_write(&mm->mmap_sem);
    5545:	4c 89 ef             	mov    %r13,%rdi
    5548:	e8 00 00 00 00       	callq  554d <__scif_pin_pages+0x20d>
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
		mm->pinned_vm -= nr_pages;
    554d:	48 8b 45 98          	mov    -0x68(%rbp),%rax
#else
		mm->locked_vm -= nr_pages;
#endif
		up_write(&mm->mmap_sem);
    5551:	4c 89 ef             	mov    %r13,%rdi
			}
		} else {
			down_write(&mm->mmap_sem);
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
		mm->pinned_vm -= nr_pages;
    5554:	48 29 83 b0 00 00 00 	sub    %rax,0xb0(%rbx)
#else
		mm->locked_vm -= nr_pages;
#endif
		up_write(&mm->mmap_sem);
    555b:	e8 00 00 00 00       	callq  5560 <__scif_pin_pages+0x220>
    5560:	49 8b 07             	mov    (%r15),%rax
    5563:	eb 89                	jmp    54ee <__scif_pin_pages+0x1ae>
    5565:	49 8b 07             	mov    (%r15),%rax
			}
		}
		pinned_pages->map_flags = 0;
	}

	if (pinned_pages->nr_pages < nr_pages) {
    5568:	48 8b 75 b8          	mov    -0x48(%rbp),%rsi
				prot &= ~SCIF_PROT_WRITE;
				try_upgrade = false;
				goto retry;
			}
		}
		pinned_pages->map_flags = 0;
    556c:	41 c7 47 0c 00 00 00 	movl   $0x0,0xc(%r15)
    5573:	00 
	}

	if (pinned_pages->nr_pages < nr_pages) {
    5574:	48 39 f0             	cmp    %rsi,%rax
    5577:	0f 8d 36 01 00 00    	jge    56b3 <__scif_pin_pages+0x373>
	*out_prot = prot;
	atomic_set(&pinned_pages->ref_count, nr_pages);
	*pages = pinned_pages;
	return err;
dec_pinned:
	if (ulimit)
    557d:	80 7d 87 00          	cmpb   $0x0,-0x79(%rbp)
		pinned_pages->map_flags = 0;
	}

	if (pinned_pages->nr_pages < nr_pages) {
		err = -EFAULT;
		pinned_pages->nr_pages = nr_pages;
    5581:	49 89 37             	mov    %rsi,(%r15)
		}
		pinned_pages->map_flags = 0;
	}

	if (pinned_pages->nr_pages < nr_pages) {
		err = -EFAULT;
    5584:	41 bc f2 ff ff ff    	mov    $0xfffffff2,%r12d
	*out_prot = prot;
	atomic_set(&pinned_pages->ref_count, nr_pages);
	*pages = pinned_pages;
	return err;
dec_pinned:
	if (ulimit)
    558a:	0f 85 40 01 00 00    	jne    56d0 <__scif_pin_pages+0x390>
		__scif_dec_pinned_vm_lock(mm, nr_pages, 0);
	/* Something went wrong! Rollback */
error_unmap:
	pinned_pages->nr_pages = nr_pages;
    5590:	48 8b 45 b8          	mov    -0x48(%rbp),%rax
	micscif_destroy_pinned_pages(pinned_pages);
    5594:	4c 89 ff             	mov    %r15,%rdi
dec_pinned:
	if (ulimit)
		__scif_dec_pinned_vm_lock(mm, nr_pages, 0);
	/* Something went wrong! Rollback */
error_unmap:
	pinned_pages->nr_pages = nr_pages;
    5597:	49 89 07             	mov    %rax,(%r15)
	micscif_destroy_pinned_pages(pinned_pages);
    559a:	e8 00 00 00 00       	callq  559f <__scif_pin_pages+0x25f>
	*pages = NULL;
    559f:	48 8b 45 90          	mov    -0x70(%rbp),%rax
	pr_debug("%s %d err %d len 0x%lx\n", __func__, __LINE__, err, len);
	return err;
    55a3:	44 89 65 c0          	mov    %r12d,-0x40(%rbp)
		__scif_dec_pinned_vm_lock(mm, nr_pages, 0);
	/* Something went wrong! Rollback */
error_unmap:
	pinned_pages->nr_pages = nr_pages;
	micscif_destroy_pinned_pages(pinned_pages);
	*pages = NULL;
    55a7:	48 c7 00 00 00 00 00 	movq   $0x0,(%rax)
	pr_debug("%s %d err %d len 0x%lx\n", __func__, __LINE__, err, len);
	return err;

}
    55ae:	8b 45 c0             	mov    -0x40(%rbp),%eax
    55b1:	48 83 c4 78          	add    $0x78,%rsp
    55b5:	5b                   	pop    %rbx
    55b6:	41 5c                	pop    %r12
    55b8:	41 5d                	pop    %r13
    55ba:	41 5e                	pop    %r14
    55bc:	41 5f                	pop    %r15
    55be:	5d                   	pop    %rbp
    55bf:	c3                   	retq   
		locked += mm->pinned_vm;
#else
		locked += mm->locked_vm;
#endif
		lock_limit = rlimit(RLIMIT_MEMLOCK) >> PAGE_SHIFT;
		if ((locked > lock_limit) && !capable(CAP_IPC_LOCK)) {
    55c0:	bf 0e 00 00 00       	mov    $0xe,%edi
    55c5:	48 89 95 70 ff ff ff 	mov    %rdx,-0x90(%rbp)
    55cc:	e8 00 00 00 00       	callq  55d1 <__scif_pin_pages+0x291>
    55d1:	48 8b 95 70 ff ff ff 	mov    -0x90(%rbp),%rdx
    55d8:	84 c0                	test   %al,%al
    55da:	0f 85 ab fe ff ff    	jne    548b <__scif_pin_pages+0x14b>
		mm = current->mm;
		down_write(&mm->mmap_sem);
		if (ulimit) {
			err = __scif_check_inc_pinned_vm(mm, nr_pages);
			if (err) {
				up_write(&mm->mmap_sem);
    55e0:	4c 89 ef             	mov    %r13,%rdi
			pr_debug("locked(%lu) > lock_limit(%lu)\n", 
				    locked, lock_limit);
			return -ENOMEM;
    55e3:	41 bc f4 ff ff ff    	mov    $0xfffffff4,%r12d
    55e9:	e8 00 00 00 00       	callq  55ee <__scif_pin_pages+0x2ae>
				pinned_pages->nr_pages = 0;
				goto error_unmap;
    55ee:	eb a0                	jmp    5590 <__scif_pin_pages+0x250>
	int ulimit = 0;
	struct mm_struct *mm = NULL;

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT))
		return -EINVAL;
    55f0:	c7 45 c0 ea ff ff ff 	movl   $0xffffffea,-0x40(%rbp)
    55f7:	eb b5                	jmp    55ae <__scif_pin_pages+0x26e>
static __always_inline int
micscif_detect_large_page(struct scif_pinned_pages *pinned_pages, char *addr)
{
	int i = 0, nr_pages, huge;
	char *next_huge, *end;
	char *end_addr = addr + (pinned_pages->nr_pages << PAGE_SHIFT);
    55f9:	48 8b 7d b8          	mov    -0x48(%rbp),%rdi
    55fd:	4c 8b 75 a8          	mov    -0x58(%rbp),%r14
    5601:	48 c1 e7 0c          	shl    $0xc,%rdi
    5605:	4c 01 f7             	add    %r14,%rdi

	while (addr < end_addr) {
    5608:	49 39 fe             	cmp    %rdi,%r14
    560b:	0f 83 9e 01 00 00    	jae    57af <__scif_pin_pages+0x46f>
    5611:	31 d2                	xor    %edx,%edx
    5613:	eb 2e                	jmp    5643 <__scif_pin_pages+0x303>
    5615:	0f 1f 00             	nopl   (%rax)
			pinned_pages->num_pages[i] = (int)nr_pages;   
			addr = end;
			i += (int)nr_pages;   
						
		} else {
			pinned_pages->num_pages[i] = 1;
    5618:	49 8b 47 28          	mov    0x28(%r15),%rax
			i++;
    561c:	83 c2 01             	add    $0x1,%edx
			addr += PAGE_SIZE;
    561f:	49 81 c6 00 10 00 00 	add    $0x1000,%r14
			pinned_pages->num_pages[i] = (int)nr_pages;   
			addr = end;
			i += (int)nr_pages;   
						
		} else {
			pinned_pages->num_pages[i] = 1;
    5626:	c7 04 88 01 00 00 00 	movl   $0x1,(%rax,%rcx,4)
			i++;
			addr += PAGE_SIZE;
			ms_info.nr_4k_pages++;
    562d:	48 83 05 00 00 00 00 	addq   $0x1,0x0(%rip)        # 5635 <__scif_pin_pages+0x2f5>
    5634:	01 
		}
		pinned_pages->nr_contig_chunks++;
    5635:	49 83 47 30 01       	addq   $0x1,0x30(%r15)
{
	int i = 0, nr_pages, huge;
	char *next_huge, *end;
	char *end_addr = addr + (pinned_pages->nr_pages << PAGE_SHIFT);

	while (addr < end_addr) {
    563a:	4c 39 f7             	cmp    %r14,%rdi
    563d:	0f 86 22 ff ff ff    	jbe    5565 <__scif_pin_pages+0x225>
 */
static __always_inline int
micscif_is_huge_page(struct scif_pinned_pages *pinned_pages, int index)
{
	int huge = 0;
	struct page *page = pinned_pages->pages[index];
    5643:	49 8b 47 20          	mov    0x20(%r15),%rax
    5647:	48 63 ca             	movslq %edx,%rcx
    564a:	48 8b 04 c8          	mov    (%rax,%rcx,8),%rax
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    564e:	48 8b 30             	mov    (%rax),%rsi
	return (compound_page_dtor *)page[1].lru.next;
}

static inline int compound_order(struct page *page)
{
	if (!PageHead(page))
    5651:	f7 c6 00 40 00 00    	test   $0x4000,%esi
    5657:	74 bf                	je     5618 <__scif_pin_pages+0x2d8>

	if (compound_order(page) + PAGE_SHIFT == SCIF_HUGE_PAGE_SHIFT)
    5659:	83 b8 88 01 00 00 09 	cmpl   $0x9,0x188(%rax)
    5660:	75 b6                	jne    5618 <__scif_pin_pages+0x2d8>
		huge = 1;
	if (huge)
		ms_info.nr_2mb_pages++;
    5662:	48 83 05 00 00 00 00 	addq   $0x1,0x0(%rip)        # 566a <__scif_pin_pages+0x32a>
    5669:	01 
	char *next_huge, *end;
	char *end_addr = addr + (pinned_pages->nr_pages << PAGE_SHIFT);

	while (addr < end_addr) {
		huge = micscif_is_huge_page(pinned_pages, i);
		if (huge) {
    566a:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 5671 <__scif_pin_pages+0x331>
    5671:	74 a5                	je     5618 <__scif_pin_pages+0x2d8>
			next_huge = (char *)ALIGN(
    5673:	49 8d 86 00 00 20 00 	lea    0x200000(%r14),%rax
				(unsigned long)(addr + 1), 
				PMD_SIZE);
			end = next_huge > end_addr ? end_addr : next_huge;
			nr_pages = (int)((end - addr) >> PAGE_SHIFT);
			pinned_pages->num_pages[i] = (int)nr_pages;   
    567a:	4d 8b 47 28          	mov    0x28(%r15),%r8
	char *end_addr = addr + (pinned_pages->nr_pages << PAGE_SHIFT);

	while (addr < end_addr) {
		huge = micscif_is_huge_page(pinned_pages, i);
		if (huge) {
			next_huge = (char *)ALIGN(
    567e:	48 25 00 00 e0 ff    	and    $0xffffffffffe00000,%rax
				(unsigned long)(addr + 1), 
				PMD_SIZE);
			end = next_huge > end_addr ? end_addr : next_huge;
    5684:	48 39 f8             	cmp    %rdi,%rax
    5687:	48 0f 47 c7          	cmova  %rdi,%rax
			nr_pages = (int)((end - addr) >> PAGE_SHIFT);
    568b:	48 89 c6             	mov    %rax,%rsi
    568e:	4c 29 f6             	sub    %r14,%rsi
			pinned_pages->num_pages[i] = (int)nr_pages;   
			addr = end;
    5691:	49 89 c6             	mov    %rax,%r14
		if (huge) {
			next_huge = (char *)ALIGN(
				(unsigned long)(addr + 1), 
				PMD_SIZE);
			end = next_huge > end_addr ? end_addr : next_huge;
			nr_pages = (int)((end - addr) >> PAGE_SHIFT);
    5694:	48 c1 fe 0c          	sar    $0xc,%rsi
			pinned_pages->num_pages[i] = (int)nr_pages;   
    5698:	41 89 34 88          	mov    %esi,(%r8,%rcx,4)
			addr = end;
			i += (int)nr_pages;   
    569c:	01 f2                	add    %esi,%edx
    569e:	eb 95                	jmp    5635 <__scif_pin_pages+0x2f5>
    56a0:	44 8b 65 b8          	mov    -0x48(%rbp),%r12d
				pinned_pages->pages[i] =
					virt_to_page((char *)addr + (i * PAGE_SIZE) );
			pinned_pages->num_pages[i] = 1;
			pinned_pages->nr_contig_chunks++;
		}
		pinned_pages->nr_pages = nr_pages;
    56a4:	48 63 45 88          	movslq -0x78(%rbp),%rax
		pinned_pages->map_flags = SCIF_MAP_KERNEL;
    56a8:	41 c7 47 0c 20 00 00 	movl   $0x20,0xc(%r15)
    56af:	00 
				pinned_pages->pages[i] =
					virt_to_page((char *)addr + (i * PAGE_SIZE) );
			pinned_pages->num_pages[i] = 1;
			pinned_pages->nr_contig_chunks++;
		}
		pinned_pages->nr_pages = nr_pages;
    56b0:	49 89 07             	mov    %rax,(%r15)
		err = -EFAULT;
		pinned_pages->nr_pages = nr_pages;
		goto dec_pinned;
	}

	*out_prot = prot;
    56b3:	48 8b 85 78 ff ff ff 	mov    -0x88(%rbp),%rax
    56ba:	44 89 20             	mov    %r12d,(%rax)
 *
 * Atomically sets the value of @v to @i.
 */
static inline void atomic_set(atomic_t *v, int i)
{
	v->counter = i;
    56bd:	8b 45 88             	mov    -0x78(%rbp),%eax
    56c0:	41 89 47 10          	mov    %eax,0x10(%r15)
	atomic_set(&pinned_pages->ref_count, nr_pages);
	*pages = pinned_pages;
    56c4:	48 8b 45 90          	mov    -0x70(%rbp),%rax
    56c8:	4c 89 38             	mov    %r15,(%rax)
	return err;
    56cb:	e9 de fe ff ff       	jmpq   55ae <__scif_pin_pages+0x26e>
}

static inline int __scif_dec_pinned_vm_lock(struct mm_struct *mm,
					int64_t nr_pages, bool try_lock)
{
	if (mm && nr_pages && mic_ulimit_check) {
    56d0:	48 85 db             	test   %rbx,%rbx
    56d3:	0f 84 b7 fe ff ff    	je     5590 <__scif_pin_pages+0x250>
    56d9:	4c 8b 75 b8          	mov    -0x48(%rbp),%r14
    56dd:	4d 85 f6             	test   %r14,%r14
    56e0:	0f 84 aa fe ff ff    	je     5590 <__scif_pin_pages+0x250>
    56e6:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 56ed <__scif_pin_pages+0x3ad>
    56ed:	0f 84 9d fe ff ff    	je     5590 <__scif_pin_pages+0x250>
		if (try_lock) {
			if (!down_write_trylock(&mm->mmap_sem)) {
				return -1;
			}
		} else {
			down_write(&mm->mmap_sem);
    56f3:	4c 89 ef             	mov    %r13,%rdi
    56f6:	e8 00 00 00 00       	callq  56fb <__scif_pin_pages+0x3bb>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
		mm->pinned_vm -= nr_pages;
#else
		mm->locked_vm -= nr_pages;
#endif
		up_write(&mm->mmap_sem);
    56fb:	4c 89 ef             	mov    %r13,%rdi
			}
		} else {
			down_write(&mm->mmap_sem);
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
		mm->pinned_vm -= nr_pages;
    56fe:	4c 29 b3 b0 00 00 00 	sub    %r14,0xb0(%rbx)
#else
		mm->locked_vm -= nr_pages;
#endif
		up_write(&mm->mmap_sem);
    5705:	e8 00 00 00 00       	callq  570a <__scif_pin_pages+0x3ca>
    570a:	e9 81 fe ff ff       	jmpq   5590 <__scif_pin_pages+0x250>
static inline int is_vmalloc_addr(const void *x)
{
#ifdef CONFIG_MMU
	unsigned long addr = (unsigned long)x;

	return addr >= VMALLOC_START && addr < VMALLOC_END;
    570f:	48 bb 00 00 00 00 00 	movabs $0x370000000000,%rbx
    5716:	37 00 00 

	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
		if (is_vmalloc_addr(addr))
			vmalloc_addr = true;

		for (i = 0; i < nr_pages; i++) {
    5719:	8b 55 88             	mov    -0x78(%rbp),%edx
    571c:	49 8d 04 1e          	lea    (%r14,%rbx,1),%rax
    5720:	4c 89 f3             	mov    %r14,%rbx
    5723:	45 31 f6             	xor    %r14d,%r14d
    5726:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
    572a:	85 d2                	test   %edx,%edx
    572c:	0f 8e 72 ff ff ff    	jle    56a4 <__scif_pin_pages+0x364>
    5732:	44 89 65 b8          	mov    %r12d,-0x48(%rbp)
    5736:	44 8b 6d c4          	mov    -0x3c(%rbp),%r13d
    573a:	49 89 dc             	mov    %rbx,%r12
    573d:	eb 4b                	jmp    578a <__scif_pin_pages+0x44a>
    573f:	90                   	nop
			if (unlikely(vmalloc_addr))
				pinned_pages->pages[i] =
					vmalloc_to_page((char *)addr + (i * PAGE_SIZE) );
			else
				pinned_pages->pages[i] =
					virt_to_page((char *)addr + (i * PAGE_SIZE) );
    5740:	e8 00 00 00 00       	callq  5745 <__scif_pin_pages+0x405>
    5745:	48 b9 00 00 00 00 00 	movabs $0xffffea0000000000,%rcx
    574c:	ea ff ff 
    574f:	48 c1 e8 0c          	shr    $0xc,%rax
    5753:	48 8d 34 80          	lea    (%rax,%rax,4),%rsi
    5757:	48 8d 04 70          	lea    (%rax,%rsi,2),%rax
    575b:	48 c1 e0 05          	shl    $0x5,%rax
    575f:	48 01 c8             	add    %rcx,%rax
    5762:	48 89 03             	mov    %rax,(%rbx)
			pinned_pages->num_pages[i] = 1;
    5765:	49 8b 47 28          	mov    0x28(%r15),%rax
    5769:	49 81 c4 00 10 00 00 	add    $0x1000,%r12
    5770:	42 c7 04 b0 01 00 00 	movl   $0x1,(%rax,%r14,4)
    5777:	00 
    5778:	49 83 c6 01          	add    $0x1,%r14
			pinned_pages->nr_contig_chunks++;
    577c:	49 83 47 30 01       	addq   $0x1,0x30(%r15)

	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
		if (is_vmalloc_addr(addr))
			vmalloc_addr = true;

		for (i = 0; i < nr_pages; i++) {
    5781:	45 39 f5             	cmp    %r14d,%r13d
    5784:	0f 8e 16 ff ff ff    	jle    56a0 <__scif_pin_pages+0x360>
			if (unlikely(vmalloc_addr))
    578a:	48 b9 fe ff ff ff ff 	movabs $0x1ffffffffffe,%rcx
    5791:	1f 00 00 
				pinned_pages->pages[i] =
    5794:	49 8b 47 20          	mov    0x20(%r15),%rax
					vmalloc_to_page((char *)addr + (i * PAGE_SIZE) );
    5798:	4c 89 e7             	mov    %r12,%rdi
	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
		if (is_vmalloc_addr(addr))
			vmalloc_addr = true;

		for (i = 0; i < nr_pages; i++) {
			if (unlikely(vmalloc_addr))
    579b:	48 39 4d c8          	cmp    %rcx,-0x38(%rbp)
				pinned_pages->pages[i] =
    579f:	4a 8d 1c f0          	lea    (%rax,%r14,8),%rbx
	if (unlikely(map_flags & SCIF_MAP_KERNEL)) {
		if (is_vmalloc_addr(addr))
			vmalloc_addr = true;

		for (i = 0; i < nr_pages; i++) {
			if (unlikely(vmalloc_addr))
    57a3:	77 9b                	ja     5740 <__scif_pin_pages+0x400>
				pinned_pages->pages[i] =
					vmalloc_to_page((char *)addr + (i * PAGE_SIZE) );
    57a5:	e8 00 00 00 00       	callq  57aa <__scif_pin_pages+0x46a>
		if (is_vmalloc_addr(addr))
			vmalloc_addr = true;

		for (i = 0; i < nr_pages; i++) {
			if (unlikely(vmalloc_addr))
				pinned_pages->pages[i] =
    57aa:	48 89 03             	mov    %rax,(%rbx)
    57ad:	eb b6                	jmp    5765 <__scif_pin_pages+0x425>
				prot &= ~SCIF_PROT_WRITE;
				try_upgrade = false;
				goto retry;
			}
		}
		pinned_pages->map_flags = 0;
    57af:	41 c7 47 0c 00 00 00 	movl   $0x0,0xc(%r15)
    57b6:	00 
    57b7:	e9 f7 fe ff ff       	jmpq   56b3 <__scif_pin_pages+0x373>

	nr_pages = (int)(len >> PAGE_SHIFT);

	/* Allocate a set of pinned pages */
	if (!(pinned_pages = micscif_create_pinned_pages(nr_pages, prot)))
		return -ENOMEM;
    57bc:	c7 45 c0 f4 ff ff ff 	movl   $0xfffffff4,-0x40(%rbp)
    57c3:	e9 e6 fd ff ff       	jmpq   55ae <__scif_pin_pages+0x26e>
    57c8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    57cf:	00 

00000000000057d0 <scif_pin_pages>:
 * in scif.h
 */
int
scif_pin_pages(void *addr, size_t len, int prot,
		int map_flags, scif_pinned_pages_t *pages)
{
    57d0:	55                   	push   %rbp
    57d1:	48 89 e5             	mov    %rsp,%rbp
    57d4:	48 83 ec 10          	sub    $0x10,%rsp
    57d8:	e8 00 00 00 00       	callq  57dd <scif_pin_pages+0xd>
    57dd:	89 55 fc             	mov    %edx,-0x4(%rbp)
	return __scif_pin_pages(addr, len, &prot, map_flags, pages);
    57e0:	48 8d 55 fc          	lea    -0x4(%rbp),%rdx
    57e4:	e8 00 00 00 00       	callq  57e9 <scif_pin_pages+0x19>
}
    57e9:	c9                   	leaveq 
    57ea:	c3                   	retq   
    57eb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000000057f0 <__scif_register.part.11>:
 *	Upon successful completion, scif_register() returns the offset
 *	at which the mapping was placed else an apt error is returned
 *	as documented in scif.h.
 */
off_t
__scif_register(scif_epd_t epd, void *addr, size_t len, off_t offset,
    57f0:	55                   	push   %rbp
    57f1:	48 89 e5             	mov    %rsp,%rbp
    57f4:	41 57                	push   %r15
    57f6:	41 56                	push   %r14
    57f8:	41 55                	push   %r13
    57fa:	41 54                	push   %r12
    57fc:	53                   	push   %rbx
    57fd:	48 81 ec a8 00 00 00 	sub    $0xa8,%rsp
    5804:	e8 00 00 00 00       	callq  5809 <__scif_register.part.11+0x19>
    5809:	49 89 fd             	mov    %rdi,%r13
    580c:	48 89 b5 68 ff ff ff 	mov    %rsi,-0x98(%rbp)
    5813:	49 89 d4             	mov    %rdx,%r12
    5816:	49 89 cf             	mov    %rcx,%r15
    5819:	45 89 c6             	mov    %r8d,%r14d
    581c:	44 89 cb             	mov    %r9d,%ebx
		(offset < 0) ||
		(offset + (off_t)len < offset)))
		return -EINVAL;


	might_sleep();
    581f:	e8 00 00 00 00       	callq  5824 <__scif_register.part.11+0x34>
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    5824:	41 8b 45 00          	mov    0x0(%r13),%eax
    5828:	49 c7 c2 98 ff ff ff 	mov    $0xffffffffffffff98,%r10
    582f:	83 f8 09             	cmp    $0x9,%eax
    5832:	74 0c                	je     5840 <__scif_register.part.11+0x50>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    5834:	41 8b 45 00          	mov    0x0(%r13),%eax
    5838:	41 b2 95             	mov    $0x95,%r10b
    583b:	83 f8 04             	cmp    $0x4,%eax
    583e:	74 18                	je     5858 <__scif_register.part.11+0x68>
error_unmap:
	micscif_destroy_window(ep, window);
error:
	printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
	return err;
}
    5840:	48 81 c4 a8 00 00 00 	add    $0xa8,%rsp
    5847:	4c 89 d0             	mov    %r10,%rax
    584a:	5b                   	pop    %rbx
    584b:	41 5c                	pop    %r12
    584d:	41 5d                	pop    %r13
    584f:	41 5e                	pop    %r14
    5851:	41 5f                	pop    %r15
    5853:	5d                   	pop    %rbp
    5854:	c3                   	retq   
    5855:	0f 1f 00             	nopl   (%rax)
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    5858:	49 8b 85 48 01 00 00 	mov    0x148(%r13),%rax
	/* Bad EP */
	if (!ep)
		return -EINVAL;
#endif

	if ((err = verify_epd(ep)))
    585f:	41 b2 ed             	mov    $0xed,%r10b
    5862:	8b 40 04             	mov    0x4(%rax),%eax
    5865:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    5868:	83 f8 01             	cmp    $0x1,%eax
    586b:	77 d3                	ja     5840 <__scif_register.part.11+0x50>
    586d:	41 83 bd 5c 01 00 00 	cmpl   $0x2,0x15c(%r13)
    5874:	02 
    5875:	75 c9                	jne    5840 <__scif_register.part.11+0x50>
		return err;

	/* Compute the offset for this registration */
	if ((err = micscif_get_window_offset(ep, map_flags, offset,
    5877:	4c 8d 85 70 ff ff ff 	lea    -0x90(%rbp),%r8
    587e:	4c 89 e1             	mov    %r12,%rcx
    5881:	4c 89 fa             	mov    %r15,%rdx
    5884:	89 de                	mov    %ebx,%esi
    5886:	4c 89 ef             	mov    %r13,%rdi
    5889:	e8 00 00 00 00       	callq  588e <__scif_register.part.11+0x9e>
    588e:	4c 63 d0             	movslq %eax,%r10
    5891:	4d 85 d2             	test   %r10,%r10
    5894:	75 aa                	jne    5840 <__scif_register.part.11+0x50>
			len, &computed_offset)))
		return err;

	/* Allocate and prepare self registration window */
	if (!(window = micscif_create_window(ep, len >> PAGE_SHIFT,
    5896:	48 8b 95 70 ff ff ff 	mov    -0x90(%rbp),%rdx
    589d:	4d 89 e2             	mov    %r12,%r10
    58a0:	31 c9                	xor    %ecx,%ecx
    58a2:	4c 89 ef             	mov    %r13,%rdi
    58a5:	49 c1 ea 0c          	shr    $0xc,%r10
    58a9:	4c 89 d6             	mov    %r10,%rsi
    58ac:	4c 89 95 60 ff ff ff 	mov    %r10,-0xa0(%rbp)
    58b3:	e8 00 00 00 00       	callq  58b8 <__scif_register.part.11+0xc8>
    58b8:	4c 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10
    58bf:	48 85 c0             	test   %rax,%rax
    58c2:	49 89 c7             	mov    %rax,%r15
    58c5:	0f 84 d7 03 00 00    	je     5ca2 <__scif_register.part.11+0x4b2>
			computed_offset, false))) {
		micscif_free_window_offset(ep, computed_offset, len);
		return -ENOMEM;
	}

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    58cb:	4d 8b 85 48 01 00 00 	mov    0x148(%r13),%r8
    58d2:	4d 85 c0             	test   %r8,%r8
    58d5:	0f 85 45 02 00 00    	jne    5b20 <__scif_register.part.11+0x330>

	window->nr_pages = len >> PAGE_SHIFT;
    58db:	4d 89 17             	mov    %r10,(%r15)

	if ((err = micscif_send_alloc_request(ep, window))) {
    58de:	4c 89 fe             	mov    %r15,%rsi
    58e1:	4c 89 ef             	mov    %r13,%rdi
    58e4:	e8 00 00 00 00       	callq  58e9 <__scif_register.part.11+0xf9>
    58e9:	4c 63 d8             	movslq %eax,%r11
    58ec:	4d 85 db             	test   %r11,%r11
    58ef:	0f 85 13 01 00 00    	jne    5a08 <__scif_register.part.11+0x218>
	scif_pinned_pages_t pinned_pages;
	off_t err;
	struct endpt *ep = (struct endpt *)epd;
	uint64_t computed_offset;
	struct reg_range_t *window;
	struct mm_struct *mm = NULL;
    58f5:	45 31 d2             	xor    %r10d,%r10d
		micscif_destroy_incomplete_window(ep, window);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		return err;
	}

	if (!(map_flags & SCIF_MAP_KERNEL)) {
    58f8:	f6 c3 20             	test   $0x20,%bl
    58fb:	75 10                	jne    590d <__scif_register.part.11+0x11d>

/* Debug API's */
void micscif_display_window(struct reg_range_t *window, const char *s, int line);
static inline struct mm_struct *__scif_acquire_mm(void)
{
	if (mic_ulimit_check) {
    58fd:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 5904 <__scif_register.part.11+0x114>
    5904:	0f 85 f6 01 00 00    	jne    5b00 <__scif_register.part.11+0x310>
		mm = __scif_acquire_mm();
		map_flags |= SCIF_MAP_ULIMIT;
    590a:	83 cb 40             	or     $0x40,%ebx
 */
int
scif_pin_pages(void *addr, size_t len, int prot,
		int map_flags, scif_pinned_pages_t *pages)
{
	return __scif_pin_pages(addr, len, &prot, map_flags, pages);
    590d:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    5914:	4c 8d 45 a0          	lea    -0x60(%rbp),%r8
	if (!(map_flags & SCIF_MAP_KERNEL)) {
		mm = __scif_acquire_mm();
		map_flags |= SCIF_MAP_ULIMIT;
	}
	/* Pin down the pages */
	if ((err = scif_pin_pages(addr, len, prot,
    5918:	89 d9                	mov    %ebx,%ecx
 */
int
scif_pin_pages(void *addr, size_t len, int prot,
		int map_flags, scif_pinned_pages_t *pages)
{
	return __scif_pin_pages(addr, len, &prot, map_flags, pages);
    591a:	4c 89 e6             	mov    %r12,%rsi
	if (!(map_flags & SCIF_MAP_KERNEL)) {
		mm = __scif_acquire_mm();
		map_flags |= SCIF_MAP_ULIMIT;
	}
	/* Pin down the pages */
	if ((err = scif_pin_pages(addr, len, prot,
    591d:	83 e1 60             	and    $0x60,%ecx
    5920:	4c 89 95 60 ff ff ff 	mov    %r10,-0xa0(%rbp)
 */
int
scif_pin_pages(void *addr, size_t len, int prot,
		int map_flags, scif_pinned_pages_t *pages)
{
	return __scif_pin_pages(addr, len, &prot, map_flags, pages);
    5927:	48 8d 95 78 ff ff ff 	lea    -0x88(%rbp),%rdx
    592e:	44 89 b5 78 ff ff ff 	mov    %r14d,-0x88(%rbp)
    5935:	e8 00 00 00 00       	callq  593a <__scif_register.part.11+0x14a>
	if (!(map_flags & SCIF_MAP_KERNEL)) {
		mm = __scif_acquire_mm();
		map_flags |= SCIF_MAP_ULIMIT;
	}
	/* Pin down the pages */
	if ((err = scif_pin_pages(addr, len, prot,
    593a:	4c 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10
    5941:	48 63 d8             	movslq %eax,%rbx
    5944:	48 85 db             	test   %rbx,%rbx
    5947:	0f 85 ff 04 00 00    	jne    5e4c <__scif_register.part.11+0x65c>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		__scif_release_mm(mm);
		goto error;
	}

	window->pinned_pages = pinned_pages;
    594d:	48 8b 45 a0          	mov    -0x60(%rbp),%rax
	window->nr_contig_chunks = pinned_pages->nr_contig_chunks;
	window->prot = pinned_pages->prot;
	window->mm = mm;

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
    5951:	4c 89 fe             	mov    %r15,%rsi
    5954:	4c 89 ef             	mov    %r13,%rdi
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		__scif_release_mm(mm);
		goto error;
	}

	window->pinned_pages = pinned_pages;
    5957:	49 89 47 66          	mov    %rax,0x66(%r15)
	window->nr_contig_chunks = pinned_pages->nr_contig_chunks;
    595b:	48 8b 50 30          	mov    0x30(%rax),%rdx
    595f:	49 89 57 08          	mov    %rdx,0x8(%r15)
	window->prot = pinned_pages->prot;
    5963:	8b 40 08             	mov    0x8(%rax),%eax
	window->mm = mm;
    5966:	4d 89 97 f6 00 00 00 	mov    %r10,0xf6(%r15)
		goto error;
	}

	window->pinned_pages = pinned_pages;
	window->nr_contig_chunks = pinned_pages->nr_contig_chunks;
	window->prot = pinned_pages->prot;
    596d:	41 89 47 10          	mov    %eax,0x10(%r15)
	window->mm = mm;

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
    5971:	e8 00 00 00 00       	callq  5976 <__scif_register.part.11+0x186>
    5976:	48 63 d8             	movslq %eax,%rbx
    5979:	48 85 db             	test   %rbx,%rbx
    597c:	0f 85 0f 06 00 00    	jne    5f91 <__scif_register.part.11+0x7a1>
		printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
		goto error_unmap;
	}

	/* Tell the peer about the new window */
	if ((err = micscif_send_scif_register(ep, window))) {
    5982:	4c 89 fe             	mov    %r15,%rsi
    5985:	4c 89 ef             	mov    %r13,%rdi
    5988:	e8 00 00 00 00       	callq  598d <__scif_register.part.11+0x19d>
    598d:	48 63 d8             	movslq %eax,%rbx
    5990:	48 85 db             	test   %rbx,%rbx
    5993:	0f 85 70 05 00 00    	jne    5f09 <__scif_register.part.11+0x719>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
		goto error_unmap;
	}

	micscif_dec_node_refcnt(ep->remote_dev, 1);
    5999:	49 8b 9d 48 01 00 00 	mov    0x148(%r13),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    59a0:	48 85 db             	test   %rbx,%rbx
    59a3:	74 21                	je     59c6 <__scif_register.part.11+0x1d6>
		if (unlikely((atomic_long_sub_return(cnt, 
    59a5:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    59ac:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    59b3:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    59ba:	00 00 
    59bc:	48 83 e8 01          	sub    $0x1,%rax
    59c0:	0f 88 ba 01 00 00    	js     5b80 <__scif_register.part.11+0x390>

	/* No further failures expected. Insert new window */
	mutex_lock(&ep->rma_info.rma_lock);
    59c6:	49 8d 9d 80 00 00 00 	lea    0x80(%r13),%rbx
    59cd:	48 89 df             	mov    %rbx,%rdi
    59d0:	e8 00 00 00 00       	callq  59d5 <__scif_register.part.11+0x1e5>
	set_window_ref_count(window, pinned_pages->nr_pages);
    59d5:	48 8b 45 a0          	mov    -0x60(%rbp),%rax
	micscif_insert_window(window, &ep->rma_info.reg_list);
    59d9:	49 8d 75 28          	lea    0x28(%r13),%rsi
    59dd:	4c 89 ff             	mov    %r15,%rdi
}

static __always_inline void
set_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
    window->ref_count = (int)nr_pages; 
    59e0:	48 8b 00             	mov    (%rax),%rax
    59e3:	41 89 47 14          	mov    %eax,0x14(%r15)
    59e7:	e8 00 00 00 00       	callq  59ec <__scif_register.part.11+0x1fc>
	mutex_unlock(&ep->rma_info.rma_lock);
    59ec:	48 89 df             	mov    %rbx,%rdi
    59ef:	e8 00 00 00 00       	callq  59f4 <__scif_register.part.11+0x204>

	pr_debug("SCIFAPI register: ep %p %s addr %p"
    59f4:	41 8b 45 00          	mov    0x0(%r13),%eax
		" len 0x%lx computed_offset 0x%llx\n", 
		epd, scif_ep_states[epd->state], addr, len, computed_offset);
	return computed_offset;
    59f8:	4c 8b 95 70 ff ff ff 	mov    -0x90(%rbp),%r10
    59ff:	e9 3c fe ff ff       	jmpq   5840 <__scif_register.part.11+0x50>
    5a04:	0f 1f 40 00          	nopl   0x0(%rax)
	micscif_inc_node_refcnt(ep->remote_dev, 1);

	window->nr_pages = len >> PAGE_SHIFT;

	if ((err = micscif_send_alloc_request(ep, window))) {
		micscif_destroy_incomplete_window(ep, window);
    5a08:	4c 89 fe             	mov    %r15,%rsi
    5a0b:	4c 89 ef             	mov    %r13,%rdi
    5a0e:	4c 89 9d 68 ff ff ff 	mov    %r11,-0x98(%rbp)
    5a15:	e8 00 00 00 00       	callq  5a1a <__scif_register.part.11+0x22a>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    5a1a:	49 8b 9d 48 01 00 00 	mov    0x148(%r13),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    5a21:	4c 8b 9d 68 ff ff ff 	mov    -0x98(%rbp),%r11
    5a28:	48 85 db             	test   %rbx,%rbx
    5a2b:	4d 89 da             	mov    %r11,%r10
    5a2e:	0f 84 0c fe ff ff    	je     5840 <__scif_register.part.11+0x50>
		if (unlikely((atomic_long_sub_return(cnt, 
    5a34:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
    5a3b:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    5a42:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    5a49:	00 00 
    5a4b:	48 83 e8 01          	sub    $0x1,%rax
    5a4f:	0f 89 eb fd ff ff    	jns    5840 <__scif_register.part.11+0x50>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    5a55:	48 8b 45 08          	mov    0x8(%rbp),%rax
    5a59:	48 89 d9             	mov    %rbx,%rcx
    5a5c:	ba a7 00 00 00       	mov    $0xa7,%edx
    5a61:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    5a68:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    5a6f:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5a76:	4c 89 9d 60 ff ff ff 	mov    %r11,-0xa0(%rbp)
    5a7d:	44 0f b7 03          	movzwl (%rbx),%r8d
    5a81:	4c 89 9d 68 ff ff ff 	mov    %r11,-0x98(%rbp)
    5a88:	48 89 04 24          	mov    %rax,(%rsp)
    5a8c:	31 c0                	xor    %eax,%eax
    5a8e:	e8 00 00 00 00       	callq  5a93 <__scif_register.part.11+0x2a3>
    5a93:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5a9a:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    5aa1:	00 00 80 
    5aa4:	4c 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10
    5aab:	48 39 f1             	cmp    %rsi,%rcx
    5aae:	0f 84 8c fd ff ff    	je     5840 <__scif_register.part.11+0x50>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5ab4:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5ab8:	48 89 c8             	mov    %rcx,%rax
    5abb:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    5ac2:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5ac4:	4c 8b 9d 68 ff ff ff 	mov    -0x98(%rbp),%r11
    5acb:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5ace:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5ad1:	4d 89 da             	mov    %r11,%r10
    5ad4:	0f 84 66 fd ff ff    	je     5840 <__scif_register.part.11+0x50>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5ada:	48 39 f2             	cmp    %rsi,%rdx
    5add:	74 16                	je     5af5 <__scif_register.part.11+0x305>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5adf:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5ae3:	48 89 d0             	mov    %rdx,%rax
    5ae6:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5aec:	48 39 c2             	cmp    %rax,%rdx
    5aef:	0f 85 4f 03 00 00    	jne    5e44 <__scif_register.part.11+0x654>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5af5:	4d 89 da             	mov    %r11,%r10
    5af8:	e9 43 fd ff ff       	jmpq   5840 <__scif_register.part.11+0x50>
    5afd:	0f 1f 00             	nopl   (%rax)
    5b00:	65 48 8b 3c 25 00 00 	mov    %gs:0x0,%rdi
    5b07:	00 00 
{
	if (mic_ulimit_check) {
#ifdef RMA_DEBUG
		atomic_long_add_return(1, &ms_info.rma_mm_cnt);
#endif
		return get_task_mm(current);
    5b09:	e8 00 00 00 00       	callq  5b0e <__scif_register.part.11+0x31e>
    5b0e:	49 89 c2             	mov    %rax,%r10
    5b11:	e9 f4 fd ff ff       	jmpq   590a <__scif_register.part.11+0x11a>
    5b16:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    5b1d:	00 00 00 
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    5b20:	49 8b 88 88 01 00 00 	mov    0x188(%r8),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    5b27:	4d 8d 88 88 01 00 00 	lea    0x188(%r8),%r9
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5b2e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    5b35:	00 00 80 
    5b38:	48 39 f1             	cmp    %rsi,%rcx
    5b3b:	0f 84 cf 00 00 00    	je     5c10 <__scif_register.part.11+0x420>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5b41:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5b45:	48 89 c8             	mov    %rcx,%rax
    5b48:	f0 49 0f b1 90 88 01 	lock cmpxchg %rdx,0x188(%r8)
    5b4f:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5b51:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5b54:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5b57:	0f 84 7e fd ff ff    	je     58db <__scif_register.part.11+0xeb>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5b5d:	48 39 f2             	cmp    %rsi,%rdx
    5b60:	0f 84 aa 00 00 00    	je     5c10 <__scif_register.part.11+0x420>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5b66:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5b6a:	48 89 d0             	mov    %rdx,%rax
    5b6d:	f0 49 0f b1 09       	lock cmpxchg %rcx,(%r9)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5b72:	48 39 d0             	cmp    %rdx,%rax
    5b75:	0f 84 60 fd ff ff    	je     58db <__scif_register.part.11+0xeb>
    5b7b:	48 89 c2             	mov    %rax,%rdx
    5b7e:	eb dd                	jmp    5b5d <__scif_register.part.11+0x36d>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    5b80:	48 8b 45 08          	mov    0x8(%rbp),%rax
    5b84:	48 89 d9             	mov    %rbx,%rcx
    5b87:	ba a7 00 00 00       	mov    $0xa7,%edx
    5b8c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    5b93:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    5b9a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5ba1:	44 0f b7 03          	movzwl (%rbx),%r8d
    5ba5:	48 89 04 24          	mov    %rax,(%rsp)
    5ba9:	31 c0                	xor    %eax,%eax
    5bab:	e8 00 00 00 00       	callq  5bb0 <__scif_register.part.11+0x3c0>
    5bb0:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5bb7:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    5bbe:	00 00 80 
    5bc1:	48 39 f1             	cmp    %rsi,%rcx
    5bc4:	0f 84 fc fd ff ff    	je     59c6 <__scif_register.part.11+0x1d6>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5bca:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5bce:	48 89 c8             	mov    %rcx,%rax
    5bd1:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    5bd8:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5bda:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5bdd:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5be0:	0f 84 e0 fd ff ff    	je     59c6 <__scif_register.part.11+0x1d6>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    5be6:	48 39 f2             	cmp    %rsi,%rdx
    5be9:	0f 84 d7 fd ff ff    	je     59c6 <__scif_register.part.11+0x1d6>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    5bef:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    5bf3:	48 89 d0             	mov    %rdx,%rax
    5bf6:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    5bfc:	48 39 c2             	cmp    %rax,%rdx
    5bff:	0f 84 c1 fd ff ff    	je     59c6 <__scif_register.part.11+0x1d6>
    5c05:	48 89 c2             	mov    %rax,%rdx
    5c08:	eb dc                	jmp    5be6 <__scif_register.part.11+0x3f6>
    5c0a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    5c10:	49 8d 80 68 01 00 00 	lea    0x168(%r8),%rax
    5c17:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    5c1e:	48 89 c7             	mov    %rax,%rdi
    5c21:	4c 89 85 60 ff ff ff 	mov    %r8,-0xa0(%rbp)
    5c28:	48 89 85 58 ff ff ff 	mov    %rax,-0xa8(%rbp)
    5c2f:	4c 89 8d 48 ff ff ff 	mov    %r9,-0xb8(%rbp)
    5c36:	e8 00 00 00 00       	callq  5c3b <__scif_register.part.11+0x44b>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    5c3b:	4c 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%r8
    5c42:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    5c49:	41 8b 40 04          	mov    0x4(%r8),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    5c4d:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    5c50:	83 fa 01             	cmp    $0x1,%edx
    5c53:	76 2e                	jbe    5c83 <__scif_register.part.11+0x493>
    5c55:	83 f8 01             	cmp    $0x1,%eax
    5c58:	4c 8b 8d 48 ff ff ff 	mov    -0xb8(%rbp),%r9
    5c5f:	74 22                	je     5c83 <__scif_register.part.11+0x493>
    5c61:	49 8b 80 88 01 00 00 	mov    0x188(%r8),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    5c68:	48 85 c0             	test   %rax,%rax
    5c6b:	78 53                	js     5cc0 <__scif_register.part.11+0x4d0>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    5c6d:	4c 89 cf             	mov    %r9,%rdi
    5c70:	4c 89 95 60 ff ff ff 	mov    %r10,-0xa0(%rbp)
    5c77:	e8 74 a8 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
    5c7c:	4c 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10
bail_out:
		mutex_unlock(&dev->sd_lock);
    5c83:	48 8b bd 58 ff ff ff 	mov    -0xa8(%rbp),%rdi
    5c8a:	4c 89 95 60 ff ff ff 	mov    %r10,-0xa0(%rbp)
    5c91:	e8 00 00 00 00       	callq  5c96 <__scif_register.part.11+0x4a6>
    5c96:	4c 8b 95 60 ff ff ff 	mov    -0xa0(%rbp),%r10
    5c9d:	e9 39 fc ff ff       	jmpq   58db <__scif_register.part.11+0xeb>
		return err;

	/* Allocate and prepare self registration window */
	if (!(window = micscif_create_window(ep, len >> PAGE_SHIFT,
			computed_offset, false))) {
		micscif_free_window_offset(ep, computed_offset, len);
    5ca2:	48 8b b5 70 ff ff ff 	mov    -0x90(%rbp),%rsi
    5ca9:	4c 89 e2             	mov    %r12,%rdx
    5cac:	4c 89 ef             	mov    %r13,%rdi
    5caf:	e8 00 00 00 00       	callq  5cb4 <__scif_register.part.11+0x4c4>
		return -ENOMEM;
    5cb4:	49 c7 c2 f4 ff ff ff 	mov    $0xfffffffffffffff4,%r10
    5cbb:	e9 80 fb ff ff       	jmpq   5840 <__scif_register.part.11+0x50>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    5cc0:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 5cc6 <__scif_register.part.11+0x4d6>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5cc6:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    5cc8:	49 c7 80 b0 01 00 00 	movq   $0x2,0x1b0(%r8)
    5ccf:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5cd3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    5cda:	c7 45 a8 2e 00 00 00 	movl   $0x2e,-0x58(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5ce1:	48 8d 75 a0          	lea    -0x60(%rbp),%rsi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    5ce5:	66 c7 45 a4 00 00    	movw   $0x0,-0x5c(%rbp)
    5ceb:	4c 89 8d 48 ff ff ff 	mov    %r9,-0xb8(%rbp)
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    5cf2:	66 89 45 a0          	mov    %ax,-0x60(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    5cf6:	41 0f b7 00          	movzwl (%r8),%eax
    5cfa:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    5d01:	4c 89 85 60 ff ff ff 	mov    %r8,-0xa0(%rbp)
    5d08:	48 89 45 ac          	mov    %rax,-0x54(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    5d0c:	e8 00 00 00 00       	callq  5d11 <__scif_register.part.11+0x521>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    5d11:	4c 8b 85 60 ff ff ff 	mov    -0xa0(%rbp),%r8
    5d18:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    5d1f:	4c 8b 8d 48 ff ff ff 	mov    -0xb8(%rbp),%r9
    5d26:	49 83 b8 b0 01 00 00 	cmpq   $0x2,0x1b0(%r8)
    5d2d:	02 
    5d2e:	74 24                	je     5d54 <__scif_register.part.11+0x564>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    5d30:	49 83 b8 b0 01 00 00 	cmpq   $0x3,0x1b0(%r8)
    5d37:	03 
    5d38:	0f 85 2f ff ff ff    	jne    5c6d <__scif_register.part.11+0x47d>
				dev->sd_state = SCIFDEV_RUNNING;
    5d3e:	41 c7 40 04 02 00 00 	movl   $0x2,0x4(%r8)
    5d45:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    5d46:	f0 41 80 a0 8f 01 00 	lock andb $0x7f,0x18f(%r8)
    5d4d:	00 7f 
    5d4f:	e9 19 ff ff ff       	jmpq   5c6d <__scif_register.part.11+0x47d>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    5d54:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    5d5b:	31 c0                	xor    %eax,%eax
    5d5d:	b9 0a 00 00 00       	mov    $0xa,%ecx
    5d62:	f3 ab                	rep stos %eax,%es:(%rdi)
    5d64:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    5d6b:	00 
    5d6c:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    5d73:	00 00 
    5d75:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    5d79:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    5d80:	48 83 c0 18          	add    $0x18,%rax
    5d84:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    5d88:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    5d8c:	49 8d 80 98 01 00 00 	lea    0x198(%r8),%rax
    5d93:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    5d9a:	eb 1a                	jmp    5db6 <__scif_register.part.11+0x5c6>
    5d9c:	e8 00 00 00 00       	callq  5da1 <__scif_register.part.11+0x5b1>
    5da1:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    5da8:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    5daf:	4c 8b 8d 40 ff ff ff 	mov    -0xc0(%rbp),%r9
    5db6:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    5dbd:	ba 02 00 00 00       	mov    $0x2,%edx
    5dc2:	4c 89 8d 40 ff ff ff 	mov    %r9,-0xc0(%rbp)
    5dc9:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    5dd0:	4c 89 85 48 ff ff ff 	mov    %r8,-0xb8(%rbp)
    5dd7:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    5dde:	e8 00 00 00 00       	callq  5de3 <__scif_register.part.11+0x5f3>
    5de3:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    5dea:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    5df1:	4c 8b 8d 40 ff ff ff 	mov    -0xc0(%rbp),%r9
    5df8:	49 83 b8 b0 01 00 00 	cmpq   $0x2,0x1b0(%r8)
    5dff:	02 
    5e00:	74 9a                	je     5d9c <__scif_register.part.11+0x5ac>
    5e02:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    5e09:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    5e10:	4c 89 8d 40 ff ff ff 	mov    %r9,-0xc0(%rbp)
    5e17:	4c 89 85 48 ff ff ff 	mov    %r8,-0xb8(%rbp)
    5e1e:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    5e25:	e8 00 00 00 00       	callq  5e2a <__scif_register.part.11+0x63a>
    5e2a:	4c 8b 8d 40 ff ff ff 	mov    -0xc0(%rbp),%r9
    5e31:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    5e38:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    5e3f:	e9 ec fe ff ff       	jmpq   5d30 <__scif_register.part.11+0x540>
    5e44:	48 89 c2             	mov    %rax,%rdx
    5e47:	e9 8e fc ff ff       	jmpq   5ada <__scif_register.part.11+0x2ea>
	}
	/* Pin down the pages */
	if ((err = scif_pin_pages(addr, len, prot,
			map_flags & (SCIF_MAP_KERNEL | SCIF_MAP_ULIMIT),
			&pinned_pages))) {
		micscif_destroy_incomplete_window(ep, window);
    5e4c:	4c 89 fe             	mov    %r15,%rsi
    5e4f:	4c 89 ef             	mov    %r13,%rdi
    5e52:	4c 89 95 68 ff ff ff 	mov    %r10,-0x98(%rbp)
    5e59:	e8 00 00 00 00       	callq  5e5e <__scif_register.part.11+0x66e>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    5e5e:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    5e65:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
    5e6c:	4d 85 e4             	test   %r12,%r12
    5e6f:	74 5d                	je     5ece <__scif_register.part.11+0x6de>
		if (unlikely((atomic_long_sub_return(cnt, 
    5e71:	4d 8d ac 24 88 01 00 	lea    0x188(%r12),%r13
    5e78:	00 
    5e79:	4c 89 ef             	mov    %r13,%rdi
    5e7c:	e8 4f a6 ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    5e81:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
    5e88:	48 85 c0             	test   %rax,%rax
    5e8b:	79 41                	jns    5ece <__scif_register.part.11+0x6de>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    5e8d:	48 8b 45 08          	mov    0x8(%rbp),%rax
    5e91:	4c 89 e1             	mov    %r12,%rcx
    5e94:	ba a7 00 00 00       	mov    $0xa7,%edx
    5e99:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    5ea0:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    5ea7:	00 
    5ea8:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5eaf:	45 0f b7 04 24       	movzwl (%r12),%r8d
    5eb4:	48 89 04 24          	mov    %rax,(%rsp)
    5eb8:	31 c0                	xor    %eax,%eax
    5eba:	e8 00 00 00 00       	callq  5ebf <__scif_register.part.11+0x6cf>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    5ebf:	4c 89 ef             	mov    %r13,%rdi
    5ec2:	e8 99 a5 ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
    5ec7:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
	return NULL;
}

static inline void __scif_release_mm(struct mm_struct *mm)
{
	if (mic_ulimit_check && mm) {
    5ece:	4d 85 d2             	test   %r10,%r10
    5ed1:	74 11                	je     5ee4 <__scif_register.part.11+0x6f4>
    5ed3:	80 3d 00 00 00 00 00 	cmpb   $0x0,0x0(%rip)        # 5eda <__scif_register.part.11+0x6ea>
    5eda:	74 08                	je     5ee4 <__scif_register.part.11+0x6f4>
#ifdef RMA_DEBUG
		WARN_ON(atomic_long_sub_return(1, &ms_info.rma_mm_cnt) < 0);
#endif
		mmput(mm);
    5edc:	4c 89 d7             	mov    %r10,%rdi
    5edf:	e8 00 00 00 00       	callq  5ee4 <__scif_register.part.11+0x6f4>
		epd, scif_ep_states[epd->state], addr, len, computed_offset);
	return computed_offset;
error_unmap:
	micscif_destroy_window(ep, window);
error:
	printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
    5ee4:	48 89 d9             	mov    %rbx,%rcx
    5ee7:	ba e2 09 00 00       	mov    $0x9e2,%edx
    5eec:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    5ef3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5efa:	31 c0                	xor    %eax,%eax
    5efc:	e8 00 00 00 00       	callq  5f01 <__scif_register.part.11+0x711>
	return err;
    5f01:	49 89 da             	mov    %rbx,%r10
    5f04:	e9 37 f9 ff ff       	jmpq   5840 <__scif_register.part.11+0x50>
		goto error_unmap;
	}

	/* Tell the peer about the new window */
	if ((err = micscif_send_scif_register(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    5f09:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    5f10:	4d 85 e4             	test   %r12,%r12
    5f13:	74 4f                	je     5f64 <__scif_register.part.11+0x774>
		if (unlikely((atomic_long_sub_return(cnt, 
    5f15:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    5f1c:	00 
    5f1d:	4c 89 f7             	mov    %r14,%rdi
    5f20:	e8 ab a5 ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    5f25:	48 85 c0             	test   %rax,%rax
    5f28:	79 3a                	jns    5f64 <__scif_register.part.11+0x774>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    5f2a:	48 8b 45 08          	mov    0x8(%rbp),%rax
    5f2e:	4c 89 e1             	mov    %r12,%rcx
    5f31:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5f38:	ba a7 00 00 00       	mov    $0xa7,%edx
    5f3d:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    5f44:	00 
    5f45:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    5f4c:	45 0f b7 04 24       	movzwl (%r12),%r8d
    5f51:	48 89 04 24          	mov    %rax,(%rsp)
    5f55:	31 c0                	xor    %eax,%eax
    5f57:	e8 00 00 00 00       	callq  5f5c <__scif_register.part.11+0x76c>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    5f5c:	4c 89 f7             	mov    %r14,%rdi
    5f5f:	e8 fc a4 ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
		printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
    5f64:	48 89 d9             	mov    %rbx,%rcx
    5f67:	ba cf 09 00 00       	mov    $0x9cf,%edx
    5f6c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    5f73:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5f7a:	31 c0                	xor    %eax,%eax
    5f7c:	e8 00 00 00 00       	callq  5f81 <__scif_register.part.11+0x791>
	pr_debug("SCIFAPI register: ep %p %s addr %p"
		" len 0x%lx computed_offset 0x%llx\n", 
		epd, scif_ep_states[epd->state], addr, len, computed_offset);
	return computed_offset;
error_unmap:
	micscif_destroy_window(ep, window);
    5f81:	4c 89 fe             	mov    %r15,%rsi
    5f84:	4c 89 ef             	mov    %r13,%rdi
    5f87:	e8 00 00 00 00       	callq  5f8c <__scif_register.part.11+0x79c>
    5f8c:	e9 53 ff ff ff       	jmpq   5ee4 <__scif_register.part.11+0x6f4>
	window->prot = pinned_pages->prot;
	window->mm = mm;

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    5f91:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    5f98:	4d 85 e4             	test   %r12,%r12
    5f9b:	74 4f                	je     5fec <__scif_register.part.11+0x7fc>
		if (unlikely((atomic_long_sub_return(cnt, 
    5f9d:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    5fa4:	00 
    5fa5:	4c 89 f7             	mov    %r14,%rdi
    5fa8:	e8 23 a5 ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    5fad:	48 85 c0             	test   %rax,%rax
    5fb0:	79 3a                	jns    5fec <__scif_register.part.11+0x7fc>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    5fb2:	48 8b 45 08          	mov    0x8(%rbp),%rax
    5fb6:	4c 89 e1             	mov    %r12,%rcx
    5fb9:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    5fc0:	ba a7 00 00 00       	mov    $0xa7,%edx
    5fc5:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    5fcc:	00 
    5fcd:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    5fd4:	45 0f b7 04 24       	movzwl (%r12),%r8d
    5fd9:	48 89 04 24          	mov    %rax,(%rsp)
    5fdd:	31 c0                	xor    %eax,%eax
    5fdf:	e8 00 00 00 00       	callq  5fe4 <__scif_register.part.11+0x7f4>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    5fe4:	4c 89 f7             	mov    %r14,%rdi
    5fe7:	e8 74 a4 ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
		if (window->num_pages[j])
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    5fec:	48 bf ff ff ff ff ff 	movabs $0xfffffffffffff,%rdi
    5ff3:	ff 0f 00 
	scif_pinned_pages_t pinned_pages;
	off_t err;
	struct endpt *ep = (struct endpt *)epd;
	uint64_t computed_offset;
	struct reg_range_t *window;
	struct mm_struct *mm = NULL;
    5ff6:	31 c9                	xor    %ecx,%ecx
	int j;
#ifdef CONFIG_ML1OM
	int l = 0, k;
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
    5ff8:	48 63 c1             	movslq %ecx,%rax
    5ffb:	49 3b 47 08          	cmp    0x8(%r15),%rax
    5fff:	7d 41                	jge    6042 <__scif_register.part.11+0x852>
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
    6001:	49 8b b7 e6 00 00 00 	mov    0xe6(%r15),%rsi
    6008:	48 8d 14 c5 00 00 00 	lea    0x0(,%rax,8),%rdx
    600f:	00 
    6010:	4d 8b 87 ee 00 00 00 	mov    0xee(%r15),%r8
    6017:	48 8b 34 c6          	mov    (%rsi,%rax,8),%rsi
    601b:	48 c1 ee 34          	shr    $0x34,%rsi
    601f:	41 89 34 80          	mov    %esi,(%r8,%rax,4)
		if (window->num_pages[j])
    6023:	49 8b b7 ee 00 00 00 	mov    0xee(%r15),%rsi
    602a:	83 3c 86 00          	cmpl   $0x0,(%rsi,%rax,4)
    602e:	74 12                	je     6042 <__scif_register.part.11+0x852>
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    6030:	48 89 d0             	mov    %rdx,%rax
    6033:	49 03 87 e6 00 00 00 	add    0xe6(%r15),%rax
	int j;
#ifdef CONFIG_ML1OM
	int l = 0, k;
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
    603a:	83 c1 01             	add    $0x1,%ecx
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
		if (window->num_pages[j])
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    603d:	48 21 38             	and    %rdi,(%rax)
    6040:	eb b6                	jmp    5ff8 <__scif_register.part.11+0x808>

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		micscif_set_nr_pages(ep->remote_dev, window);
		printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
    6042:	48 89 d9             	mov    %rbx,%rcx
    6045:	ba c8 09 00 00       	mov    $0x9c8,%edx
    604a:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6051:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6058:	31 c0                	xor    %eax,%eax
    605a:	e8 00 00 00 00       	callq  605f <__scif_register.part.11+0x86f>
    605f:	e9 1d ff ff ff       	jmpq   5f81 <__scif_register.part.11+0x791>
    6064:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
    606b:	00 00 00 00 00 

0000000000006070 <__scif_register_pinned_pages>:
 *	else an apt error is returned as documented in scif.h
 */
off_t
__scif_register_pinned_pages(scif_epd_t epd,
	scif_pinned_pages_t pinned_pages, off_t offset, int map_flags)
{
    6070:	55                   	push   %rbp
    6071:	48 89 e5             	mov    %rsp,%rbp
    6074:	41 57                	push   %r15
    6076:	41 56                	push   %r14
    6078:	41 55                	push   %r13
    607a:	41 54                	push   %r12
    607c:	53                   	push   %rbx
    607d:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    6084:	e8 00 00 00 00       	callq  6089 <__scif_register_pinned_pages+0x19>
	/* Bad EP */
	if (!ep || !pinned_pages || pinned_pages->magic != SCIFEP_MAGIC)
		return -EINVAL;
#endif
	/* Unsupported flags */
	if (map_flags & ~SCIF_MAP_FIXED)
    6089:	f7 c1 ef ff ff ff    	test   $0xffffffef,%ecx
 *	else an apt error is returned as documented in scif.h
 */
off_t
__scif_register_pinned_pages(scif_epd_t epd,
	scif_pinned_pages_t pinned_pages, off_t offset, int map_flags)
{
    608f:	48 89 d0             	mov    %rdx,%rax
	/* Bad EP */
	if (!ep || !pinned_pages || pinned_pages->magic != SCIFEP_MAGIC)
		return -EINVAL;
#endif
	/* Unsupported flags */
	if (map_flags & ~SCIF_MAP_FIXED)
    6092:	0f 85 f0 01 00 00    	jne    6288 <__scif_register_pinned_pages+0x218>
		return -EINVAL;

	len = pinned_pages->nr_pages << PAGE_SHIFT;
    6098:	4c 8b 36             	mov    (%rsi),%r14
    609b:	49 c1 e6 0c          	shl    $0xc,%r14

	/*
	 * Offset is not page aligned/negative or offset+len
	 * wraps around with SCIF_MAP_FIXED.
	 */
	if ((map_flags & SCIF_MAP_FIXED) &&
    609f:	f6 c1 10             	test   $0x10,%cl
    60a2:	74 2c                	je     60d0 <__scif_register_pinned_pages+0x60>
		((align_low(offset, PAGE_SIZE) != offset) ||
    60a4:	48 c1 ea 3f          	shr    $0x3f,%rdx
    60a8:	84 d2                	test   %dl,%dl
    60aa:	0f 85 d8 01 00 00    	jne    6288 <__scif_register_pinned_pages+0x218>
	wait_queue_head_t       wq;
};

static inline uint64_t align_low(uint64_t data, uint32_t granularity)
{
	return ALIGN(data - (granularity - 1), granularity);
    60b0:	48 89 c2             	mov    %rax,%rdx
    60b3:	48 81 e2 00 f0 ff ff 	and    $0xfffffffffffff000,%rdx
    60ba:	48 39 d0             	cmp    %rdx,%rax
    60bd:	0f 85 c5 01 00 00    	jne    6288 <__scif_register_pinned_pages+0x218>
		(offset < 0) ||
		(offset + (off_t)len < offset)))
    60c3:	49 8d 14 06          	lea    (%r14,%rax,1),%rdx
	 * Offset is not page aligned/negative or offset+len
	 * wraps around with SCIF_MAP_FIXED.
	 */
	if ((map_flags & SCIF_MAP_FIXED) &&
		((align_low(offset, PAGE_SIZE) != offset) ||
		(offset < 0) ||
    60c7:	48 39 d0             	cmp    %rdx,%rax
    60ca:	0f 8f b8 01 00 00    	jg     6288 <__scif_register_pinned_pages+0x218>
    60d0:	48 89 fb             	mov    %rdi,%rbx
    60d3:	41 89 cd             	mov    %ecx,%r13d
    60d6:	49 89 c7             	mov    %rax,%r15
    60d9:	49 89 f4             	mov    %rsi,%r12
		(offset + (off_t)len < offset)))
		return -EINVAL;

	might_sleep();
    60dc:	e8 00 00 00 00       	callq  60e1 <__scif_register_pinned_pages+0x71>
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    60e1:	8b 03                	mov    (%rbx),%eax
    60e3:	83 f8 09             	cmp    $0x9,%eax
    60e6:	74 28                	je     6110 <__scif_register_pinned_pages+0xa0>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    60e8:	8b 13                	mov    (%rbx),%edx
    60ea:	48 c7 c0 95 ff ff ff 	mov    $0xffffffffffffff95,%rax
    60f1:	83 fa 04             	cmp    $0x4,%edx
    60f4:	74 2a                	je     6120 <__scif_register_pinned_pages+0xb0>
	return computed_offset;
error_unmap:
	micscif_destroy_window(ep, window);
	printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	return err;
}
    60f6:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    60fd:	5b                   	pop    %rbx
    60fe:	41 5c                	pop    %r12
    6100:	41 5d                	pop    %r13
    6102:	41 5e                	pop    %r14
    6104:	41 5f                	pop    %r15
    6106:	5d                   	pop    %rbp
    6107:	c3                   	retq   
    6108:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    610f:	00 
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    6110:	48 c7 c0 98 ff ff ff 	mov    $0xffffffffffffff98,%rax
    6117:	eb dd                	jmp    60f6 <__scif_register_pinned_pages+0x86>
    6119:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    6120:	48 8b 83 48 01 00 00 	mov    0x148(%rbx),%rax
    6127:	8b 50 04             	mov    0x4(%rax),%edx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    612a:	48 c7 c0 ed ff ff ff 	mov    $0xffffffffffffffed,%rax
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    6131:	83 ea 02             	sub    $0x2,%edx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    6134:	83 fa 01             	cmp    $0x1,%edx
    6137:	77 bd                	ja     60f6 <__scif_register_pinned_pages+0x86>
    6139:	83 bb 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rbx)
    6140:	75 b4                	jne    60f6 <__scif_register_pinned_pages+0x86>

	if ((err = verify_epd(ep)))
		return err;

	/* Compute the offset for this registration */
	if ((err = micscif_get_window_offset(ep, map_flags, offset,
    6142:	4c 8d 85 70 ff ff ff 	lea    -0x90(%rbp),%r8
    6149:	4c 89 f1             	mov    %r14,%rcx
    614c:	4c 89 fa             	mov    %r15,%rdx
    614f:	44 89 ee             	mov    %r13d,%esi
    6152:	48 89 df             	mov    %rbx,%rdi
    6155:	e8 00 00 00 00       	callq  615a <__scif_register_pinned_pages+0xea>
    615a:	48 98                	cltq   
    615c:	85 c0                	test   %eax,%eax
    615e:	75 96                	jne    60f6 <__scif_register_pinned_pages+0x86>
			len, &computed_offset)))
		return err;

	/* Allocate and prepare self registration window */
	if (!(window = micscif_create_window(ep, pinned_pages->nr_pages,
    6160:	48 8b 95 70 ff ff ff 	mov    -0x90(%rbp),%rdx
    6167:	31 c9                	xor    %ecx,%ecx
    6169:	48 89 df             	mov    %rbx,%rdi
    616c:	49 8b 34 24          	mov    (%r12),%rsi
    6170:	e8 00 00 00 00       	callq  6175 <__scif_register_pinned_pages+0x105>
    6175:	48 85 c0             	test   %rax,%rax
    6178:	49 89 c7             	mov    %rax,%r15
    617b:	0f 84 7b 02 00 00    	je     63fc <__scif_register_pinned_pages+0x38c>
			computed_offset, false))) {
		micscif_free_window_offset(ep, computed_offset, len);
		return -ENOMEM;
	}

	window->pinned_pages = pinned_pages;
    6181:	4c 89 60 66          	mov    %r12,0x66(%rax)
	window->nr_pages = pinned_pages->nr_pages;
    6185:	49 8b 04 24          	mov    (%r12),%rax
    6189:	49 89 07             	mov    %rax,(%r15)
	window->nr_contig_chunks = pinned_pages->nr_contig_chunks;
    618c:	49 8b 44 24 30       	mov    0x30(%r12),%rax
    6191:	49 89 47 08          	mov    %rax,0x8(%r15)
	window->prot = pinned_pages->prot;
    6195:	41 8b 44 24 08       	mov    0x8(%r12),%eax
    619a:	41 89 47 10          	mov    %eax,0x10(%r15)
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read(const atomic_t *v)
{
	return (*(volatile int *)&(v)->counter);
    619e:	41 8b 4c 24 10       	mov    0x10(%r12),%ecx
	 * This set of pinned pages now belongs to this window as well.
	 * Assert if the ref count is zero since it is an error to
	 * pass pinned_pages to scif_register_pinned_pages() after
	 * calling scif_unpin_pages().
	 */
	if (!atomic_add_unless(&pinned_pages->ref_count, 
    61a3:	49 8b 04 24          	mov    (%r12),%rax
static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    61a7:	85 c9                	test   %ecx,%ecx
    61a9:	89 c7                	mov    %eax,%edi
    61ab:	0f 84 07 01 00 00    	je     62b8 <__scif_register_pinned_pages+0x248>
			break;
		old = atomic_cmpxchg((v), c, c + (a));
    61b1:	8d 14 01             	lea    (%rcx,%rax,1),%edx
#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	return cmpxchg(&v->counter, old, new);
    61b4:	89 c8                	mov    %ecx,%eax
    61b6:	49 8d 74 24 10       	lea    0x10(%r12),%rsi
    61bb:	f0 41 0f b1 54 24 10 	lock cmpxchg %edx,0x10(%r12)
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    61c2:	39 c1                	cmp    %eax,%ecx
#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	return cmpxchg(&v->counter, old, new);
    61c4:	89 c2                	mov    %eax,%edx
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    61c6:	0f 85 cc 00 00 00    	jne    6298 <__scif_register_pinned_pages+0x228>
				(int32_t)pinned_pages->nr_pages, 0))
		BUG_ON(1);

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    61cc:	4c 8b ab 48 01 00 00 	mov    0x148(%rbx),%r13
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    61d3:	4d 85 ed             	test   %r13,%r13
    61d6:	0f 85 e4 00 00 00    	jne    62c0 <__scif_register_pinned_pages+0x250>

	if ((err = micscif_send_alloc_request(ep, window))) {
    61dc:	4c 89 fe             	mov    %r15,%rsi
    61df:	48 89 df             	mov    %rbx,%rdi
    61e2:	e8 00 00 00 00       	callq  61e7 <__scif_register_pinned_pages+0x177>
    61e7:	85 c0                	test   %eax,%eax
    61e9:	41 89 c5             	mov    %eax,%r13d
    61ec:	0f 85 97 02 00 00    	jne    6489 <__scif_register_pinned_pages+0x419>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error_unmap;
	}

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
    61f2:	4c 89 fe             	mov    %r15,%rsi
    61f5:	48 89 df             	mov    %rbx,%rdi
    61f8:	e8 00 00 00 00       	callq  61fd <__scif_register_pinned_pages+0x18d>
    61fd:	85 c0                	test   %eax,%eax
    61ff:	41 89 c5             	mov    %eax,%r13d
    6202:	0f 85 2d 04 00 00    	jne    6635 <__scif_register_pinned_pages+0x5c5>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error_unmap;
	}

	/* Tell the peer about the new window */
	if ((err = micscif_send_scif_register(ep, window))) {
    6208:	4c 89 fe             	mov    %r15,%rsi
    620b:	48 89 df             	mov    %rbx,%rdi
    620e:	e8 00 00 00 00       	callq  6213 <__scif_register_pinned_pages+0x1a3>
    6213:	85 c0                	test   %eax,%eax
    6215:	41 89 c5             	mov    %eax,%r13d
    6218:	0f 85 9a 03 00 00    	jne    65b8 <__scif_register_pinned_pages+0x548>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error_unmap;
	}

	micscif_dec_node_refcnt(ep->remote_dev, 1);
    621e:	4c 8b ab 48 01 00 00 	mov    0x148(%rbx),%r13
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    6225:	4d 85 ed             	test   %r13,%r13
    6228:	74 21                	je     624b <__scif_register_pinned_pages+0x1db>
		if (unlikely((atomic_long_sub_return(cnt, 
    622a:	4d 8d b5 88 01 00 00 	lea    0x188(%r13),%r14
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    6231:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    6238:	f0 49 0f c1 85 88 01 	lock xadd %rax,0x188(%r13)
    623f:	00 00 
    6241:	48 83 e8 01          	sub    $0x1,%rax
    6245:	0f 88 d5 00 00 00    	js     6320 <__scif_register_pinned_pages+0x2b0>

	/* No further failures expected. Insert new window */
	mutex_lock(&ep->rma_info.rma_lock);
    624b:	4c 8d ab 80 00 00 00 	lea    0x80(%rbx),%r13
    6252:	4c 89 ef             	mov    %r13,%rdi
    6255:	e8 00 00 00 00       	callq  625a <__scif_register_pinned_pages+0x1ea>
}

static __always_inline void
set_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
    window->ref_count = (int)nr_pages; 
    625a:	49 8b 04 24          	mov    (%r12),%rax
	set_window_ref_count(window, pinned_pages->nr_pages);
	micscif_insert_window(window, &ep->rma_info.reg_list);
    625e:	48 8d 73 28          	lea    0x28(%rbx),%rsi
    6262:	4c 89 ff             	mov    %r15,%rdi
    6265:	41 89 47 14          	mov    %eax,0x14(%r15)
    6269:	e8 00 00 00 00       	callq  626e <__scif_register_pinned_pages+0x1fe>
	mutex_unlock(&ep->rma_info.rma_lock);
    626e:	4c 89 ef             	mov    %r13,%rdi
    6271:	e8 00 00 00 00       	callq  6276 <__scif_register_pinned_pages+0x206>

	return computed_offset;
    6276:	48 8b 85 70 ff ff ff 	mov    -0x90(%rbp),%rax
    627d:	e9 74 fe ff ff       	jmpq   60f6 <__scif_register_pinned_pages+0x86>
    6282:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	if (!ep || !pinned_pages || pinned_pages->magic != SCIFEP_MAGIC)
		return -EINVAL;
#endif
	/* Unsupported flags */
	if (map_flags & ~SCIF_MAP_FIXED)
		return -EINVAL;
    6288:	48 c7 c0 ea ff ff ff 	mov    $0xffffffffffffffea,%rax
    628f:	e9 62 fe ff ff       	jmpq   60f6 <__scif_register_pinned_pages+0x86>
    6294:	0f 1f 40 00          	nopl   0x0(%rax)
static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    6298:	85 d2                	test   %edx,%edx
    629a:	74 1c                	je     62b8 <__scif_register_pinned_pages+0x248>
			break;
		old = atomic_cmpxchg((v), c, c + (a));
    629c:	8d 0c 17             	lea    (%rdi,%rdx,1),%ecx
#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	return cmpxchg(&v->counter, old, new);
    629f:	89 d0                	mov    %edx,%eax
    62a1:	f0 0f b1 0e          	lock cmpxchg %ecx,(%rsi)
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    62a5:	39 c2                	cmp    %eax,%edx
    62a7:	0f 84 1f ff ff ff    	je     61cc <__scif_register_pinned_pages+0x15c>
    62ad:	89 c2                	mov    %eax,%edx
static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    62af:	85 d2                	test   %edx,%edx
    62b1:	75 e9                	jne    629c <__scif_register_pinned_pages+0x22c>
    62b3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	 * pass pinned_pages to scif_register_pinned_pages() after
	 * calling scif_unpin_pages().
	 */
	if (!atomic_add_unless(&pinned_pages->ref_count, 
				(int32_t)pinned_pages->nr_pages, 0))
		BUG_ON(1);
    62b8:	0f 0b                	ud2    
    62ba:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    62c0:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    62c7:	4d 8d b5 88 01 00 00 	lea    0x188(%r13),%r14
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    62ce:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    62d5:	00 00 80 
    62d8:	48 39 f1             	cmp    %rsi,%rcx
    62db:	0f 84 cf 00 00 00    	je     63b0 <__scif_register_pinned_pages+0x340>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    62e1:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    62e5:	48 89 c8             	mov    %rcx,%rax
    62e8:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    62ef:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    62f1:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    62f4:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    62f7:	0f 84 df fe ff ff    	je     61dc <__scif_register_pinned_pages+0x16c>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    62fd:	48 39 f2             	cmp    %rsi,%rdx
    6300:	0f 84 aa 00 00 00    	je     63b0 <__scif_register_pinned_pages+0x340>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    6306:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    630a:	48 89 d0             	mov    %rdx,%rax
    630d:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    6312:	48 39 c2             	cmp    %rax,%rdx
    6315:	0f 84 c1 fe ff ff    	je     61dc <__scif_register_pinned_pages+0x16c>
    631b:	48 89 c2             	mov    %rax,%rdx
    631e:	eb dd                	jmp    62fd <__scif_register_pinned_pages+0x28d>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    6320:	48 8b 45 08          	mov    0x8(%rbp),%rax
    6324:	4c 89 e9             	mov    %r13,%rcx
    6327:	ba a7 00 00 00       	mov    $0xa7,%edx
    632c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    6333:	4d 8b 8d 88 01 00 00 	mov    0x188(%r13),%r9
    633a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6341:	45 0f b7 45 00       	movzwl 0x0(%r13),%r8d
    6346:	48 89 04 24          	mov    %rax,(%rsp)
    634a:	31 c0                	xor    %eax,%eax
    634c:	e8 00 00 00 00       	callq  6351 <__scif_register_pinned_pages+0x2e1>
    6351:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    6358:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    635f:	00 00 80 
    6362:	48 39 f1             	cmp    %rsi,%rcx
    6365:	0f 84 e0 fe ff ff    	je     624b <__scif_register_pinned_pages+0x1db>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    636b:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    636f:	48 89 c8             	mov    %rcx,%rax
    6372:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    6379:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    637b:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    637e:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    6381:	0f 84 c4 fe ff ff    	je     624b <__scif_register_pinned_pages+0x1db>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    6387:	48 39 f2             	cmp    %rsi,%rdx
    638a:	0f 84 bb fe ff ff    	je     624b <__scif_register_pinned_pages+0x1db>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    6390:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    6394:	48 89 d0             	mov    %rdx,%rax
    6397:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    639c:	48 39 c2             	cmp    %rax,%rdx
    639f:	0f 84 a6 fe ff ff    	je     624b <__scif_register_pinned_pages+0x1db>
    63a5:	48 89 c2             	mov    %rax,%rdx
    63a8:	eb dd                	jmp    6387 <__scif_register_pinned_pages+0x317>
    63aa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    63b0:	49 8d 85 68 01 00 00 	lea    0x168(%r13),%rax
    63b7:	48 89 c7             	mov    %rax,%rdi
    63ba:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    63c1:	e8 00 00 00 00       	callq  63c6 <__scif_register_pinned_pages+0x356>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    63c6:	41 8b 45 04          	mov    0x4(%r13),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    63ca:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    63cd:	83 fa 01             	cmp    $0x1,%edx
    63d0:	76 19                	jbe    63eb <__scif_register_pinned_pages+0x37b>
    63d2:	83 f8 01             	cmp    $0x1,%eax
    63d5:	74 14                	je     63eb <__scif_register_pinned_pages+0x37b>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    63d7:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    63de:	48 85 c0             	test   %rax,%rax
    63e1:	78 37                	js     641a <__scif_register_pinned_pages+0x3aa>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    63e3:	4c 89 f7             	mov    %r14,%rdi
    63e6:	e8 05 a1 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    63eb:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    63f2:	e8 00 00 00 00       	callq  63f7 <__scif_register_pinned_pages+0x387>
    63f7:	e9 e0 fd ff ff       	jmpq   61dc <__scif_register_pinned_pages+0x16c>
		return err;

	/* Allocate and prepare self registration window */
	if (!(window = micscif_create_window(ep, pinned_pages->nr_pages,
			computed_offset, false))) {
		micscif_free_window_offset(ep, computed_offset, len);
    63fc:	48 8b b5 70 ff ff ff 	mov    -0x90(%rbp),%rsi
    6403:	4c 89 f2             	mov    %r14,%rdx
    6406:	48 89 df             	mov    %rbx,%rdi
    6409:	e8 00 00 00 00       	callq  640e <__scif_register_pinned_pages+0x39e>
		return -ENOMEM;
    640e:	48 c7 c0 f4 ff ff ff 	mov    $0xfffffffffffffff4,%rax
    6415:	e9 dc fc ff ff       	jmpq   60f6 <__scif_register_pinned_pages+0x86>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    641a:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 6420 <__scif_register_pinned_pages+0x3b0>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    6420:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    6427:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    642b:	31 d2                	xor    %edx,%edx
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    642d:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    6433:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    643a:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    6441:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    6445:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    6449:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    644e:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    6452:	e8 00 00 00 00       	callq  6457 <__scif_register_pinned_pages+0x3e7>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    6457:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    645e:	02 
    645f:	0f 84 cc 00 00 00    	je     6531 <__scif_register_pinned_pages+0x4c1>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    6465:	49 83 bd b0 01 00 00 	cmpq   $0x3,0x1b0(%r13)
    646c:	03 
    646d:	0f 85 70 ff ff ff    	jne    63e3 <__scif_register_pinned_pages+0x373>
				dev->sd_state = SCIFDEV_RUNNING;
    6473:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    647a:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    647b:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    6482:	00 7f 
    6484:	e9 5a ff ff ff       	jmpq   63e3 <__scif_register_pinned_pages+0x373>
		BUG_ON(1);

	micscif_inc_node_refcnt(ep->remote_dev, 1);

	if ((err = micscif_send_alloc_request(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    6489:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    6490:	4d 85 e4             	test   %r12,%r12
    6493:	74 4f                	je     64e4 <__scif_register_pinned_pages+0x474>
		if (unlikely((atomic_long_sub_return(cnt, 
    6495:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    649c:	00 
    649d:	4c 89 f7             	mov    %r14,%rdi
    64a0:	e8 2b a0 ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    64a5:	48 85 c0             	test   %rax,%rax
    64a8:	79 3a                	jns    64e4 <__scif_register_pinned_pages+0x474>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    64aa:	48 8b 45 08          	mov    0x8(%rbp),%rax
    64ae:	4c 89 e1             	mov    %r12,%rcx
    64b1:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    64b8:	ba a7 00 00 00       	mov    $0xa7,%edx
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    64bd:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    64c4:	00 
    64c5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    64cc:	45 0f b7 04 24       	movzwl (%r12),%r8d
    64d1:	48 89 04 24          	mov    %rax,(%rsp)
    64d5:	31 c0                	xor    %eax,%eax
    64d7:	e8 00 00 00 00       	callq  64dc <__scif_register_pinned_pages+0x46c>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    64dc:	4c 89 f7             	mov    %r14,%rdi
    64df:	e8 7c 9f ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    64e4:	44 89 e9             	mov    %r13d,%ecx
    64e7:	ba 19 08 00 00       	mov    $0x819,%edx
    64ec:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    64f3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    64fa:	31 c0                	xor    %eax,%eax
    64fc:	e8 00 00 00 00       	callq  6501 <__scif_register_pinned_pages+0x491>
	micscif_insert_window(window, &ep->rma_info.reg_list);
	mutex_unlock(&ep->rma_info.rma_lock);

	return computed_offset;
error_unmap:
	micscif_destroy_window(ep, window);
    6501:	4c 89 fe             	mov    %r15,%rsi
    6504:	48 89 df             	mov    %rbx,%rdi
    6507:	e8 00 00 00 00       	callq  650c <__scif_register_pinned_pages+0x49c>
	printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    650c:	44 89 e9             	mov    %r13d,%ecx
    650f:	ba 37 08 00 00       	mov    $0x837,%edx
    6514:	31 c0                	xor    %eax,%eax
    6516:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    651d:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6524:	e8 00 00 00 00       	callq  6529 <__scif_register_pinned_pages+0x4b9>
	return err;
    6529:	49 63 c5             	movslq %r13d,%rax
    652c:	e9 c5 fb ff ff       	jmpq   60f6 <__scif_register_pinned_pages+0x86>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    6531:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    6538:	31 c0                	xor    %eax,%eax
    653a:	b9 0a 00 00 00       	mov    $0xa,%ecx
    653f:	f3 ab                	rep stos %eax,%es:(%rdi)
    6541:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    6548:	00 
    6549:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    6550:	00 00 
    6552:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    6556:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    655d:	48 83 c0 18          	add    $0x18,%rax
    6561:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    6565:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    6569:	49 8d 85 98 01 00 00 	lea    0x198(%r13),%rax
    6570:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    6577:	eb 05                	jmp    657e <__scif_register_pinned_pages+0x50e>
    6579:	e8 00 00 00 00       	callq  657e <__scif_register_pinned_pages+0x50e>
    657e:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    6585:	ba 02 00 00 00       	mov    $0x2,%edx
    658a:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    6591:	e8 00 00 00 00       	callq  6596 <__scif_register_pinned_pages+0x526>
    6596:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    659d:	02 
    659e:	74 d9                	je     6579 <__scif_register_pinned_pages+0x509>
    65a0:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    65a7:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    65ae:	e8 00 00 00 00       	callq  65b3 <__scif_register_pinned_pages+0x543>
    65b3:	e9 ad fe ff ff       	jmpq   6465 <__scif_register_pinned_pages+0x3f5>
		goto error_unmap;
	}

	/* Tell the peer about the new window */
	if ((err = micscif_send_scif_register(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    65b8:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    65bf:	4d 85 e4             	test   %r12,%r12
    65c2:	74 4f                	je     6613 <__scif_register_pinned_pages+0x5a3>
		if (unlikely((atomic_long_sub_return(cnt, 
    65c4:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    65cb:	00 
    65cc:	4c 89 f7             	mov    %r14,%rdi
    65cf:	e8 fc 9e ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    65d4:	48 85 c0             	test   %rax,%rax
    65d7:	79 3a                	jns    6613 <__scif_register_pinned_pages+0x5a3>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    65d9:	48 8b 45 08          	mov    0x8(%rbp),%rax
    65dd:	4c 89 e1             	mov    %r12,%rcx
    65e0:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    65e7:	ba a7 00 00 00       	mov    $0xa7,%edx
    65ec:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    65f3:	00 
    65f4:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    65fb:	45 0f b7 04 24       	movzwl (%r12),%r8d
    6600:	48 89 04 24          	mov    %rax,(%rsp)
    6604:	31 c0                	xor    %eax,%eax
    6606:	e8 00 00 00 00       	callq  660b <__scif_register_pinned_pages+0x59b>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    660b:	4c 89 f7             	mov    %r14,%rdi
    660e:	e8 4d 9e ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    6613:	44 89 e9             	mov    %r13d,%ecx
    6616:	ba 28 08 00 00       	mov    $0x828,%edx
    661b:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6622:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6629:	31 c0                	xor    %eax,%eax
    662b:	e8 00 00 00 00       	callq  6630 <__scif_register_pinned_pages+0x5c0>
		goto error_unmap;
    6630:	e9 cc fe ff ff       	jmpq   6501 <__scif_register_pinned_pages+0x491>
		goto error_unmap;
	}

	/* Prepare the remote registration window */
	if ((err = micscif_prep_remote_window(ep, window))) {
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    6635:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    663c:	4d 85 e4             	test   %r12,%r12
    663f:	74 4f                	je     6690 <__scif_register_pinned_pages+0x620>
		if (unlikely((atomic_long_sub_return(cnt, 
    6641:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    6648:	00 
    6649:	4c 89 f7             	mov    %r14,%rdi
    664c:	e8 7f 9e ff ff       	callq  4d0 <atomic_long_sub_return.constprop.16>
    6651:	48 85 c0             	test   %rax,%rax
    6654:	79 3a                	jns    6690 <__scif_register_pinned_pages+0x620>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    6656:	48 8b 45 08          	mov    0x8(%rbp),%rax
    665a:	4c 89 e1             	mov    %r12,%rcx
    665d:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6664:	ba a7 00 00 00       	mov    $0xa7,%edx
    6669:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    6670:	00 
    6671:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6678:	45 0f b7 04 24       	movzwl (%r12),%r8d
    667d:	48 89 04 24          	mov    %rax,(%rsp)
    6681:	31 c0                	xor    %eax,%eax
    6683:	e8 00 00 00 00       	callq  6688 <__scif_register_pinned_pages+0x618>
				" caller %p Lost Node?? \n", 
				__func__, __LINE__, dev, dev->sd_node, 
				atomic_long_read(&dev->scif_ref_cnt), 
				__builtin_return_address(0));
			atomic_long_add_unless(&dev->scif_ref_cnt, cnt, 
    6688:	4c 89 f7             	mov    %r14,%rdi
    668b:	e8 d0 9d ff ff       	callq  460 <atomic_long_add_unless.constprop.15>
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
		if (window->num_pages[j])
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    6690:	48 bf ff ff ff ff ff 	movabs $0xfffffffffffff,%rdi
    6697:	ff 0f 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    669a:	31 c9                	xor    %ecx,%ecx
    669c:	eb 3f                	jmp    66dd <__scif_register_pinned_pages+0x66d>
#ifdef CONFIG_ML1OM
	int l = 0, k;
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
    669e:	49 8b b7 e6 00 00 00 	mov    0xe6(%r15),%rsi
    66a5:	48 8d 14 c5 00 00 00 	lea    0x0(,%rax,8),%rdx
    66ac:	00 
    66ad:	4d 8b 87 ee 00 00 00 	mov    0xee(%r15),%r8
    66b4:	48 8b 34 c6          	mov    (%rsi,%rax,8),%rsi
    66b8:	48 c1 ee 34          	shr    $0x34,%rsi
    66bc:	41 89 34 80          	mov    %esi,(%r8,%rax,4)
		if (window->num_pages[j])
    66c0:	49 8b b7 ee 00 00 00 	mov    0xee(%r15),%rsi
    66c7:	83 3c 86 00          	cmpl   $0x0,(%rsi,%rax,4)
    66cb:	74 19                	je     66e6 <__scif_register_pinned_pages+0x676>
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    66cd:	48 89 d0             	mov    %rdx,%rax
    66d0:	49 03 87 e6 00 00 00 	add    0xe6(%r15),%rax
	int j;
#ifdef CONFIG_ML1OM
	int l = 0, k;
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
    66d7:	83 c1 01             	add    $0x1,%ecx
		window->num_pages[j] = RMA_GET_NR_PAGES(window->dma_addr[j]);
		if (window->num_pages[j])
			window->dma_addr[j] = RMA_GET_ADDR(window->dma_addr[j]);
    66da:	48 21 38             	and    %rdi,(%rax)
	int j;
#ifdef CONFIG_ML1OM
	int l = 0, k;
#endif

	for (j = 0; j < window->nr_contig_chunks; j++) {
    66dd:	48 63 c1             	movslq %ecx,%rax
    66e0:	49 3b 47 08          	cmp    0x8(%r15),%rax
    66e4:	7c b8                	jl     669e <__scif_register_pinned_pages+0x62e>
		micscif_set_nr_pages(ep->remote_dev, window);
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    66e6:	44 89 e9             	mov    %r13d,%ecx
    66e9:	ba 21 08 00 00       	mov    $0x821,%edx
    66ee:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    66f5:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    66fc:	31 c0                	xor    %eax,%eax
    66fe:	e8 00 00 00 00       	callq  6703 <__scif_register_pinned_pages+0x693>
		goto error_unmap;
    6703:	e9 f9 fd ff ff       	jmpq   6501 <__scif_register_pinned_pages+0x491>
    6708:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    670f:	00 

0000000000006710 <scif_register_pinned_pages>:
}

off_t
scif_register_pinned_pages(scif_epd_t epd,
	scif_pinned_pages_t pinned_pages, off_t offset, int map_flags)
{
    6710:	55                   	push   %rbp
    6711:	48 89 e5             	mov    %rsp,%rbp
    6714:	41 57                	push   %r15
    6716:	41 56                	push   %r14
    6718:	41 55                	push   %r13
    671a:	41 54                	push   %r12
    671c:	53                   	push   %rbx
    671d:	48 83 ec 08          	sub    $0x8,%rsp
    6721:	e8 00 00 00 00       	callq  6726 <scif_register_pinned_pages+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    6726:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    672d:	49 89 fc             	mov    %rdi,%r12
    6730:	49 89 f5             	mov    %rsi,%r13
    6733:	49 89 d6             	mov    %rdx,%r14
    6736:	41 89 cf             	mov    %ecx,%r15d
    6739:	48 89 df             	mov    %rbx,%rdi
    673c:	e8 00 00 00 00       	callq  6741 <scif_register_pinned_pages+0x31>
	off_t ret;
	get_kref_count(epd);
	ret = __scif_register_pinned_pages(epd, pinned_pages, offset, map_flags);
    6741:	44 89 f9             	mov    %r15d,%ecx
    6744:	4c 89 f2             	mov    %r14,%rdx
    6747:	4c 89 ee             	mov    %r13,%rsi
    674a:	4c 89 e7             	mov    %r12,%rdi
    674d:	e8 00 00 00 00       	callq  6752 <scif_register_pinned_pages+0x42>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    6752:	48 89 df             	mov    %rbx,%rdi
    6755:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    675c:	49 89 c4             	mov    %rax,%r12
    675f:	e8 00 00 00 00       	callq  6764 <scif_register_pinned_pages+0x54>
	put_kref_count(epd);
	return ret;
}
    6764:	48 83 c4 08          	add    $0x8,%rsp
    6768:	4c 89 e0             	mov    %r12,%rax
    676b:	5b                   	pop    %rbx
    676c:	41 5c                	pop    %r12
    676e:	41 5d                	pop    %r13
    6770:	41 5e                	pop    %r14
    6772:	41 5f                	pop    %r15
    6774:	5d                   	pop    %rbp
    6775:	c3                   	retq   
    6776:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    677d:	00 00 00 

0000000000006780 <__scif_get_pages>:
 *	Upon successful completion, scif_get_pages() returns 0;
 *	else an apt error is returned as documented in scif.h.
 */
int
__scif_get_pages(scif_epd_t epd, off_t offset, size_t len, struct scif_range **pages)
{
    6780:	55                   	push   %rbp
    6781:	48 89 e5             	mov    %rsp,%rbp
    6784:	41 57                	push   %r15
    6786:	41 56                	push   %r14
    6788:	41 55                	push   %r13
    678a:	41 54                	push   %r12
    678c:	53                   	push   %rbx
    678d:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    6794:	e8 00 00 00 00       	callq  6799 <__scif_get_pages+0x19>
	struct endpt *ep = (struct endpt *)epd;
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
	int nr_pages, err, i;

	pr_debug("SCIFAPI get_pinned_pages: ep %p %s offset 0x%lx len 0x%lx\n", 
    6799:	8b 07                	mov    (%rdi),%eax
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    679b:	8b 07                	mov    (%rdi),%eax
int
__scif_get_pages(scif_epd_t epd, off_t offset, size_t len, struct scif_range **pages)
{
	struct endpt *ep = (struct endpt *)epd;
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
    679d:	48 c7 45 98 00 00 00 	movq   $0x0,-0x68(%rbp)
    67a4:	00 
    67a5:	83 f8 09             	cmp    $0x9,%eax
    67a8:	74 20                	je     67ca <__scif_get_pages+0x4a>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    67aa:	44 8b 07             	mov    (%rdi),%r8d
		return -ENOTCONN;
    67ad:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    67b2:	41 83 f8 04          	cmp    $0x4,%r8d
    67b6:	74 19                	je     67d1 <__scif_get_pages+0x51>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	} else {
		micscif_create_node_dep(ep->remote_dev, nr_pages);
	}
	return err;
}
    67b8:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    67bf:	5b                   	pop    %rbx
    67c0:	41 5c                	pop    %r12
    67c2:	41 5d                	pop    %r13
    67c4:	41 5e                	pop    %r14
    67c6:	41 5f                	pop    %r15
    67c8:	5d                   	pop    %rbp
    67c9:	c3                   	retq   
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;
    67ca:	b8 98 ff ff ff       	mov    $0xffffff98,%eax
    67cf:	eb e7                	jmp    67b8 <__scif_get_pages+0x38>
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    67d1:	48 8b 87 48 01 00 00 	mov    0x148(%rdi),%rax
    67d8:	44 8b 40 04          	mov    0x4(%rax),%r8d
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    67dc:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    67e1:	41 83 e8 02          	sub    $0x2,%r8d
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    67e5:	41 83 f8 01          	cmp    $0x1,%r8d
    67e9:	77 cd                	ja     67b8 <__scif_get_pages+0x38>
    67eb:	83 bf 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rdi)
    67f2:	75 c4                	jne    67b8 <__scif_get_pages+0x38>

	if ((err = verify_epd(ep)))
		return err;

	if ((!len) ||
		(offset < 0) ||
    67f4:	48 89 f0             	mov    %rsi,%rax
    67f7:	48 c1 e8 3f          	shr    $0x3f,%rax
		ep, scif_ep_states[ep->state], offset, len);

	if ((err = verify_epd(ep)))
		return err;

	if ((!len) ||
    67fb:	84 c0                	test   %al,%al
    67fd:	0f 85 ed 04 00 00    	jne    6cf0 <__scif_get_pages+0x570>
    6803:	48 85 d2             	test   %rdx,%rdx
    6806:	0f 84 e4 04 00 00    	je     6cf0 <__scif_get_pages+0x570>
		(offset < 0) ||
    680c:	48 89 f0             	mov    %rsi,%rax
		(offset + len < offset) ||
    680f:	48 89 f3             	mov    %rsi,%rbx

	if ((err = verify_epd(ep)))
		return err;

	if ((!len) ||
		(offset < 0) ||
    6812:	48 01 d0             	add    %rdx,%rax
    6815:	0f 82 d5 04 00 00    	jb     6cf0 <__scif_get_pages+0x570>
	wait_queue_head_t       wq;
};

static inline uint64_t align_low(uint64_t data, uint32_t granularity)
{
	return ALIGN(data - (granularity - 1), granularity);
    681b:	48 89 f0             	mov    %rsi,%rax
    681e:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
		(offset + len < offset) ||
    6824:	48 39 c6             	cmp    %rax,%rsi
    6827:	0f 85 c3 04 00 00    	jne    6cf0 <__scif_get_pages+0x570>
    682d:	48 89 d0             	mov    %rdx,%rax
    6830:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
		(align_low((uint64_t)offset, PAGE_SIZE) != (uint64_t)offset) ||
    6836:	48 39 c2             	cmp    %rax,%rdx
    6839:	0f 85 b1 04 00 00    	jne    6cf0 <__scif_get_pages+0x570>
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	nr_pages = len >> PAGE_SHIFT;
    683f:	48 89 d0             	mov    %rdx,%rax
	req.prot = 0;
	req.nr_bytes = len;
	req.type = WINDOW_SINGLE;
	req.head = &ep->rma_info.remote_reg_list;

	mutex_lock(&ep->rma_info.rma_lock);
    6842:	48 89 7d 88          	mov    %rdi,-0x78(%rbp)
    6846:	49 89 cc             	mov    %rcx,%r12
		(offset + len < offset) ||
		(align_low((uint64_t)offset, PAGE_SIZE) != (uint64_t)offset) ||
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	nr_pages = len >> PAGE_SHIFT;
    6849:	48 c1 e8 0c          	shr    $0xc,%rax

	req.out_window = &window;
	req.offset = offset;
    684d:	48 89 75 a8          	mov    %rsi,-0x58(%rbp)
		(offset + len < offset) ||
		(align_low((uint64_t)offset, PAGE_SIZE) != (uint64_t)offset) ||
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	nr_pages = len >> PAGE_SHIFT;
    6851:	48 89 85 70 ff ff ff 	mov    %rax,-0x90(%rbp)

	req.out_window = &window;
    6858:	48 8d 45 98          	lea    -0x68(%rbp),%rax
    685c:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
	req.offset = offset;
	req.prot = 0;
	req.nr_bytes = len;
	req.type = WINDOW_SINGLE;
	req.head = &ep->rma_info.remote_reg_list;
    6860:	48 8d 47 38          	lea    0x38(%rdi),%rax
    6864:	48 89 45 c0          	mov    %rax,-0x40(%rbp)

	mutex_lock(&ep->rma_info.rma_lock);
    6868:	48 8d 87 80 00 00 00 	lea    0x80(%rdi),%rax
    686f:	48 89 c7             	mov    %rax,%rdi
	nr_pages = len >> PAGE_SHIFT;

	req.out_window = &window;
	req.offset = offset;
	req.prot = 0;
	req.nr_bytes = len;
    6872:	48 89 55 b0          	mov    %rdx,-0x50(%rbp)
	req.type = WINDOW_SINGLE;
	req.head = &ep->rma_info.remote_reg_list;

	mutex_lock(&ep->rma_info.rma_lock);
    6876:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)

	nr_pages = len >> PAGE_SHIFT;

	req.out_window = &window;
	req.offset = offset;
	req.prot = 0;
    687d:	c7 45 b8 00 00 00 00 	movl   $0x0,-0x48(%rbp)
	req.nr_bytes = len;
	req.type = WINDOW_SINGLE;
    6884:	c7 45 bc 01 00 00 00 	movl   $0x1,-0x44(%rbp)
	req.head = &ep->rma_info.remote_reg_list;

	mutex_lock(&ep->rma_info.rma_lock);
    688b:	e8 00 00 00 00       	callq  6890 <__scif_get_pages+0x110>
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
    6890:	48 8d 7d a0          	lea    -0x60(%rbp),%rdi
    6894:	e8 00 00 00 00       	callq  6899 <__scif_get_pages+0x119>
    6899:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    689d:	85 c0                	test   %eax,%eax
    689f:	41 89 c5             	mov    %eax,%r13d
    68a2:	0f 85 66 05 00 00    	jne    6e0e <__scif_get_pages+0x68e>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	RMA_MAGIC(window);
    68a8:	48 b8 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rax
    68af:	00 1f 5c 
    68b2:	48 8b 55 98          	mov    -0x68(%rbp),%rdx
    68b6:	48 39 42 18          	cmp    %rax,0x18(%rdx)
    68ba:	0f 85 c2 04 00 00    	jne    6d82 <__scif_get_pages+0x602>
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
    68c0:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 68c7 <__scif_get_pages+0x147>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
    68c7:	48 85 ff             	test   %rdi,%rdi
    68ca:	0f 84 2a 04 00 00    	je     6cfa <__scif_get_pages+0x57a>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
    68d0:	ba 20 00 00 00       	mov    $0x20,%edx
    68d5:	be d0 80 00 00       	mov    $0x80d0,%esi
    68da:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
    68de:	e8 00 00 00 00       	callq  68e3 <__scif_get_pages+0x163>

	/* Allocate scif_range */
	if (!(*pages = kzalloc(sizeof(struct scif_range), GFP_KERNEL))) {
    68e3:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    68e7:	48 85 c0             	test   %rax,%rax
    68ea:	49 89 c6             	mov    %rax,%r14
    68ed:	49 89 04 24          	mov    %rax,(%r12)
    68f1:	0f 84 af 04 00 00    	je     6da6 <__scif_get_pages+0x626>
		err = -ENOMEM;
		goto error;
	}

	/* Allocate phys addr array */
	if (!((*pages)->phys_addr = scif_zalloc(nr_pages * sizeof(dma_addr_t)))) {
    68f7:	4c 63 ad 70 ff ff ff 	movslq -0x90(%rbp),%r13
    68fe:	49 c1 e5 03          	shl    $0x3,%r13
 * vmalloc(..) is *slow*.
 */
static __always_inline void *scif_zalloc(size_t size)
{
	void *ret;
	size_t align = ALIGN(size, PAGE_SIZE);
    6902:	49 8d 85 ff 0f 00 00 	lea    0xfff(%r13),%rax

	if (!align)
    6909:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
    690f:	48 89 45 88          	mov    %rax,-0x78(%rbp)
    6913:	0f 84 bc 02 00 00    	je     6bd5 <__scif_get_pages+0x455>
		return NULL;

	if (align <= (1 << (MAX_ORDER + PAGE_SHIFT - 1)))
    6919:	48 3d 00 00 40 00    	cmp    $0x400000,%rax
    691f:	0f 87 e8 03 00 00    	ja     6d0d <__scif_get_pages+0x58d>
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    6925:	4c 8d 78 ff          	lea    -0x1(%rax),%r15
	order = -1;
    6929:	be ff ff ff ff       	mov    $0xffffffff,%esi
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    692e:	4c 89 fa             	mov    %r15,%rdx
    6931:	48 c1 ea 0b          	shr    $0xb,%rdx
    6935:	48 89 d0             	mov    %rdx,%rax
    6938:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    693f:	00 
	order = -1;
	do {
		size >>= 1;
		order++;
    6940:	83 c6 01             	add    $0x1,%esi
	} while (size);
    6943:	48 d1 e8             	shr    %rax
    6946:	75 f8                	jne    6940 <__scif_get_pages+0x1c0>
		if ((ret = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
    6948:	bf d0 80 00 00       	mov    $0x80d0,%edi
    694d:	48 89 95 78 ff ff ff 	mov    %rdx,-0x88(%rbp)
    6954:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6958:	e8 00 00 00 00       	callq  695d <__scif_get_pages+0x1dd>
    695d:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
    6961:	48 85 c0             	test   %rax,%rax
    6964:	48 8b 95 78 ff ff ff 	mov    -0x88(%rbp),%rdx
    696b:	0f 84 4c 04 00 00    	je     6dbd <__scif_get_pages+0x63d>
    6971:	49 89 46 10          	mov    %rax,0x10(%r14)
		goto error;
	}

#ifndef _MIC_SCIF_
	/* Allocate virtual address array */
	if (!((*pages)->va = scif_zalloc(nr_pages * sizeof(void *)))) {
    6975:	4d 8b 34 24          	mov    (%r12),%r14
    6979:	49 89 d7             	mov    %rdx,%r15
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
    697c:	be ff ff ff ff       	mov    $0xffffffff,%esi
    6981:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	do {
		size >>= 1;
		order++;
    6988:	83 c6 01             	add    $0x1,%esi
	} while (size);
    698b:	49 d1 ef             	shr    %r15
    698e:	75 f8                	jne    6988 <__scif_get_pages+0x208>
    6990:	bf d0 80 00 00       	mov    $0x80d0,%edi
    6995:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6999:	e8 00 00 00 00       	callq  699e <__scif_get_pages+0x21e>
    699e:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
    69a2:	48 85 c0             	test   %rax,%rax
    69a5:	0f 84 9c 03 00 00    	je     6d47 <__scif_get_pages+0x5c7>
    69ab:	49 89 46 18          	mov    %rax,0x18(%r14)
	if ((err = micscif_rma_list_gtt_map(window, offset, nr_pages))) {
		goto error;
	}
#endif
	/* Populate the values */
	(*pages)->cookie = window;
    69af:	48 8b 55 98          	mov    -0x68(%rbp),%rdx
    69b3:	49 8b 04 24          	mov    (%r12),%rax
	(*pages)->nr_pages = nr_pages;
    69b7:	48 8b bd 70 ff ff ff 	mov    -0x90(%rbp),%rdi
	if ((err = micscif_rma_list_gtt_map(window, offset, nr_pages))) {
		goto error;
	}
#endif
	/* Populate the values */
	(*pages)->cookie = window;
    69be:	48 89 10             	mov    %rdx,(%rax)
	(*pages)->nr_pages = nr_pages;
    69c1:	49 8b 04 24          	mov    (%r12),%rax
	(*pages)->prot_flags = window->prot;
    69c5:	4c 8b 6d 98          	mov    -0x68(%rbp),%r13

	for (i = 0; i < nr_pages; i++) {
    69c9:	85 ff                	test   %edi,%edi
		goto error;
	}
#endif
	/* Populate the values */
	(*pages)->cookie = window;
	(*pages)->nr_pages = nr_pages;
    69cb:	89 78 08             	mov    %edi,0x8(%rax)
	(*pages)->prot_flags = window->prot;
    69ce:	49 8b 04 24          	mov    (%r12),%rax
    69d2:	41 8b 55 10          	mov    0x10(%r13),%edx
    69d6:	89 50 0c             	mov    %edx,0xc(%rax)

	for (i = 0; i < nr_pages; i++) {
    69d9:	0f 8e bb 01 00 00    	jle    6b9a <__scif_get_pages+0x41a>
    69df:	89 f8                	mov    %edi,%eax
    69e1:	4d 89 e7             	mov    %r12,%r15
    69e4:	45 31 f6             	xor    %r14d,%r14d
    69e7:	83 e8 01             	sub    $0x1,%eax
    69ea:	4d 89 cc             	mov    %r9,%r12
    69ed:	48 c1 e0 03          	shl    $0x3,%rax
    69f1:	48 89 85 78 ff ff ff 	mov    %rax,-0x88(%rbp)
    69f8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    69ff:	00 
		(*pages)->phys_addr[i] =
    6a00:	49 8b 07             	mov    (%r15),%rax
    6a03:	4d 89 f1             	mov    %r14,%r9
 * for the next iteration.
 */
static __always_inline dma_addr_t
micscif_get_dma_addr(struct reg_range_t *window, uint64_t off, size_t *nr_bytes, int *index, uint64_t *start_off)
{
	if (window->nr_pages == window->nr_contig_chunks) {
    6a06:	49 8b 7d 08          	mov    0x8(%r13),%rdi
#if !defined(_MIC_SCIF_) && defined(CONFIG_ML1OM)
			is_self_scifdev(ep->remote_dev) ?
				micscif_get_dma_addr(window, offset + (i * PAGE_SIZE),
				NULL, NULL, NULL) : window->phys_addr[i];
#else
			get_phys_addr(micscif_get_dma_addr(window, offset + (i * PAGE_SIZE),
    6a0a:	4d 8b 94 24 48 01 00 	mov    0x148(%r12),%r10
    6a11:	00 
	(*pages)->cookie = window;
	(*pages)->nr_pages = nr_pages;
	(*pages)->prot_flags = window->prot;

	for (i = 0; i < nr_pages; i++) {
		(*pages)->phys_addr[i] =
    6a12:	4c 03 48 10          	add    0x10(%rax),%r9
    6a16:	49 8b 45 00          	mov    0x0(%r13),%rax
    6a1a:	48 39 f8             	cmp    %rdi,%rax
    6a1d:	0f 84 3d 01 00 00    	je     6b60 <__scif_get_pages+0x3e0>
		return window->dma_addr[page_nr] | page_off;
	} else {
		int i = index ? *index : 0;
		uint64_t end;
		uint64_t start = start_off ? *start_off : window->offset;
		for (; i < window->nr_contig_chunks; i++) {
    6a23:	48 85 ff             	test   %rdi,%rdi
        }
		return window->dma_addr[page_nr] | page_off;
	} else {
		int i = index ? *index : 0;
		uint64_t end;
		uint64_t start = start_off ? *start_off : window->offset;
    6a26:	49 8b 45 20          	mov    0x20(%r13),%rax
		for (; i < window->nr_contig_chunks; i++) {
    6a2a:	0f 8e 54 03 00 00    	jle    6d84 <__scif_get_pages+0x604>
			end = start + (window->num_pages[i] << PAGE_SHIFT);
    6a30:	4d 8b 9d ee 00 00 00 	mov    0xee(%r13),%r11
    6a37:	41 8b 33             	mov    (%r11),%esi
    6a3a:	c1 e6 0c             	shl    $0xc,%esi
    6a3d:	48 63 f6             	movslq %esi,%rsi
    6a40:	48 01 c6             	add    %rax,%rsi
			if (off >= start && off < end) {
    6a43:	48 39 f3             	cmp    %rsi,%rbx
    6a46:	73 09                	jae    6a51 <__scif_get_pages+0x2d1>
    6a48:	48 39 c3             	cmp    %rax,%rbx
    6a4b:	0f 83 e3 00 00 00    	jae    6b34 <__scif_get_pages+0x3b4>
    6a51:	31 c9                	xor    %ecx,%ecx
    6a53:	eb 06                	jmp    6a5b <__scif_get_pages+0x2db>
    6a55:	0f 1f 00             	nopl   (%rax)
	} else {
		int i = index ? *index : 0;
		uint64_t end;
		uint64_t start = start_off ? *start_off : window->offset;
		for (; i < window->nr_contig_chunks; i++) {
			end = start + (window->num_pages[i] << PAGE_SHIFT);
    6a58:	48 89 c6             	mov    %rax,%rsi
		return window->dma_addr[page_nr] | page_off;
	} else {
		int i = index ? *index : 0;
		uint64_t end;
		uint64_t start = start_off ? *start_off : window->offset;
		for (; i < window->nr_contig_chunks; i++) {
    6a5b:	83 c1 01             	add    $0x1,%ecx
    6a5e:	48 63 d1             	movslq %ecx,%rdx
    6a61:	48 39 d7             	cmp    %rdx,%rdi
    6a64:	0f 8e 1a 03 00 00    	jle    6d84 <__scif_get_pages+0x604>
			end = start + (window->num_pages[i] << PAGE_SHIFT);
    6a6a:	41 8b 04 93          	mov    (%r11,%rdx,4),%eax
    6a6e:	c1 e0 0c             	shl    $0xc,%eax
    6a71:	48 98                	cltq   
    6a73:	48 01 f0             	add    %rsi,%rax
			if (off >= start && off < end) {
    6a76:	48 39 c3             	cmp    %rax,%rbx
    6a79:	73 dd                	jae    6a58 <__scif_get_pages+0x2d8>
    6a7b:	48 39 f3             	cmp    %rsi,%rbx
    6a7e:	72 d8                	jb     6a58 <__scif_get_pages+0x2d8>
    6a80:	48 c1 e2 03          	shl    $0x3,%rdx
					*index = i;
				if (start_off)
					*start_off = start;
				if (nr_bytes)
					*nr_bytes = end - off;
				return (window->dma_addr[i] + (off - start));
    6a84:	49 8b 85 e6 00 00 00 	mov    0xe6(%r13),%rax
    6a8b:	48 89 d9             	mov    %rbx,%rcx
    6a8e:	48 29 f1             	sub    %rsi,%rcx
    6a91:	48 03 0c 10          	add    (%rax,%rdx,1),%rcx
    6a95:	49 89 cd             	mov    %rcx,%r13
    6a98:	41 0f b7 12          	movzwl (%r10),%edx
static __always_inline phys_addr_t
get_phys_addr(phys_addr_t phys, struct micscif_dev *dev)
{
	phys_addr_t out_phys;

	if (is_self_scifdev(dev))
    6a9c:	3b 15 00 00 00 00    	cmp    0x0(%rip),%edx        # 6aa2 <__scif_get_pages+0x322>
    6aa2:	89 d0                	mov    %edx,%eax
    6aa4:	0f 85 96 00 00 00    	jne    6b40 <__scif_get_pages+0x3c0>
    6aaa:	4d 89 29             	mov    %r13,(%r9)
    6aad:	49 8b 84 24 48 01 00 	mov    0x148(%r12),%rax
    6ab4:	00 
    6ab5:	0f b7 10             	movzwl (%rax),%edx
#else
			get_phys_addr(micscif_get_dma_addr(window, offset + (i * PAGE_SIZE),
				NULL, NULL, NULL), ep->remote_dev);
#endif
#ifndef _MIC_SCIF_
		if (!is_self_scifdev(ep->remote_dev))
    6ab8:	3b 15 00 00 00 00    	cmp    0x0(%rip),%edx        # 6abe <__scif_get_pages+0x33e>
    6abe:	89 d0                	mov    %edx,%eax
    6ac0:	74 55                	je     6b17 <__scif_get_pages+0x397>
			(*pages)->va[i] =
    6ac2:	49 8b 17             	mov    (%r15),%rdx
    6ac5:	4c 89 f7             	mov    %r14,%rdi
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.va +
    6ac8:	83 e8 01             	sub    $0x1,%eax
			get_phys_addr(micscif_get_dma_addr(window, offset + (i * PAGE_SIZE),
				NULL, NULL, NULL), ep->remote_dev);
#endif
#ifndef _MIC_SCIF_
		if (!is_self_scifdev(ep->remote_dev))
			(*pages)->va[i] =
    6acb:	48 03 7a 18          	add    0x18(%rdx),%rdi
    6acf:	48 89 7d 80          	mov    %rdi,-0x80(%rbp)
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.va +
    6ad3:	0f b7 f8             	movzwl %ax,%edi
    6ad6:	e8 00 00 00 00       	callq  6adb <__scif_get_pages+0x35b>
    6adb:	48 8b 40 18          	mov    0x18(%rax),%rax
    6adf:	48 89 45 88          	mov    %rax,-0x78(%rbp)
				(*pages)->phys_addr[i] -
    6ae3:	49 8b 07             	mov    (%r15),%rax
    6ae6:	48 8b 40 10          	mov    0x10(%rax),%rax
    6aea:	4e 8b 2c 30          	mov    (%rax,%r14,1),%r13
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.pa;
    6aee:	49 8b 84 24 48 01 00 	mov    0x148(%r12),%rax
    6af5:	00 
    6af6:	0f b7 38             	movzwl (%rax),%edi
    6af9:	83 ef 01             	sub    $0x1,%edi
    6afc:	0f b7 ff             	movzwl %di,%edi
    6aff:	e8 00 00 00 00       	callq  6b04 <__scif_get_pages+0x384>
#endif
#ifndef _MIC_SCIF_
		if (!is_self_scifdev(ep->remote_dev))
			(*pages)->va[i] =
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.va +
				(*pages)->phys_addr[i] -
    6b04:	48 8b 7d 88          	mov    -0x78(%rbp),%rdi
    6b08:	4c 2b 68 20          	sub    0x20(%rax),%r13
    6b0c:	48 8b 55 80          	mov    -0x80(%rbp),%rdx
    6b10:	4e 8d 2c 2f          	lea    (%rdi,%r13,1),%r13
    6b14:	4c 89 2a             	mov    %r13,(%rdx)
    6b17:	48 81 c3 00 10 00 00 	add    $0x1000,%rbx
	/* Populate the values */
	(*pages)->cookie = window;
	(*pages)->nr_pages = nr_pages;
	(*pages)->prot_flags = window->prot;

	for (i = 0; i < nr_pages; i++) {
    6b1e:	4c 3b b5 78 ff ff ff 	cmp    -0x88(%rbp),%r14
    6b25:	74 6c                	je     6b93 <__scif_get_pages+0x413>
    6b27:	4c 8b 6d 98          	mov    -0x68(%rbp),%r13
    6b2b:	49 83 c6 08          	add    $0x8,%r14
    6b2f:	e9 cc fe ff ff       	jmpq   6a00 <__scif_get_pages+0x280>
		int i = index ? *index : 0;
		uint64_t end;
		uint64_t start = start_off ? *start_off : window->offset;
		for (; i < window->nr_contig_chunks; i++) {
			end = start + (window->num_pages[i] << PAGE_SHIFT);
			if (off >= start && off < end) {
    6b34:	48 89 c6             	mov    %rax,%rsi
    6b37:	31 d2                	xor    %edx,%edx
    6b39:	e9 46 ff ff ff       	jmpq   6a84 <__scif_get_pages+0x304>
    6b3e:	66 90                	xchg   %ax,%ax
		out_phys = phys;
	else {
		phys_addr_t __apt_base =
		(phys_addr_t)get_per_dev_ctx(dev->sd_node - 1)->aper.pa;
    6b40:	83 e8 01             	sub    $0x1,%eax
    6b43:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
    6b47:	0f b7 f8             	movzwl %ax,%edi
    6b4a:	e8 00 00 00 00       	callq  6b4f <__scif_get_pages+0x3cf>
		out_phys = phys + __apt_base;
    6b4f:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    6b53:	4c 03 68 20          	add    0x20(%rax),%r13
    6b57:	e9 4e ff ff ff       	jmpq   6aaa <__scif_get_pages+0x32a>
    6b5c:	0f 1f 40 00          	nopl   0x0(%rax)
 */
static __always_inline dma_addr_t
micscif_get_dma_addr(struct reg_range_t *window, uint64_t off, size_t *nr_bytes, int *index, uint64_t *start_off)
{
	if (window->nr_pages == window->nr_contig_chunks) {
		int page_nr = (int)((off - window->offset) >> PAGE_SHIFT);
    6b60:	48 89 da             	mov    %rbx,%rdx
    6b63:	49 2b 55 20          	sub    0x20(%r13),%rdx
		off_t page_off = off & ~PAGE_MASK;
    6b67:	48 89 d9             	mov    %rbx,%rcx
    6b6a:	81 e1 ff 0f 00 00    	and    $0xfff,%ecx
 */
static __always_inline dma_addr_t
micscif_get_dma_addr(struct reg_range_t *window, uint64_t off, size_t *nr_bytes, int *index, uint64_t *start_off)
{
	if (window->nr_pages == window->nr_contig_chunks) {
		int page_nr = (int)((off - window->offset) >> PAGE_SHIFT);
    6b70:	48 c1 ea 0c          	shr    $0xc,%rdx
		off_t page_off = off & ~PAGE_MASK;
		if (nr_bytes)
			*nr_bytes = PAGE_SIZE - page_off;
        if (page_nr >= window->nr_pages) {
    6b74:	48 63 d2             	movslq %edx,%rdx
    6b77:	48 39 d0             	cmp    %rdx,%rax
    6b7a:	0f 8e ac 02 00 00    	jle    6e2c <__scif_get_pages+0x6ac>
            printk(KERN_ERR "%s dma_addr out of boundary\n", __FUNCTION__);
        }
		return window->dma_addr[page_nr] | page_off;
    6b80:	49 8b 85 e6 00 00 00 	mov    0xe6(%r13),%rax
    6b87:	48 0b 0c d0          	or     (%rax,%rdx,8),%rcx
    6b8b:	49 89 cd             	mov    %rcx,%r13
    6b8e:	e9 05 ff ff ff       	jmpq   6a98 <__scif_get_pages+0x318>
    6b93:	4c 8b 6d 98          	mov    -0x68(%rbp),%r13
    6b97:	4d 89 e1             	mov    %r12,%r9
				(*pages)->phys_addr[i] -
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.pa;
#endif
	}

	window->get_put_ref_count += nr_pages;
    6b9a:	48 8b 9d 70 ff ff ff 	mov    -0x90(%rbp),%rbx
    6ba1:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
	get_window_ref_count(window, nr_pages);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    6ba5:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
				(*pages)->phys_addr[i] -
				get_per_dev_ctx(ep->remote_dev->sd_node - 1)->aper.pa;
#endif
	}

	window->get_put_ref_count += nr_pages;
    6bac:	41 01 9d 86 00 00 00 	add    %ebx,0x86(%r13)
}

static __always_inline void
get_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
	window->ref_count += (int)nr_pages;
    6bb3:	41 01 5d 14          	add    %ebx,0x14(%r13)
	get_window_ref_count(window, nr_pages);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    6bb7:	e8 00 00 00 00       	callq  6bbc <__scif_get_pages+0x43c>
			kfree(*pages);
			*pages = NULL;
		}
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	} else {
		micscif_create_node_dep(ep->remote_dev, nr_pages);
    6bbc:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    6bc0:	89 de                	mov    %ebx,%esi
    6bc2:	49 8b b9 48 01 00 00 	mov    0x148(%r9),%rdi
    6bc9:	e8 00 00 00 00       	callq  6bce <__scif_get_pages+0x44e>
    6bce:	31 c0                	xor    %eax,%eax
    6bd0:	e9 e3 fb ff ff       	jmpq   67b8 <__scif_get_pages+0x38>
		err = -ENOMEM;
		goto error;
	}

	/* Allocate phys addr array */
	if (!((*pages)->phys_addr = scif_zalloc(nr_pages * sizeof(dma_addr_t)))) {
    6bd5:	49 c7 46 10 00 00 00 	movq   $0x0,0x10(%r14)
    6bdc:	00 
	}

	window->get_put_ref_count += nr_pages;
	get_window_ref_count(window, nr_pages);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    6bdd:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    6be4:	41 bd f4 ff ff ff    	mov    $0xfffffff4,%r13d
    6bea:	e8 00 00 00 00       	callq  6bef <__scif_get_pages+0x46f>
	if (err) {
		if (*pages) {
    6bef:	49 8b 04 24          	mov    (%r12),%rax
    6bf3:	48 85 c0             	test   %rax,%rax
    6bf6:	0f 84 c8 00 00 00    	je     6cc4 <__scif_get_pages+0x544>
			if ((*pages)->phys_addr)
    6bfc:	48 8b 78 10          	mov    0x10(%rax),%rdi
    6c00:	48 85 ff             	test   %rdi,%rdi
    6c03:	74 53                	je     6c58 <__scif_get_pages+0x4d8>
				scif_free((*pages)->phys_addr, nr_pages * sizeof(dma_addr_t));
    6c05:	48 63 85 70 ff ff ff 	movslq -0x90(%rbp),%rax
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
    6c0c:	be ff ff ff ff       	mov    $0xffffffff,%esi
    6c11:	48 ba 00 00 00 00 00 	movabs $0x370000000000,%rdx
    6c18:	37 00 00 
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
    6c1b:	48 b9 fe ff ff ff ff 	movabs $0x1ffffffffffe,%rcx
    6c22:	1f 00 00 
    6c25:	48 01 fa             	add    %rdi,%rdx
 * @size: Size of the allocation.
 * Helper API which frees memory allocated via scif_zalloc().
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);
    6c28:	48 8d 04 c5 ff 0f 00 	lea    0xfff(,%rax,8),%rax
    6c2f:	00 
    6c30:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    6c36:	48 83 e8 01          	sub    $0x1,%rax
    6c3a:	48 c1 e8 0b          	shr    $0xb,%rax

	if (unlikely(is_vmalloc_addr(addr)))
    6c3e:	48 39 ca             	cmp    %rcx,%rdx
    6c41:	0f 86 2b 02 00 00    	jbe    6e72 <__scif_get_pages+0x6f2>
	order = -1;
	do {
		size >>= 1;
		order++;
    6c47:	83 c6 01             	add    $0x1,%esi
	} while (size);
    6c4a:	48 d1 e8             	shr    %rax
    6c4d:	75 f8                	jne    6c47 <__scif_get_pages+0x4c7>
		vfree(addr);
	else {
		free_pages((unsigned long)addr, get_order(align));
    6c4f:	e8 00 00 00 00       	callq  6c54 <__scif_get_pages+0x4d4>
    6c54:	49 8b 04 24          	mov    (%r12),%rax
#ifndef _MIC_SCIF_
			if ((*pages)->va)
    6c58:	48 8b 78 18          	mov    0x18(%rax),%rdi
    6c5c:	48 85 ff             	test   %rdi,%rdi
    6c5f:	74 53                	je     6cb4 <__scif_get_pages+0x534>
				scif_free((*pages)->va, nr_pages * sizeof(void *));
    6c61:	48 63 85 70 ff ff ff 	movslq -0x90(%rbp),%rax
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
    6c68:	be ff ff ff ff       	mov    $0xffffffff,%esi
    6c6d:	48 ba 00 00 00 00 00 	movabs $0x370000000000,%rdx
    6c74:	37 00 00 
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
    6c77:	48 b9 fe ff ff ff ff 	movabs $0x1ffffffffffe,%rcx
    6c7e:	1f 00 00 
    6c81:	48 01 fa             	add    %rdi,%rdx
 * @size: Size of the allocation.
 * Helper API which frees memory allocated via scif_zalloc().
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);
    6c84:	48 8d 04 c5 ff 0f 00 	lea    0xfff(,%rax,8),%rax
    6c8b:	00 
    6c8c:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    6c92:	48 83 e8 01          	sub    $0x1,%rax
    6c96:	48 c1 e8 0b          	shr    $0xb,%rax

	if (unlikely(is_vmalloc_addr(addr)))
    6c9a:	48 39 ca             	cmp    %rcx,%rdx
    6c9d:	0f 86 dd 01 00 00    	jbe    6e80 <__scif_get_pages+0x700>
	order = -1;
	do {
		size >>= 1;
		order++;
    6ca3:	83 c6 01             	add    $0x1,%esi
	} while (size);
    6ca6:	48 d1 e8             	shr    %rax
    6ca9:	75 f8                	jne    6ca3 <__scif_get_pages+0x523>
		vfree(addr);
	else {
		free_pages((unsigned long)addr, get_order(align));
    6cab:	e8 00 00 00 00       	callq  6cb0 <__scif_get_pages+0x530>
    6cb0:	49 8b 04 24          	mov    (%r12),%rax
#endif
			kfree(*pages);
    6cb4:	48 89 c7             	mov    %rax,%rdi
    6cb7:	e8 00 00 00 00       	callq  6cbc <__scif_get_pages+0x53c>
			*pages = NULL;
    6cbc:	49 c7 04 24 00 00 00 	movq   $0x0,(%r12)
    6cc3:	00 
		}
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    6cc4:	44 89 e9             	mov    %r13d,%ecx
    6cc7:	ba b9 08 00 00       	mov    $0x8b9,%edx
    6ccc:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6cd3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6cda:	31 c0                	xor    %eax,%eax
    6cdc:	e8 00 00 00 00       	callq  6ce1 <__scif_get_pages+0x561>
    6ce1:	44 89 e8             	mov    %r13d,%eax
    6ce4:	e9 cf fa ff ff       	jmpq   67b8 <__scif_get_pages+0x38>
    6ce9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	if ((!len) ||
		(offset < 0) ||
		(offset + len < offset) ||
		(align_low((uint64_t)offset, PAGE_SIZE) != (uint64_t)offset) ||
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;
    6cf0:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    6cf5:	e9 be fa ff ff       	jmpq   67b8 <__scif_get_pages+0x38>
		goto error;
	}
	RMA_MAGIC(window);

	/* Allocate scif_range */
	if (!(*pages = kzalloc(sizeof(struct scif_range), GFP_KERNEL))) {
    6cfa:	49 c7 04 24 10 00 00 	movq   $0x10,(%r12)
    6d01:	00 

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
    6d02:	41 be 10 00 00 00    	mov    $0x10,%r14d
    6d08:	e9 ea fb ff ff       	jmpq   68f7 <__scif_get_pages+0x177>

	if (align <= (1 << (MAX_ORDER + PAGE_SHIFT - 1)))
		if ((ret = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
						get_order(align))))
			goto done;
	if (!(ret = vmalloc(align)))
    6d0d:	48 8b 7d 88          	mov    -0x78(%rbp),%rdi
    6d11:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6d15:	e8 00 00 00 00       	callq  6d1a <__scif_get_pages+0x59a>
    6d1a:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
    6d1e:	48 85 c0             	test   %rax,%rax
    6d21:	49 89 c7             	mov    %rax,%r15
    6d24:	0f 84 ab fe ff ff    	je     6bd5 <__scif_get_pages+0x455>
		return NULL;

	/* TODO: Use vzalloc once kernel supports it */
	memset(ret, 0, size);
    6d2a:	4c 89 ea             	mov    %r13,%rdx
    6d2d:	31 f6                	xor    %esi,%esi
    6d2f:	4c 89 ff             	mov    %r15,%rdi
    6d32:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6d36:	e8 00 00 00 00       	callq  6d3b <__scif_get_pages+0x5bb>
		err = -ENOMEM;
		goto error;
	}

	/* Allocate phys addr array */
	if (!((*pages)->phys_addr = scif_zalloc(nr_pages * sizeof(dma_addr_t)))) {
    6d3b:	4d 89 7e 10          	mov    %r15,0x10(%r14)
    6d3f:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
		goto error;
	}

#ifndef _MIC_SCIF_
	/* Allocate virtual address array */
	if (!((*pages)->va = scif_zalloc(nr_pages * sizeof(void *)))) {
    6d43:	4d 8b 34 24          	mov    (%r12),%r14

	if (align <= (1 << (MAX_ORDER + PAGE_SHIFT - 1)))
		if ((ret = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
						get_order(align))))
			goto done;
	if (!(ret = vmalloc(align)))
    6d47:	48 8b 7d 88          	mov    -0x78(%rbp),%rdi
    6d4b:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6d4f:	e8 00 00 00 00       	callq  6d54 <__scif_get_pages+0x5d4>
    6d54:	48 85 c0             	test   %rax,%rax
    6d57:	49 89 c7             	mov    %rax,%r15
    6d5a:	74 19                	je     6d75 <__scif_get_pages+0x5f5>
		return NULL;

	/* TODO: Use vzalloc once kernel supports it */
	memset(ret, 0, size);
    6d5c:	4c 89 ea             	mov    %r13,%rdx
    6d5f:	31 f6                	xor    %esi,%esi
    6d61:	48 89 c7             	mov    %rax,%rdi
    6d64:	e8 00 00 00 00       	callq  6d69 <__scif_get_pages+0x5e9>
    6d69:	4c 89 f8             	mov    %r15,%rax
    6d6c:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
    6d70:	e9 36 fc ff ff       	jmpq   69ab <__scif_get_pages+0x22b>
    6d75:	49 c7 46 18 00 00 00 	movq   $0x0,0x18(%r14)
    6d7c:	00 
    6d7d:	e9 5b fe ff ff       	jmpq   6bdd <__scif_get_pages+0x45d>
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	RMA_MAGIC(window);
    6d82:	0f 0b                	ud2    
			}
			start += (window->num_pages[i] << PAGE_SHIFT);
		}
	}
#ifdef CONFIG_MK1OM
	printk(KERN_ERR "%s %d BUG. Addr not found? window %p off 0x%llx\n", __func__, __LINE__, window, off);
    6d84:	49 89 d8             	mov    %rbx,%r8
    6d87:	4c 89 e9             	mov    %r13,%rcx
    6d8a:	31 c0                	xor    %eax,%eax
    6d8c:	ba 14 03 00 00       	mov    $0x314,%edx
    6d91:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6d98:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6d9f:	e8 00 00 00 00       	callq  6da4 <__scif_get_pages+0x624>
	BUG_ON(1);
    6da4:	0f 0b                	ud2    

	/* Allocate scif_range */
	if (!(*pages = kzalloc(sizeof(struct scif_range), GFP_KERNEL))) {
		err = -ENOMEM;
    6da6:	41 bd f4 ff ff ff    	mov    $0xfffffff4,%r13d
	}

	window->get_put_ref_count += nr_pages;
	get_window_ref_count(window, nr_pages);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    6dac:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    6db3:	e8 00 00 00 00       	callq  6db8 <__scif_get_pages+0x638>
    6db8:	e9 32 fe ff ff       	jmpq   6bef <__scif_get_pages+0x46f>

	if (align <= (1 << (MAX_ORDER + PAGE_SHIFT - 1)))
		if ((ret = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
						get_order(align))))
			goto done;
	if (!(ret = vmalloc(align)))
    6dbd:	48 8b 7d 88          	mov    -0x78(%rbp),%rdi
    6dc1:	4c 89 4d 80          	mov    %r9,-0x80(%rbp)
    6dc5:	e8 00 00 00 00       	callq  6dca <__scif_get_pages+0x64a>
    6dca:	4c 8b 4d 80          	mov    -0x80(%rbp),%r9
    6dce:	48 85 c0             	test   %rax,%rax
    6dd1:	48 89 c1             	mov    %rax,%rcx
    6dd4:	0f 84 fb fd ff ff    	je     6bd5 <__scif_get_pages+0x455>
		return NULL;

	/* TODO: Use vzalloc once kernel supports it */
	memset(ret, 0, size);
    6dda:	48 89 cf             	mov    %rcx,%rdi
    6ddd:	4c 89 ea             	mov    %r13,%rdx
    6de0:	31 f6                	xor    %esi,%esi
    6de2:	4c 89 8d 78 ff ff ff 	mov    %r9,-0x88(%rbp)
    6de9:	48 89 4d 80          	mov    %rcx,-0x80(%rbp)
		goto error;
	}

#ifndef _MIC_SCIF_
	/* Allocate virtual address array */
	if (!((*pages)->va = scif_zalloc(nr_pages * sizeof(void *)))) {
    6ded:	49 c1 ef 0b          	shr    $0xb,%r15
    6df1:	e8 00 00 00 00       	callq  6df6 <__scif_get_pages+0x676>
		err = -ENOMEM;
		goto error;
	}

	/* Allocate phys addr array */
	if (!((*pages)->phys_addr = scif_zalloc(nr_pages * sizeof(dma_addr_t)))) {
    6df6:	48 8b 4d 80          	mov    -0x80(%rbp),%rcx
    6dfa:	4c 8b 8d 78 ff ff ff 	mov    -0x88(%rbp),%r9
    6e01:	49 89 4e 10          	mov    %rcx,0x10(%r14)
		goto error;
	}

#ifndef _MIC_SCIF_
	/* Allocate virtual address array */
	if (!((*pages)->va = scif_zalloc(nr_pages * sizeof(void *)))) {
    6e05:	4d 8b 34 24          	mov    (%r12),%r14
    6e09:	e9 6e fb ff ff       	jmpq   697c <__scif_get_pages+0x1fc>
	req.head = &ep->rma_info.remote_reg_list;

	mutex_lock(&ep->rma_info.rma_lock);
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    6e0e:	89 c1                	mov    %eax,%ecx
    6e10:	ba 74 08 00 00       	mov    $0x874,%edx
    6e15:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6e1c:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6e23:	31 c0                	xor    %eax,%eax
    6e25:	e8 00 00 00 00       	callq  6e2a <__scif_get_pages+0x6aa>
		goto error;
    6e2a:	eb 80                	jmp    6dac <__scif_get_pages+0x62c>
		int page_nr = (int)((off - window->offset) >> PAGE_SHIFT);
		off_t page_off = off & ~PAGE_MASK;
		if (nr_bytes)
			*nr_bytes = PAGE_SIZE - page_off;
        if (page_nr >= window->nr_pages) {
            printk(KERN_ERR "%s dma_addr out of boundary\n", __FUNCTION__);
    6e2c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6e33:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    6e3a:	31 c0                	xor    %eax,%eax
    6e3c:	48 89 95 58 ff ff ff 	mov    %rdx,-0xa8(%rbp)
    6e43:	48 89 8d 60 ff ff ff 	mov    %rcx,-0xa0(%rbp)
    6e4a:	4c 89 55 80          	mov    %r10,-0x80(%rbp)
    6e4e:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
    6e52:	e8 00 00 00 00       	callq  6e57 <__scif_get_pages+0x6d7>
    6e57:	48 8b 95 58 ff ff ff 	mov    -0xa8(%rbp),%rdx
    6e5e:	48 8b 8d 60 ff ff ff 	mov    -0xa0(%rbp),%rcx
    6e65:	4c 8b 55 80          	mov    -0x80(%rbp),%r10
    6e69:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    6e6d:	e9 0e fd ff ff       	jmpq   6b80 <__scif_get_pages+0x400>
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
		vfree(addr);
    6e72:	e8 00 00 00 00       	callq  6e77 <__scif_get_pages+0x6f7>
    6e77:	49 8b 04 24          	mov    (%r12),%rax
    6e7b:	e9 d8 fd ff ff       	jmpq   6c58 <__scif_get_pages+0x4d8>
    6e80:	e8 00 00 00 00       	callq  6e85 <__scif_get_pages+0x705>
    6e85:	49 8b 04 24          	mov    (%r12),%rax
    6e89:	e9 26 fe ff ff       	jmpq   6cb4 <__scif_get_pages+0x534>
    6e8e:	66 90                	xchg   %ax,%ax

0000000000006e90 <scif_get_pages>:
	return err;
}

int
scif_get_pages(scif_epd_t epd, off_t offset, size_t len, struct scif_range **pages)
{
    6e90:	55                   	push   %rbp
    6e91:	48 89 e5             	mov    %rsp,%rbp
    6e94:	41 57                	push   %r15
    6e96:	41 56                	push   %r14
    6e98:	41 55                	push   %r13
    6e9a:	41 54                	push   %r12
    6e9c:	53                   	push   %rbx
    6e9d:	48 83 ec 08          	sub    $0x8,%rsp
    6ea1:	e8 00 00 00 00       	callq  6ea6 <scif_get_pages+0x16>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    6ea6:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    6ead:	49 89 fc             	mov    %rdi,%r12
    6eb0:	49 89 f5             	mov    %rsi,%r13
    6eb3:	49 89 d6             	mov    %rdx,%r14
    6eb6:	49 89 cf             	mov    %rcx,%r15
    6eb9:	48 89 df             	mov    %rbx,%rdi
    6ebc:	e8 00 00 00 00       	callq  6ec1 <scif_get_pages+0x31>
	int ret;
	get_kref_count(epd);
	ret = __scif_get_pages(epd, offset, len, pages);
    6ec1:	4c 89 f9             	mov    %r15,%rcx
    6ec4:	4c 89 f2             	mov    %r14,%rdx
    6ec7:	4c 89 ee             	mov    %r13,%rsi
    6eca:	4c 89 e7             	mov    %r12,%rdi
    6ecd:	e8 00 00 00 00       	callq  6ed2 <scif_get_pages+0x42>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    6ed2:	48 89 df             	mov    %rbx,%rdi
    6ed5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    6edc:	41 89 c4             	mov    %eax,%r12d
    6edf:	e8 00 00 00 00       	callq  6ee4 <scif_get_pages+0x54>
	put_kref_count(epd);
	return ret;
}
    6ee4:	48 83 c4 08          	add    $0x8,%rsp
    6ee8:	44 89 e0             	mov    %r12d,%eax
    6eeb:	5b                   	pop    %rbx
    6eec:	41 5c                	pop    %r12
    6eee:	41 5d                	pop    %r13
    6ef0:	41 5e                	pop    %r14
    6ef2:	41 5f                	pop    %r15
    6ef4:	5d                   	pop    %rbp
    6ef5:	c3                   	retq   
    6ef6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    6efd:	00 00 00 

0000000000006f00 <__scif_put_pages>:
 *	Upon success, zero is returned.
 *	else an apt error is returned as documented in scif.h.
 */
int
__scif_put_pages(struct scif_range *pages)
{
    6f00:	55                   	push   %rbp
    6f01:	48 89 e5             	mov    %rsp,%rbp
    6f04:	41 57                	push   %r15
    6f06:	41 56                	push   %r14
    6f08:	41 55                	push   %r13
    6f0a:	41 54                	push   %r12
    6f0c:	53                   	push   %rbx
    6f0d:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    6f14:	e8 00 00 00 00       	callq  6f19 <__scif_put_pages+0x19>
	struct endpt *ep;
	struct reg_range_t *window;
	struct nodemsg msg;

	if (!pages || !pages->cookie)
    6f19:	48 85 ff             	test   %rdi,%rdi
    6f1c:	0f 84 36 02 00 00    	je     7158 <__scif_put_pages+0x258>
    6f22:	4c 8b 27             	mov    (%rdi),%r12
    6f25:	4d 85 e4             	test   %r12,%r12
    6f28:	0f 84 2a 02 00 00    	je     7158 <__scif_put_pages+0x258>
		return -EINVAL;

	window = pages->cookie;

	if (!window || window->magic != SCIFEP_MAGIC ||
    6f2e:	48 b8 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rax
    6f35:	00 1f 5c 
    6f38:	49 39 44 24 18       	cmp    %rax,0x18(%r12)
    6f3d:	0f 85 15 02 00 00    	jne    7158 <__scif_put_pages+0x258>
    6f43:	41 8b 94 24 86 00 00 	mov    0x86(%r12),%edx
    6f4a:	00 
    6f4b:	85 d2                	test   %edx,%edx
    6f4d:	0f 84 05 02 00 00    	je     7158 <__scif_put_pages+0x258>
		!window->get_put_ref_count)
		return -EINVAL;

	ep = (struct endpt *)window->ep;
    6f53:	4d 8b 6c 24 34       	mov    0x34(%r12),%r13
	 * If the state is SCIFEP_CONNECTED or SCIFEP_DISCONNECTED then the
	 * callee should be allowed to release references to the pages,
	 * else the endpoint was not connected in the first place,
	 * hence the ENOTCONN.
	 */
	if (ep->state != SCIFEP_CONNECTED && ep->state != SCIFEP_DISCONNECTED)
    6f58:	41 8b 45 00          	mov    0x0(%r13),%eax
    6f5c:	83 f8 04             	cmp    $0x4,%eax
    6f5f:	74 0d                	je     6f6e <__scif_put_pages+0x6e>
    6f61:	41 8b 45 00          	mov    0x0(%r13),%eax
    6f65:	83 f8 09             	cmp    $0x9,%eax
    6f68:	0f 85 c8 02 00 00    	jne    7236 <__scif_put_pages+0x336>
	 * are called after the node is removed currently.
	 *	if (!scifdev_alive(ep))
	 *		return -ENODEV;
	 */

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    6f6e:	4d 8b b5 48 01 00 00 	mov    0x148(%r13),%r14
    6f75:	48 89 fb             	mov    %rdi,%rbx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    6f78:	4d 85 f6             	test   %r14,%r14
    6f7b:	0f 85 ef 01 00 00    	jne    7170 <__scif_put_pages+0x270>
	mutex_lock(&ep->rma_info.rma_lock);
    6f81:	4d 8d b5 80 00 00 00 	lea    0x80(%r13),%r14
    6f88:	4c 89 f7             	mov    %r14,%rdi
    6f8b:	e8 00 00 00 00       	callq  6f90 <__scif_put_pages+0x90>
#if !defined(_MIC_SCIF_) && defined(CONFIG_ML1OM)
	micscif_rma_list_gtt_unmap(window, window->offset, window->nr_pages);
#endif

	/* Decrement the ref counts and check for errors */
	window->get_put_ref_count -= pages->nr_pages;
    6f90:	41 8b 84 24 86 00 00 	mov    0x86(%r12),%eax
    6f97:	00 
    6f98:	2b 43 08             	sub    0x8(%rbx),%eax
	BUG_ON(window->get_put_ref_count < 0);
    6f9b:	85 c0                	test   %eax,%eax
#if !defined(_MIC_SCIF_) && defined(CONFIG_ML1OM)
	micscif_rma_list_gtt_unmap(window, window->offset, window->nr_pages);
#endif

	/* Decrement the ref counts and check for errors */
	window->get_put_ref_count -= pages->nr_pages;
    6f9d:	41 89 84 24 86 00 00 	mov    %eax,0x86(%r12)
    6fa4:	00 
	BUG_ON(window->get_put_ref_count < 0);
    6fa5:	0f 88 bd 01 00 00    	js     7168 <__scif_put_pages+0x268>
}

static __always_inline void
put_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
	window->ref_count -= (int)nr_pages; 
    6fab:	41 8b 44 24 14       	mov    0x14(%r12),%eax
    6fb0:	2b 43 08             	sub    0x8(%rbx),%eax
	BUG_ON(window->nr_pages < 0);
    6fb3:	49 83 3c 24 00       	cmpq   $0x0,(%r12)
}

static __always_inline void
put_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
	window->ref_count -= (int)nr_pages; 
    6fb8:	41 89 44 24 14       	mov    %eax,0x14(%r12)
	BUG_ON(window->nr_pages < 0);
    6fbd:	0f 88 5d 02 00 00    	js     7220 <__scif_put_pages+0x320>
	put_window_ref_count(window, pages->nr_pages);

	/* Initiate window destruction if ref count is zero */
	if (!window->ref_count) {
    6fc3:	85 c0                	test   %eax,%eax
    6fc5:	0f 84 15 01 00 00    	je     70e0 <__scif_put_pages+0x1e0>
		micscif_nodeqp_send(ep->remote_dev, &msg, ep);
		list_del(&window->list_member);
		/* Destroy this window from the peer's registered AS */
		micscif_destroy_remote_window(ep, window);
	}
	mutex_unlock(&ep->rma_info.rma_lock);
    6fcb:	4c 89 f7             	mov    %r14,%rdi
    6fce:	e8 00 00 00 00       	callq  6fd3 <__scif_put_pages+0xd3>

	micscif_dec_node_refcnt(ep->remote_dev, 1);
    6fd3:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    6fda:	4d 85 e4             	test   %r12,%r12
    6fdd:	74 2a                	je     7009 <__scif_put_pages+0x109>
		if (unlikely((atomic_long_sub_return(cnt, 
    6fdf:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    6fe6:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    6fe7:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    6fee:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    6ff5:	01 00 00 
    6ff8:	48 83 e8 01          	sub    $0x1,%rax
    6ffc:	0f 88 3e 02 00 00    	js     7240 <__scif_put_pages+0x340>
    7002:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
	micscif_destroy_node_dep(ep->remote_dev, pages->nr_pages);
    7009:	8b 73 08             	mov    0x8(%rbx),%esi
    700c:	4c 89 e7             	mov    %r12,%rdi
    700f:	e8 00 00 00 00       	callq  7014 <__scif_put_pages+0x114>
	scif_free(pages->phys_addr, pages->nr_pages * sizeof(dma_addr_t));
    7014:	48 63 43 08          	movslq 0x8(%rbx),%rax
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
    7018:	be ff ff ff ff       	mov    $0xffffffff,%esi
    701d:	48 ba 00 00 00 00 00 	movabs $0x370000000000,%rdx
    7024:	37 00 00 
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
    7027:	48 b9 fe ff ff ff ff 	movabs $0x1ffffffffffe,%rcx
    702e:	1f 00 00 
    7031:	48 8b 7b 10          	mov    0x10(%rbx),%rdi
 * @size: Size of the allocation.
 * Helper API which frees memory allocated via scif_zalloc().
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);
    7035:	48 8d 04 c5 ff 0f 00 	lea    0xfff(,%rax,8),%rax
    703c:	00 
    703d:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
    7043:	48 01 fa             	add    %rdi,%rdx
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    7046:	48 83 e8 01          	sub    $0x1,%rax
    704a:	48 c1 e8 0b          	shr    $0xb,%rax

	if (unlikely(is_vmalloc_addr(addr)))
    704e:	48 39 ca             	cmp    %rcx,%rdx
    7051:	0f 86 cb 01 00 00    	jbe    7222 <__scif_put_pages+0x322>
    7057:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    705e:	00 00 
	order = -1;
	do {
		size >>= 1;
		order++;
    7060:	83 c6 01             	add    $0x1,%esi
	} while (size);
    7063:	48 d1 e8             	shr    %rax
    7066:	75 f8                	jne    7060 <__scif_put_pages+0x160>
		vfree(addr);
	else {
		free_pages((unsigned long)addr, get_order(align));
    7068:	e8 00 00 00 00       	callq  706d <__scif_put_pages+0x16d>
#ifndef _MIC_SCIF_
	scif_free(pages->va, pages->nr_pages * sizeof(void*));
    706d:	48 63 43 08          	movslq 0x8(%rbx),%rax
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
    7071:	be ff ff ff ff       	mov    $0xffffffff,%esi
    7076:	48 ba 00 00 00 00 00 	movabs $0x370000000000,%rdx
    707d:	37 00 00 
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
    7080:	48 b9 fe ff ff ff ff 	movabs $0x1ffffffffffe,%rcx
    7087:	1f 00 00 
    708a:	48 8b 7b 18          	mov    0x18(%rbx),%rdi
 * @size: Size of the allocation.
 * Helper API which frees memory allocated via scif_zalloc().
 */
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);
    708e:	48 8d 04 c5 ff 0f 00 	lea    0xfff(,%rax,8),%rax
    7095:	00 
    7096:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
    709c:	48 01 fa             	add    %rdi,%rdx
/* Pure 2^n version of get_order */
static inline __attribute_const__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
    709f:	48 83 e8 01          	sub    $0x1,%rax
    70a3:	48 c1 e8 0b          	shr    $0xb,%rax

	if (unlikely(is_vmalloc_addr(addr)))
    70a7:	48 39 ca             	cmp    %rcx,%rdx
    70aa:	0f 86 7c 01 00 00    	jbe    722c <__scif_put_pages+0x32c>
	order = -1;
	do {
		size >>= 1;
		order++;
    70b0:	83 c6 01             	add    $0x1,%esi
	} while (size);
    70b3:	48 d1 e8             	shr    %rax
    70b6:	75 f8                	jne    70b0 <__scif_put_pages+0x1b0>
		vfree(addr);
	else {
		free_pages((unsigned long)addr, get_order(align));
    70b8:	e8 00 00 00 00       	callq  70bd <__scif_put_pages+0x1bd>
#endif
	kfree(pages);
    70bd:	48 89 df             	mov    %rbx,%rdi
    70c0:	e8 00 00 00 00       	callq  70c5 <__scif_put_pages+0x1c5>
	return 0;
    70c5:	31 c0                	xor    %eax,%eax
}
    70c7:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    70ce:	5b                   	pop    %rbx
    70cf:	41 5c                	pop    %r12
    70d1:	41 5d                	pop    %r13
    70d3:	41 5e                	pop    %r14
    70d5:	41 5f                	pop    %r15
    70d7:	5d                   	pop    %rbp
    70d8:	c3                   	retq   
    70d9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
	BUG_ON(window->get_put_ref_count < 0);
	put_window_ref_count(window, pages->nr_pages);

	/* Initiate window destruction if ref count is zero */
	if (!window->ref_count) {
		drain_dma_intr(ep->rma_info.dma_chan);
    70e0:	49 8b bd 28 01 00 00 	mov    0x128(%r13),%rdi
    70e7:	e8 00 00 00 00       	callq  70ec <__scif_put_pages+0x1ec>
		/* Inform the peer about this window being destroyed. */
		msg.uop = SCIF_MUNMAP;
    70ec:	c7 45 ac 1b 00 00 00 	movl   $0x1b,-0x54(%rbp)
		msg.src = ep->port;
    70f3:	41 8b 45 06          	mov    0x6(%r13),%eax
		msg.payload[0] = window->peer_window;
		/* No error handling for notification messages */
		micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    70f7:	4c 89 ea             	mov    %r13,%rdx
    70fa:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
	/* Initiate window destruction if ref count is zero */
	if (!window->ref_count) {
		drain_dma_intr(ep->rma_info.dma_chan);
		/* Inform the peer about this window being destroyed. */
		msg.uop = SCIF_MUNMAP;
		msg.src = ep->port;
    70fe:	89 45 a4             	mov    %eax,-0x5c(%rbp)
		msg.payload[0] = window->peer_window;
    7101:	49 8b 44 24 50       	mov    0x50(%r12),%rax
    7106:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
		/* No error handling for notification messages */
		micscif_nodeqp_send(ep->remote_dev, &msg, ep);
    710a:	49 8b bd 48 01 00 00 	mov    0x148(%r13),%rdi
    7111:	e8 00 00 00 00       	callq  7116 <__scif_put_pages+0x216>
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
    7116:	49 8b 44 24 44       	mov    0x44(%r12),%rax
		list_del(&window->list_member);
		/* Destroy this window from the peer's registered AS */
		micscif_destroy_remote_window(ep, window);
    711b:	4c 89 e6             	mov    %r12,%rsi
    711e:	4c 89 ef             	mov    %r13,%rdi
    7121:	49 8b 54 24 3c       	mov    0x3c(%r12),%rdx
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
    7126:	48 89 42 08          	mov    %rax,0x8(%rdx)
	prev->next = next;
    712a:	48 89 10             	mov    %rdx,(%rax)
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
    712d:	48 b8 00 01 10 00 00 	movabs $0xdead000000100100,%rax
    7134:	00 ad de 
    7137:	49 89 44 24 3c       	mov    %rax,0x3c(%r12)
	entry->prev = LIST_POISON2;
    713c:	48 b8 00 02 20 00 00 	movabs $0xdead000000200200,%rax
    7143:	00 ad de 
    7146:	49 89 44 24 44       	mov    %rax,0x44(%r12)
    714b:	e8 00 00 00 00       	callq  7150 <__scif_put_pages+0x250>
    7150:	e9 76 fe ff ff       	jmpq   6fcb <__scif_put_pages+0xcb>
    7155:	0f 1f 00             	nopl   (%rax)
	struct endpt *ep;
	struct reg_range_t *window;
	struct nodemsg msg;

	if (!pages || !pages->cookie)
		return -EINVAL;
    7158:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    715d:	e9 65 ff ff ff       	jmpq   70c7 <__scif_put_pages+0x1c7>
    7162:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
	micscif_rma_list_gtt_unmap(window, window->offset, window->nr_pages);
#endif

	/* Decrement the ref counts and check for errors */
	window->get_put_ref_count -= pages->nr_pages;
	BUG_ON(window->get_put_ref_count < 0);
    7168:	0f 0b                	ud2    
    716a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    7170:	49 8b b6 88 01 00 00 	mov    0x188(%r14),%rsi
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    7177:	4d 8d 86 88 01 00 00 	lea    0x188(%r14),%r8
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    717e:	48 bf 00 00 00 00 00 	movabs $0x8000000000000000,%rdi
    7185:	00 00 80 
    7188:	48 39 fe             	cmp    %rdi,%rsi
    718b:	74 3e                	je     71cb <__scif_put_pages+0x2cb>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    718d:	48 8d 56 01          	lea    0x1(%rsi),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7191:	48 89 f0             	mov    %rsi,%rax
    7194:	f0 49 0f b1 96 88 01 	lock cmpxchg %rdx,0x188(%r14)
    719b:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    719d:	48 39 f0             	cmp    %rsi,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    71a0:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    71a3:	0f 84 d8 fd ff ff    	je     6f81 <__scif_put_pages+0x81>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    71a9:	48 39 fa             	cmp    %rdi,%rdx
    71ac:	74 1d                	je     71cb <__scif_put_pages+0x2cb>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    71ae:	48 8d 72 01          	lea    0x1(%rdx),%rsi
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    71b2:	48 89 d0             	mov    %rdx,%rax
    71b5:	f0 49 0f b1 30       	lock cmpxchg %rsi,(%r8)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    71ba:	48 39 d0             	cmp    %rdx,%rax
    71bd:	0f 84 be fd ff ff    	je     6f81 <__scif_put_pages+0x81>
    71c3:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    71c6:	48 39 fa             	cmp    %rdi,%rdx
    71c9:	75 e3                	jne    71ae <__scif_put_pages+0x2ae>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    71cb:	49 8d 86 68 01 00 00 	lea    0x168(%r14),%rax
    71d2:	48 89 c7             	mov    %rax,%rdi
    71d5:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    71dc:	e8 00 00 00 00       	callq  71e1 <__scif_put_pages+0x2e1>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    71e1:	41 8b 46 04          	mov    0x4(%r14),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    71e5:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    71e8:	83 fa 01             	cmp    $0x1,%edx
    71eb:	76 1e                	jbe    720b <__scif_put_pages+0x30b>
    71ed:	83 f8 01             	cmp    $0x1,%eax
    71f0:	74 19                	je     720b <__scif_put_pages+0x30b>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    71f2:	49 8b 86 88 01 00 00 	mov    0x188(%r14),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    71f9:	48 85 c0             	test   %rax,%rax
    71fc:	0f 88 ce 00 00 00    	js     72d0 <__scif_put_pages+0x3d0>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
    7202:	f0 49 83 86 88 01 00 	lock addq $0x1,0x188(%r14)
    7209:	00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    720b:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    7212:	e8 00 00 00 00       	callq  7217 <__scif_put_pages+0x317>
    7217:	e9 65 fd ff ff       	jmpq   6f81 <__scif_put_pages+0x81>
    721c:	0f 1f 40 00          	nopl   0x0(%rax)

static __always_inline void
put_window_ref_count(struct reg_range_t *window, int64_t nr_pages)
{
	window->ref_count -= (int)nr_pages; 
	BUG_ON(window->nr_pages < 0);
    7220:	0f 0b                	ud2    
static __always_inline void scif_free(void *addr, size_t size)
{
	size_t align = ALIGN(size, PAGE_SIZE);

	if (unlikely(is_vmalloc_addr(addr)))
		vfree(addr);
    7222:	e8 00 00 00 00       	callq  7227 <__scif_put_pages+0x327>
    7227:	e9 41 fe ff ff       	jmpq   706d <__scif_put_pages+0x16d>
    722c:	e8 00 00 00 00       	callq  7231 <__scif_put_pages+0x331>
    7231:	e9 87 fe ff ff       	jmpq   70bd <__scif_put_pages+0x1bd>
	 * callee should be allowed to release references to the pages,
	 * else the endpoint was not connected in the first place,
	 * hence the ENOTCONN.
	 */
	if (ep->state != SCIFEP_CONNECTED && ep->state != SCIFEP_DISCONNECTED)
		return -ENOTCONN;
    7236:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
    723b:	e9 87 fe ff ff       	jmpq   70c7 <__scif_put_pages+0x1c7>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    7240:	48 8b 45 08          	mov    0x8(%rbp),%rax
    7244:	4c 89 e1             	mov    %r12,%rcx
    7247:	ba a7 00 00 00       	mov    $0xa7,%edx
    724c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    7253:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    725a:	00 
    725b:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    7262:	45 0f b7 04 24       	movzwl (%r12),%r8d
    7267:	48 89 04 24          	mov    %rax,(%rsp)
    726b:	31 c0                	xor    %eax,%eax
    726d:	e8 00 00 00 00       	callq  7272 <__scif_put_pages+0x372>
    7272:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    7279:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    727a:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    7281:	00 00 80 
    7284:	48 39 f1             	cmp    %rsi,%rcx
    7287:	0f 84 75 fd ff ff    	je     7002 <__scif_put_pages+0x102>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    728d:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7291:	48 89 c8             	mov    %rcx,%rax
    7294:	f0 49 0f b1 16       	lock cmpxchg %rdx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7299:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    729c:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    729f:	0f 84 5d fd ff ff    	je     7002 <__scif_put_pages+0x102>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    72a5:	48 39 f2             	cmp    %rsi,%rdx
    72a8:	0f 84 54 fd ff ff    	je     7002 <__scif_put_pages+0x102>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    72ae:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    72b2:	48 89 d0             	mov    %rdx,%rax
    72b5:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    72ba:	48 39 d0             	cmp    %rdx,%rax
    72bd:	0f 84 3f fd ff ff    	je     7002 <__scif_put_pages+0x102>
    72c3:	48 89 c2             	mov    %rax,%rdx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    72c6:	48 39 f2             	cmp    %rsi,%rdx
    72c9:	75 e3                	jne    72ae <__scif_put_pages+0x3ae>
    72cb:	e9 32 fd ff ff       	jmpq   7002 <__scif_put_pages+0x102>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    72d0:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 72d6 <__scif_put_pages+0x3d6>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    72d6:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    72d8:	49 c7 86 b0 01 00 00 	movq   $0x2,0x1b0(%r14)
    72df:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    72e3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    72ea:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    72f1:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    72f5:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
    72f9:	31 c0                	xor    %eax,%eax
    72fb:	66 89 45 a8          	mov    %ax,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
    72ff:	41 0f b7 06          	movzwl (%r14),%eax
    7303:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    7307:	e8 00 00 00 00       	callq  730c <__scif_put_pages+0x40c>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    730c:	49 8b 86 b0 01 00 00 	mov    0x1b0(%r14),%rax
    7313:	48 83 f8 02          	cmp    $0x2,%rax
    7317:	74 20                	je     7339 <__scif_put_pages+0x439>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    7319:	48 83 f8 03          	cmp    $0x3,%rax
    731d:	0f 85 df fe ff ff    	jne    7202 <__scif_put_pages+0x302>
				dev->sd_state = SCIFDEV_RUNNING;
    7323:	41 c7 46 04 02 00 00 	movl   $0x2,0x4(%r14)
    732a:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    732b:	f0 41 80 a6 8f 01 00 	lock andb $0x7f,0x18f(%r14)
    7332:	00 7f 
    7334:	e9 c9 fe ff ff       	jmpq   7202 <__scif_put_pages+0x302>
    7339:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    7340:	00 00 
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    7342:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    7346:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    734d:	48 83 c0 18          	add    $0x18,%rax
    7351:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    7358:	00 
    7359:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    735d:	4d 8d be 98 01 00 00 	lea    0x198(%r14),%r15
    7364:	48 c7 85 78 ff ff ff 	movq   $0x0,-0x88(%rbp)
    736b:	00 00 00 00 
    736f:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    7373:	eb 05                	jmp    737a <__scif_put_pages+0x47a>
    7375:	e8 00 00 00 00       	callq  737a <__scif_put_pages+0x47a>
    737a:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    7381:	ba 02 00 00 00       	mov    $0x2,%edx
    7386:	4c 89 ff             	mov    %r15,%rdi
    7389:	e8 00 00 00 00       	callq  738e <__scif_put_pages+0x48e>
    738e:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
    7395:	02 
    7396:	74 dd                	je     7375 <__scif_put_pages+0x475>
    7398:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    739f:	4c 89 ff             	mov    %r15,%rdi
    73a2:	e8 00 00 00 00       	callq  73a7 <__scif_put_pages+0x4a7>
    73a7:	49 8b 86 b0 01 00 00 	mov    0x1b0(%r14),%rax
    73ae:	e9 66 ff ff ff       	jmpq   7319 <__scif_put_pages+0x419>
    73b3:	66 66 66 66 2e 0f 1f 	data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    73ba:	84 00 00 00 00 00 

00000000000073c0 <scif_put_pages>:
	return 0;
}

int
scif_put_pages(struct scif_range *pages)
{
    73c0:	55                   	push   %rbp
    73c1:	48 89 e5             	mov    %rsp,%rbp
    73c4:	41 54                	push   %r12
    73c6:	53                   	push   %rbx
    73c7:	e8 00 00 00 00       	callq  73cc <scif_put_pages+0xc>
	int ret;
	struct reg_range_t *window = pages->cookie;
	struct endpt *ep = (struct endpt *)window->ep;
    73cc:	48 8b 07             	mov    (%rdi),%rax
    73cf:	48 8b 58 34          	mov    0x34(%rax),%rbx
	return 0;
}

int
scif_put_pages(struct scif_range *pages)
{
    73d3:	49 89 fc             	mov    %rdi,%r12
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read(const atomic_t *v)
{
	return (*(volatile int *)&(v)->counter);
    73d6:	8b 83 70 01 00 00    	mov    0x170(%rbx),%eax
	int ret;
	struct reg_range_t *window = pages->cookie;
	struct endpt *ep = (struct endpt *)window->ep;
	if (atomic_read(&(&(ep->ref_count))->refcount) > 0) {
    73dc:	85 c0                	test   %eax,%eax
    73de:	7e 40                	jle    7420 <scif_put_pages+0x60>
		kref_get(&(ep->ref_count));
    73e0:	48 8d bb 70 01 00 00 	lea    0x170(%rbx),%rdi
    73e7:	e8 00 00 00 00       	callq  73ec <scif_put_pages+0x2c>
	} else {
		WARN_ON(1);
	}
	ret = __scif_put_pages(pages);
    73ec:	4c 89 e7             	mov    %r12,%rdi
    73ef:	e8 00 00 00 00       	callq  73f4 <scif_put_pages+0x34>
    73f4:	41 89 c4             	mov    %eax,%r12d
    73f7:	8b 83 70 01 00 00    	mov    0x170(%rbx),%eax
	if (atomic_read(&(&(ep->ref_count))->refcount) > 0) {
    73fd:	85 c0                	test   %eax,%eax
    73ff:	7e 13                	jle    7414 <scif_put_pages+0x54>
		kref_put(&(ep->ref_count), scif_ref_rel);
    7401:	48 8d bb 70 01 00 00 	lea    0x170(%rbx),%rdi
    7408:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    740f:	e8 00 00 00 00       	callq  7414 <scif_put_pages+0x54>
	} else {
		//WARN_ON(1);
	}
	return ret;
}
    7414:	44 89 e0             	mov    %r12d,%eax
    7417:	5b                   	pop    %rbx
    7418:	41 5c                	pop    %r12
    741a:	5d                   	pop    %rbp
    741b:	c3                   	retq   
    741c:	0f 1f 40 00          	nopl   0x0(%rax)
	struct reg_range_t *window = pages->cookie;
	struct endpt *ep = (struct endpt *)window->ep;
	if (atomic_read(&(&(ep->ref_count))->refcount) > 0) {
		kref_get(&(ep->ref_count));
	} else {
		WARN_ON(1);
    7420:	be 2a 09 00 00       	mov    $0x92a,%esi
    7425:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    742c:	e8 00 00 00 00       	callq  7431 <scif_put_pages+0x71>
    7431:	eb b9                	jmp    73ec <scif_put_pages+0x2c>
    7433:	66 66 66 66 2e 0f 1f 	data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    743a:	84 00 00 00 00 00 

0000000000007440 <__scif_register>:
 *	as documented in scif.h.
 */
off_t
__scif_register(scif_epd_t epd, void *addr, size_t len, off_t offset,
					int prot, int map_flags)
{
    7440:	55                   	push   %rbp
    7441:	48 89 e5             	mov    %rsp,%rbp
    7444:	e8 00 00 00 00       	callq  7449 <__scif_register+0x9>
	struct endpt *ep = (struct endpt *)epd;
	uint64_t computed_offset;
	struct reg_range_t *window;
	struct mm_struct *mm = NULL;

	pr_debug("SCIFAPI register: ep %p %s addr %p len 0x%lx"
    7449:	8b 07                	mov    (%rdi),%eax
		" offset 0x%lx prot 0x%x map_flags 0x%x\n", 
		epd, scif_ep_states[epd->state], addr, len, offset, prot, map_flags);

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_FIXED | SCIF_MAP_KERNEL))
    744b:	41 f7 c1 cf ff ff ff 	test   $0xffffffcf,%r9d
    7452:	75 6c                	jne    74c0 <__scif_register+0x80>
	/* Unsupported protection requested */
	if (prot & ~(SCIF_PROT_READ | SCIF_PROT_WRITE))
		return -EINVAL;

	/* addr/len must be page aligned. len should be non zero */
	if ((!len) ||
    7454:	48 85 d2             	test   %rdx,%rdx
    7457:	74 67                	je     74c0 <__scif_register+0x80>
    7459:	41 f7 c0 fc ff ff ff 	test   $0xfffffffc,%r8d
    7460:	75 5e                	jne    74c0 <__scif_register+0x80>
	wait_queue_head_t       wq;
};

static inline uint64_t align_low(uint64_t data, uint32_t granularity)
{
	return ALIGN(data - (granularity - 1), granularity);
    7462:	49 89 f2             	mov    %rsi,%r10
		" offset 0x%lx prot 0x%x map_flags 0x%x\n", 
		epd, scif_ep_states[epd->state], addr, len, offset, prot, map_flags);

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_FIXED | SCIF_MAP_KERNEL))
		return -EINVAL;
    7465:	48 c7 c0 ea ff ff ff 	mov    $0xffffffffffffffea,%rax
    746c:	49 81 e2 00 f0 ff ff 	and    $0xfffffffffffff000,%r10
	/* Unsupported protection requested */
	if (prot & ~(SCIF_PROT_READ | SCIF_PROT_WRITE))
		return -EINVAL;

	/* addr/len must be page aligned. len should be non zero */
	if ((!len) ||
    7473:	4c 39 d6             	cmp    %r10,%rsi
    7476:	75 3e                	jne    74b6 <__scif_register+0x76>
    7478:	49 89 d2             	mov    %rdx,%r10
    747b:	49 81 e2 00 f0 ff ff 	and    $0xfffffffffffff000,%r10
		(align_low((uint64_t)addr, PAGE_SIZE) != (uint64_t)addr) ||
    7482:	4c 39 d2             	cmp    %r10,%rdx
    7485:	75 2f                	jne    74b6 <__scif_register+0x76>

	/*
	 * Offset is not page aligned/negative or offset+len
	 * wraps around with SCIF_MAP_FIXED.
	 */
	if ((map_flags & SCIF_MAP_FIXED) &&
    7487:	41 f6 c1 10          	test   $0x10,%r9b
    748b:	74 24                	je     74b1 <__scif_register+0x71>
		((align_low(offset, PAGE_SIZE) != offset) ||
    748d:	49 89 ca             	mov    %rcx,%r10
    7490:	49 c1 ea 3f          	shr    $0x3f,%r10
    7494:	45 84 d2             	test   %r10b,%r10b
    7497:	75 27                	jne    74c0 <__scif_register+0x80>
    7499:	49 89 ca             	mov    %rcx,%r10
    749c:	49 81 e2 00 f0 ff ff 	and    $0xfffffffffffff000,%r10
    74a3:	4c 39 d1             	cmp    %r10,%rcx
    74a6:	75 18                	jne    74c0 <__scif_register+0x80>
		(offset < 0) ||
		(offset + (off_t)len < offset)))
    74a8:	4c 8d 14 11          	lea    (%rcx,%rdx,1),%r10
	 * Offset is not page aligned/negative or offset+len
	 * wraps around with SCIF_MAP_FIXED.
	 */
	if ((map_flags & SCIF_MAP_FIXED) &&
		((align_low(offset, PAGE_SIZE) != offset) ||
		(offset < 0) ||
    74ac:	4c 39 d1             	cmp    %r10,%rcx
    74af:	7f 05                	jg     74b6 <__scif_register+0x76>
    74b1:	e8 3a e3 ff ff       	callq  57f0 <__scif_register.part.11>
error_unmap:
	micscif_destroy_window(ep, window);
error:
	printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
	return err;
}
    74b6:	5d                   	pop    %rbp
    74b7:	c3                   	retq   
    74b8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    74bf:	00 
		" offset 0x%lx prot 0x%x map_flags 0x%x\n", 
		epd, scif_ep_states[epd->state], addr, len, offset, prot, map_flags);

	/* Unsupported flags */
	if (map_flags & ~(SCIF_MAP_FIXED | SCIF_MAP_KERNEL))
		return -EINVAL;
    74c0:	48 c7 c0 ea ff ff ff 	mov    $0xffffffffffffffea,%rax
error_unmap:
	micscif_destroy_window(ep, window);
error:
	printk(KERN_ERR "%s %d err %ld\n", __func__, __LINE__, err);
	return err;
}
    74c7:	5d                   	pop    %rbp
    74c8:	c3                   	retq   
    74c9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000074d0 <scif_register>:

off_t
scif_register(scif_epd_t epd, void *addr, size_t len, off_t offset,
					int prot, int map_flags)
{
    74d0:	55                   	push   %rbp
    74d1:	48 89 e5             	mov    %rsp,%rbp
    74d4:	41 57                	push   %r15
    74d6:	41 56                	push   %r14
    74d8:	41 55                	push   %r13
    74da:	41 54                	push   %r12
    74dc:	53                   	push   %rbx
    74dd:	48 83 ec 18          	sub    $0x18,%rsp
    74e1:	e8 00 00 00 00       	callq  74e6 <scif_register+0x16>
    74e6:	49 89 ff             	mov    %rdi,%r15
    74e9:	49 89 f4             	mov    %rsi,%r12
    74ec:	49 89 d5             	mov    %rdx,%r13
    74ef:	44 89 45 c8          	mov    %r8d,-0x38(%rbp)
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    74f3:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    74fa:	49 89 ce             	mov    %rcx,%r14
    74fd:	44 89 4d cc          	mov    %r9d,-0x34(%rbp)
    7501:	48 89 df             	mov    %rbx,%rdi
    7504:	e8 00 00 00 00       	callq  7509 <scif_register+0x39>
	off_t ret;
	get_kref_count(epd);
	ret = __scif_register(epd, addr, len, offset, prot, map_flags);
    7509:	44 8b 4d cc          	mov    -0x34(%rbp),%r9d
    750d:	4c 89 f1             	mov    %r14,%rcx
    7510:	4c 89 ea             	mov    %r13,%rdx
    7513:	44 8b 45 c8          	mov    -0x38(%rbp),%r8d
    7517:	4c 89 e6             	mov    %r12,%rsi
    751a:	4c 89 ff             	mov    %r15,%rdi
    751d:	e8 00 00 00 00       	callq  7522 <scif_register+0x52>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    7522:	48 89 df             	mov    %rbx,%rdi
    7525:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    752c:	49 89 c4             	mov    %rax,%r12
    752f:	e8 00 00 00 00       	callq  7534 <scif_register+0x64>
	put_kref_count(epd);
	return ret;
}
    7534:	48 83 c4 18          	add    $0x18,%rsp
    7538:	4c 89 e0             	mov    %r12,%rax
    753b:	5b                   	pop    %rbx
    753c:	41 5c                	pop    %r12
    753e:	41 5d                	pop    %r13
    7540:	41 5e                	pop    %r14
    7542:	41 5f                	pop    %r15
    7544:	5d                   	pop    %rbp
    7545:	c3                   	retq   
    7546:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    754d:	00 00 00 

0000000000007550 <__scif_unregister>:
 *	Upon successful completion, scif_unegister() returns zero
 *	else an apt error is returned as documented in scif.h.
 */
int
__scif_unregister(scif_epd_t epd, off_t offset, size_t len)
{
    7550:	55                   	push   %rbp
    7551:	48 89 e5             	mov    %rsp,%rbp
    7554:	41 57                	push   %r15
    7556:	41 56                	push   %r14
    7558:	41 55                	push   %r13
    755a:	41 54                	push   %r12
    755c:	53                   	push   %rbx
    755d:	48 81 ec b8 00 00 00 	sub    $0xb8,%rsp
    7564:	e8 00 00 00 00       	callq  7569 <__scif_unregister+0x19>
	struct endpt *ep = (struct endpt *)epd;
	struct reg_range_t *window = NULL;
	struct micscif_rma_req req;
	int nr_pages, err;

	pr_debug("SCIFAPI unregister: ep %p %s offset 0x%lx len 0x%lx\n", 
    7569:	8b 07                	mov    (%rdi),%eax
 */
int
__scif_unregister(scif_epd_t epd, off_t offset, size_t len)
{
	struct endpt *ep = (struct endpt *)epd;
	struct reg_range_t *window = NULL;
    756b:	48 c7 85 40 ff ff ff 	movq   $0x0,-0xc0(%rbp)
    7572:	00 00 00 00 

	pr_debug("SCIFAPI unregister: ep %p %s offset 0x%lx len 0x%lx\n", 
		ep, scif_ep_states[ep->state], offset, len);

	/* len must be page aligned. len should be non zero */
	if ((!len) ||
    7576:	48 85 d2             	test   %rdx,%rdx
    7579:	0f 84 d1 01 00 00    	je     7750 <__scif_unregister+0x200>
	wait_queue_head_t       wq;
};

static inline uint64_t align_low(uint64_t data, uint32_t granularity)
{
	return ALIGN(data - (granularity - 1), granularity);
    757f:	48 89 d0             	mov    %rdx,%rax
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;
    7582:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    7587:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax

	pr_debug("SCIFAPI unregister: ep %p %s offset 0x%lx len 0x%lx\n", 
		ep, scif_ep_states[ep->state], offset, len);

	/* len must be page aligned. len should be non zero */
	if ((!len) ||
    758d:	48 39 c2             	cmp    %rax,%rdx
    7590:	75 29                	jne    75bb <__scif_unregister+0x6b>
    7592:	48 89 f0             	mov    %rsi,%rax
    7595:	48 25 00 f0 ff ff    	and    $0xfffffffffffff000,%rax
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	/* Offset is not page aligned or offset+len wraps around */
	if ((align_low(offset, PAGE_SIZE) != offset) ||
    759b:	48 39 c6             	cmp    %rax,%rsi
    759e:	75 1b                	jne    75bb <__scif_unregister+0x6b>
		(offset + (off_t)len < offset))
    75a0:	48 8d 04 16          	lea    (%rsi,%rdx,1),%rax
	if ((!len) ||
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;

	/* Offset is not page aligned or offset+len wraps around */
	if ((align_low(offset, PAGE_SIZE) != offset) ||
    75a4:	48 39 c6             	cmp    %rax,%rsi
    75a7:	7f 12                	jg     75bb <__scif_unregister+0x6b>
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    75a9:	8b 07                	mov    (%rdi),%eax
		return -ECONNRESET;
    75ab:	b3 98                	mov    $0x98,%bl
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    75ad:	83 f8 09             	cmp    $0x9,%eax
    75b0:	74 09                	je     75bb <__scif_unregister+0x6b>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    75b2:	8b 07                	mov    (%rdi),%eax
		return -ENOTCONN;
    75b4:	b3 95                	mov    $0x95,%bl
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    75b6:	83 f8 04             	cmp    $0x4,%eax
    75b9:	74 15                	je     75d0 <__scif_unregister+0x80>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
	micscif_dec_node_refcnt(ep->remote_dev, 1);
	return err;
}
    75bb:	48 81 c4 b8 00 00 00 	add    $0xb8,%rsp
    75c2:	89 d8                	mov    %ebx,%eax
    75c4:	5b                   	pop    %rbx
    75c5:	41 5c                	pop    %r12
    75c7:	41 5d                	pop    %r13
    75c9:	41 5e                	pop    %r14
    75cb:	41 5f                	pop    %r15
    75cd:	5d                   	pop    %rbp
    75ce:	c3                   	retq   
    75cf:	90                   	nop
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    75d0:	48 8b 87 48 01 00 00 	mov    0x148(%rdi),%rax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    75d7:	b3 ed                	mov    $0xed,%bl
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    75d9:	8b 40 04             	mov    0x4(%rax),%eax
    75dc:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    75df:	83 f8 01             	cmp    $0x1,%eax
    75e2:	77 d7                	ja     75bb <__scif_unregister+0x6b>
    75e4:	83 bf 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rdi)
    75eb:	75 ce                	jne    75bb <__scif_unregister+0x6b>
    75ed:	49 89 fd             	mov    %rdi,%r13
    75f0:	48 89 d3             	mov    %rdx,%rbx
    75f3:	49 89 f4             	mov    %rsi,%r12
		return -EINVAL;

	if ((err = verify_epd(ep)))
		return err;

	might_sleep();
    75f6:	e8 00 00 00 00       	callq  75fb <__scif_unregister+0xab>
	req.prot = 0;
	req.nr_bytes = len;
	req.type = WINDOW_FULL;
	req.head = &ep->rma_info.reg_list;

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    75fb:	4d 8b b5 48 01 00 00 	mov    0x148(%r13),%r14

	if ((err = verify_epd(ep)))
		return err;

	might_sleep();
	nr_pages = (int)(len >> PAGE_SHIFT);
    7602:	49 89 df             	mov    %rbx,%r15

	req.out_window = &window;
	req.offset = offset;
    7605:	4c 89 65 a8          	mov    %r12,-0x58(%rbp)
		return err;

	might_sleep();
	nr_pages = (int)(len >> PAGE_SHIFT);

	req.out_window = &window;
    7609:	48 8d 85 40 ff ff ff 	lea    -0xc0(%rbp),%rax

	if ((err = verify_epd(ep)))
		return err;

	might_sleep();
	nr_pages = (int)(len >> PAGE_SHIFT);
    7610:	49 c1 ef 0c          	shr    $0xc,%r15

	req.out_window = &window;
	req.offset = offset;
	req.prot = 0;
	req.nr_bytes = len;
    7614:	48 89 5d b0          	mov    %rbx,-0x50(%rbp)
		return err;

	might_sleep();
	nr_pages = (int)(len >> PAGE_SHIFT);

	req.out_window = &window;
    7618:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
	req.offset = offset;
	req.prot = 0;
	req.nr_bytes = len;
	req.type = WINDOW_FULL;
	req.head = &ep->rma_info.reg_list;
    761c:	49 8d 45 28          	lea    0x28(%r13),%rax
	might_sleep();
	nr_pages = (int)(len >> PAGE_SHIFT);

	req.out_window = &window;
	req.offset = offset;
	req.prot = 0;
    7620:	c7 45 b8 00 00 00 00 	movl   $0x0,-0x48(%rbp)
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    7627:	4d 85 f6             	test   %r14,%r14
	req.nr_bytes = len;
	req.type = WINDOW_FULL;
    762a:	c7 45 bc 02 00 00 00 	movl   $0x2,-0x44(%rbp)
	req.head = &ep->rma_info.reg_list;
    7631:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    7635:	0f 85 1f 01 00 00    	jne    775a <__scif_unregister+0x20a>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);
    763b:	49 8d 9d 80 00 00 00 	lea    0x80(%r13),%rbx
    7642:	48 89 df             	mov    %rbx,%rdi
    7645:	e8 00 00 00 00       	callq  764a <__scif_unregister+0xfa>
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
    764a:	48 8d 7d a0          	lea    -0x60(%rbp),%rdi
    764e:	e8 00 00 00 00       	callq  7653 <__scif_unregister+0x103>
    7653:	85 c0                	test   %eax,%eax
    7655:	41 89 c6             	mov    %eax,%r14d
    7658:	0f 85 28 02 00 00    	jne    7886 <__scif_unregister+0x336>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	/* Unregister all the windows in this range */
	if ((err = micscif_rma_list_unregister(window, offset, nr_pages)))
    765e:	48 8b bd 40 ff ff ff 	mov    -0xc0(%rbp),%rdi
    7665:	44 89 fa             	mov    %r15d,%edx
    7668:	4c 89 e6             	mov    %r12,%rsi
    766b:	e8 00 00 00 00       	callq  7670 <__scif_unregister+0x120>
    7670:	85 c0                	test   %eax,%eax
    7672:	41 89 c6             	mov    %eax,%r14d
    7675:	0f 85 2c 02 00 00    	jne    78a7 <__scif_unregister+0x357>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    767b:	48 89 df             	mov    %rbx,%rdi
	micscif_dec_node_refcnt(ep->remote_dev, 1);
	return err;
    767e:	44 89 f3             	mov    %r14d,%ebx
	}
	/* Unregister all the windows in this range */
	if ((err = micscif_rma_list_unregister(window, offset, nr_pages)))
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    7681:	e8 00 00 00 00       	callq  7686 <__scif_unregister+0x136>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    7686:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    768d:	4d 85 e4             	test   %r12,%r12
    7690:	0f 84 25 ff ff ff    	je     75bb <__scif_unregister+0x6b>
		if (unlikely((atomic_long_sub_return(cnt, 
    7696:	4d 8d ac 24 88 01 00 	lea    0x188(%r12),%r13
    769d:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    769e:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    76a5:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    76ac:	01 00 00 
    76af:	48 83 e8 01          	sub    $0x1,%rax
    76b3:	0f 89 02 ff ff ff    	jns    75bb <__scif_unregister+0x6b>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    76b9:	48 8b 45 08          	mov    0x8(%rbp),%rax
    76bd:	4c 89 e1             	mov    %r12,%rcx
    76c0:	ba a7 00 00 00       	mov    $0xa7,%edx
    76c5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    76cc:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    76d3:	00 
    76d4:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    76db:	45 0f b7 04 24       	movzwl (%r12),%r8d
    76e0:	48 89 04 24          	mov    %rax,(%rsp)
    76e4:	31 c0                	xor    %eax,%eax
    76e6:	e8 00 00 00 00       	callq  76eb <__scif_unregister+0x19b>
    76eb:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    76f2:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    76f3:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    76fa:	00 00 80 
    76fd:	48 39 f1             	cmp    %rsi,%rcx
    7700:	0f 84 b5 fe ff ff    	je     75bb <__scif_unregister+0x6b>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7706:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    770a:	48 89 c8             	mov    %rcx,%rax
    770d:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    7714:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7717:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    771a:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    771d:	0f 84 98 fe ff ff    	je     75bb <__scif_unregister+0x6b>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7723:	48 39 f2             	cmp    %rsi,%rdx
    7726:	74 16                	je     773e <__scif_unregister+0x1ee>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7728:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    772c:	48 89 d0             	mov    %rdx,%rax
    772f:	f0 49 0f b1 4d 00    	lock cmpxchg %rcx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7735:	48 39 c2             	cmp    %rax,%rdx
    7738:	0f 85 40 01 00 00    	jne    787e <__scif_unregister+0x32e>
	return err;
    773e:	44 89 f3             	mov    %r14d,%ebx
    7741:	e9 75 fe ff ff       	jmpq   75bb <__scif_unregister+0x6b>
    7746:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    774d:	00 00 00 
		ep, scif_ep_states[ep->state], offset, len);

	/* len must be page aligned. len should be non zero */
	if ((!len) ||
		(align_low((uint64_t)len, PAGE_SIZE) != (uint64_t)len))
		return -EINVAL;
    7750:	bb ea ff ff ff       	mov    $0xffffffea,%ebx
    7755:	e9 61 fe ff ff       	jmpq   75bb <__scif_unregister+0x6b>
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    775a:	49 8b 8e 88 01 00 00 	mov    0x188(%r14),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    7761:	49 8d 9e 88 01 00 00 	lea    0x188(%r14),%rbx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7768:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    776f:	00 00 80 
    7772:	48 39 c1             	cmp    %rax,%rcx
    7775:	74 49                	je     77c0 <__scif_unregister+0x270>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7777:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    777b:	48 89 c8             	mov    %rcx,%rax
    777e:	f0 49 0f b1 96 88 01 	lock cmpxchg %rdx,0x188(%r14)
    7785:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7787:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    778a:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    778d:	0f 84 a8 fe ff ff    	je     763b <__scif_unregister+0xeb>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7793:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    779a:	00 00 80 
    779d:	48 39 f2             	cmp    %rsi,%rdx
    77a0:	74 1e                	je     77c0 <__scif_unregister+0x270>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    77a2:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    77a6:	48 89 d0             	mov    %rdx,%rax
    77a9:	f0 48 0f b1 0b       	lock cmpxchg %rcx,(%rbx)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    77ae:	48 39 d0             	cmp    %rdx,%rax
    77b1:	0f 84 84 fe ff ff    	je     763b <__scif_unregister+0xeb>
    77b7:	48 89 c2             	mov    %rax,%rdx
    77ba:	eb e1                	jmp    779d <__scif_unregister+0x24d>
    77bc:	0f 1f 40 00          	nopl   0x0(%rax)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    77c0:	49 8d 86 68 01 00 00 	lea    0x168(%r14),%rax
    77c7:	48 89 c7             	mov    %rax,%rdi
    77ca:	48 89 85 38 ff ff ff 	mov    %rax,-0xc8(%rbp)
    77d1:	e8 00 00 00 00       	callq  77d6 <__scif_unregister+0x286>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    77d6:	41 8b 46 04          	mov    0x4(%r14),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    77da:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    77dd:	83 fa 01             	cmp    $0x1,%edx
    77e0:	76 19                	jbe    77fb <__scif_unregister+0x2ab>
    77e2:	83 f8 01             	cmp    $0x1,%eax
    77e5:	74 14                	je     77fb <__scif_unregister+0x2ab>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    77e7:	49 8b 86 88 01 00 00 	mov    0x188(%r14),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    77ee:	48 85 c0             	test   %rax,%rax
    77f1:	78 19                	js     780c <__scif_unregister+0x2bc>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    77f3:	48 89 df             	mov    %rbx,%rdi
    77f6:	e8 f5 8c ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    77fb:	48 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%rdi
    7802:	e8 00 00 00 00       	callq  7807 <__scif_unregister+0x2b7>
    7807:	e9 2f fe ff ff       	jmpq   763b <__scif_unregister+0xeb>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    780c:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 7812 <__scif_unregister+0x2c2>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    7812:	31 d2                	xor    %edx,%edx
    7814:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    781b:	c7 85 7c ff ff ff 2e 	movl   $0x2e,-0x84(%rbp)
    7822:	00 00 00 
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    7825:	66 c7 85 78 ff ff ff 	movw   $0x0,-0x88(%rbp)
    782c:	00 00 
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    782e:	48 8d b5 74 ff ff ff 	lea    -0x8c(%rbp),%rsi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    7835:	49 c7 86 b0 01 00 00 	movq   $0x2,0x1b0(%r14)
    783c:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    7840:	66 89 85 74 ff ff ff 	mov    %ax,-0x8c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    7847:	41 0f b7 06          	movzwl (%r14),%eax
    784b:	48 89 45 80          	mov    %rax,-0x80(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    784f:	e8 00 00 00 00       	callq  7854 <__scif_unregister+0x304>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    7854:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
    785b:	02 
    785c:	74 6a                	je     78c8 <__scif_unregister+0x378>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    785e:	49 83 be b0 01 00 00 	cmpq   $0x3,0x1b0(%r14)
    7865:	03 
    7866:	75 8b                	jne    77f3 <__scif_unregister+0x2a3>
				dev->sd_state = SCIFDEV_RUNNING;
    7868:	41 c7 46 04 02 00 00 	movl   $0x2,0x4(%r14)
    786f:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    7870:	f0 41 80 a6 8f 01 00 	lock andb $0x7f,0x18f(%r14)
    7877:	00 7f 
    7879:	e9 75 ff ff ff       	jmpq   77f3 <__scif_unregister+0x2a3>
    787e:	48 89 c2             	mov    %rax,%rdx
    7881:	e9 9d fe ff ff       	jmpq   7723 <__scif_unregister+0x1d3>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    7886:	89 c1                	mov    %eax,%ecx
    7888:	ba 22 0a 00 00       	mov    $0xa22,%edx
    788d:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    7894:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    789b:	31 c0                	xor    %eax,%eax
    789d:	e8 00 00 00 00       	callq  78a2 <__scif_unregister+0x352>
		goto error;
    78a2:	e9 d4 fd ff ff       	jmpq   767b <__scif_unregister+0x12b>
	}
	/* Unregister all the windows in this range */
	if ((err = micscif_rma_list_unregister(window, offset, nr_pages)))
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    78a7:	89 c1                	mov    %eax,%ecx
    78a9:	ba 27 0a 00 00       	mov    $0xa27,%edx
    78ae:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    78b5:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    78bc:	31 c0                	xor    %eax,%eax
    78be:	e8 00 00 00 00       	callq  78c3 <__scif_unregister+0x373>
    78c3:	e9 b3 fd ff ff       	jmpq   767b <__scif_unregister+0x12b>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    78c8:	48 8d bd 48 ff ff ff 	lea    -0xb8(%rbp),%rdi
    78cf:	31 c0                	xor    %eax,%eax
    78d1:	b9 0a 00 00 00       	mov    $0xa,%ecx
    78d6:	f3 ab                	rep stos %eax,%es:(%rdi)
    78d8:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
    78df:	00 00 00 00 
    78e3:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    78ea:	00 00 
    78ec:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    78f3:	48 8d 85 48 ff ff ff 	lea    -0xb8(%rbp),%rax
    78fa:	48 83 c0 18          	add    $0x18,%rax
    78fe:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    7905:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    790c:	49 8d 86 98 01 00 00 	lea    0x198(%r14),%rax
    7913:	48 89 85 30 ff ff ff 	mov    %rax,-0xd0(%rbp)
    791a:	eb 09                	jmp    7925 <__scif_unregister+0x3d5>
    791c:	0f 1f 40 00          	nopl   0x0(%rax)
    7920:	e8 00 00 00 00       	callq  7925 <__scif_unregister+0x3d5>
    7925:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    792c:	ba 02 00 00 00       	mov    $0x2,%edx
    7931:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    7938:	e8 00 00 00 00       	callq  793d <__scif_unregister+0x3ed>
    793d:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
    7944:	02 
    7945:	74 d9                	je     7920 <__scif_unregister+0x3d0>
    7947:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    794e:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    7955:	e8 00 00 00 00       	callq  795a <__scif_unregister+0x40a>
    795a:	e9 ff fe ff ff       	jmpq   785e <__scif_unregister+0x30e>
    795f:	90                   	nop

0000000000007960 <scif_unregister>:
	return err;
}

int
scif_unregister(scif_epd_t epd, off_t offset, size_t len)
{
    7960:	55                   	push   %rbp
    7961:	48 89 e5             	mov    %rsp,%rbp
    7964:	41 56                	push   %r14
    7966:	41 55                	push   %r13
    7968:	41 54                	push   %r12
    796a:	53                   	push   %rbx
    796b:	e8 00 00 00 00       	callq  7970 <scif_unregister+0x10>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    7970:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    7977:	49 89 fc             	mov    %rdi,%r12
    797a:	49 89 f5             	mov    %rsi,%r13
    797d:	49 89 d6             	mov    %rdx,%r14
    7980:	48 89 df             	mov    %rbx,%rdi
    7983:	e8 00 00 00 00       	callq  7988 <scif_unregister+0x28>
	int ret;
	get_kref_count(epd);
	ret = __scif_unregister(epd, offset, len);
    7988:	4c 89 f2             	mov    %r14,%rdx
    798b:	4c 89 ee             	mov    %r13,%rsi
    798e:	4c 89 e7             	mov    %r12,%rdi
    7991:	e8 00 00 00 00       	callq  7996 <scif_unregister+0x36>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    7996:	48 89 df             	mov    %rbx,%rdi
    7999:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    79a0:	41 89 c4             	mov    %eax,%r12d
    79a3:	e8 00 00 00 00       	callq  79a8 <scif_unregister+0x48>
	put_kref_count(epd);
	return ret;
}
    79a8:	5b                   	pop    %rbx
    79a9:	44 89 e0             	mov    %r12d,%eax
    79ac:	41 5c                	pop    %r12
    79ae:	41 5d                	pop    %r13
    79b0:	41 5e                	pop    %r14
    79b2:	5d                   	pop    %rbp
    79b3:	c3                   	retq   
    79b4:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
    79bb:	00 00 00 00 00 

00000000000079c0 <__scif_pollfd>:
	put_kref_count(epd);
	return ret;
}

unsigned int __scif_pollfd(struct file *f, poll_table *wait, struct endpt *ep)
{
    79c0:	55                   	push   %rbp
    79c1:	48 89 e5             	mov    %rsp,%rbp
    79c4:	41 57                	push   %r15
    79c6:	41 56                	push   %r14
    79c8:	41 55                	push   %r13
    79ca:	41 54                	push   %r12
    79cc:	53                   	push   %rbx
    79cd:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    79d4:	e8 00 00 00 00       	callq  79d9 <__scif_pollfd+0x19>
	unsigned int mask = 0;
	unsigned long sflags;

	pr_debug("SCIFAPI pollfd: ep %p %s\n", ep, scif_ep_states[ep->state]);

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    79d9:	4c 8b aa 48 01 00 00 	mov    0x148(%rdx),%r13
unsigned int __scif_pollfd(struct file *f, poll_table *wait, struct endpt *ep)
{
	unsigned int mask = 0;
	unsigned long sflags;

	pr_debug("SCIFAPI pollfd: ep %p %s\n", ep, scif_ep_states[ep->state]);
    79e0:	8b 02                	mov    (%rdx),%eax
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    79e2:	4d 85 ed             	test   %r13,%r13
	put_kref_count(epd);
	return ret;
}

unsigned int __scif_pollfd(struct file *f, poll_table *wait, struct endpt *ep)
{
    79e5:	49 89 ff             	mov    %rdi,%r15
    79e8:	49 89 f6             	mov    %rsi,%r14
    79eb:	48 89 d3             	mov    %rdx,%rbx
    79ee:	0f 85 7c 02 00 00    	jne    7c70 <__scif_pollfd+0x2b0>
 * Map the spin_lock functions to the raw variants for PREEMPT_RT=n
 */

static inline raw_spinlock_t *spinlock_check(spinlock_t *lock)
{
	return &lock->rlock;
    79f4:	4c 8d 6b 04          	lea    0x4(%rbx),%r13
	unsigned long sflags;

	pr_debug("SCIFAPI pollfd: ep %p %s\n", ep, scif_ep_states[ep->state]);

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);
    79f8:	4c 89 ef             	mov    %r13,%rdi
    79fb:	e8 00 00 00 00       	callq  7a00 <__scif_pollfd+0x40>
    7a00:	48 89 c2             	mov    %rax,%rdx

	/* Is it OK to use wait->key?? */
	if (ep->state == SCIFEP_LISTENING) {
    7a03:	8b 03                	mov    (%rbx),%eax
    7a05:	83 f8 03             	cmp    $0x3,%eax
    7a08:	0f 84 32 01 00 00    	je     7b40 <__scif_pollfd+0x180>
			mask |= SCIF_POLLERR;
		}
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
    7a0e:	4d 85 f6             	test   %r14,%r14
    7a11:	0f 84 81 01 00 00    	je     7b98 <__scif_pollfd+0x1d8>
    7a17:	49 8b 46 08          	mov    0x8(%r14),%rax
    7a1b:	a8 01                	test   $0x1,%al
    7a1d:	0f 85 7d 00 00 00    	jne    7aa0 <__scif_pollfd+0xe0>
	return ret;
}

unsigned int __scif_pollfd(struct file *f, poll_table *wait, struct endpt *ep)
{
	unsigned int mask = 0;
    7a23:	45 31 e4             	xor    %r12d,%r12d
		spin_lock_irqsave(&ep->lock, sflags);
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
    7a26:	a8 04                	test   $0x4,%al
    7a28:	74 1e                	je     7a48 <__scif_pollfd+0x88>
		if (ep->state != SCIFEP_CONNECTED &&
    7a2a:	8b 03                	mov    (%rbx),%eax
    7a2c:	83 f8 04             	cmp    $0x4,%eax
    7a2f:	0f 84 13 02 00 00    	je     7c48 <__scif_pollfd+0x288>
		    ep->state != SCIFEP_LISTENING) {
    7a35:	8b 03                	mov    (%rbx),%eax
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
		if (ep->state != SCIFEP_CONNECTED &&
    7a37:	83 f8 03             	cmp    $0x3,%eax
    7a3a:	0f 84 08 02 00 00    	je     7c48 <__scif_pollfd+0x288>
		    ep->state != SCIFEP_LISTENING) {
			mask |= SCIF_POLLERR;
    7a40:	41 83 cc 08          	or     $0x8,%r12d
    7a44:	0f 1f 40 00          	nopl   0x0(%rax)
			mask |= SCIF_POLLOUT;
	}

return_scif_poll:
	/* If the endpoint is in the diconnected state then return hangup instead of error */
	if (ep->state == SCIFEP_DISCONNECTED) {
    7a48:	8b 03                	mov    (%rbx),%eax
    7a4a:	83 f8 09             	cmp    $0x9,%eax
    7a4d:	0f 84 dd 00 00 00    	je     7b30 <__scif_pollfd+0x170>
	raw_spin_unlock_irq(&lock->rlock);
}

static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
	raw_spin_unlock_irqrestore(&lock->rlock, flags);
    7a53:	48 89 d6             	mov    %rdx,%rsi
    7a56:	4c 89 ef             	mov    %r13,%rdi
    7a59:	e8 00 00 00 00       	callq  7a5e <__scif_pollfd+0x9e>
		mask &= ~SCIF_POLLERR;
		mask |= SCIF_POLLHUP;
	}

	spin_unlock_irqrestore(&ep->lock, sflags);
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    7a5e:	48 8b 9b 48 01 00 00 	mov    0x148(%rbx),%rbx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    7a65:	48 85 db             	test   %rbx,%rbx
    7a68:	74 21                	je     7a8b <__scif_pollfd+0xcb>
		if (unlikely((atomic_long_sub_return(cnt, 
    7a6a:	4c 8d ab 88 01 00 00 	lea    0x188(%rbx),%r13
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    7a71:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    7a78:	f0 48 0f c1 83 88 01 	lock xadd %rax,0x188(%rbx)
    7a7f:	00 00 
    7a81:	48 83 e8 01          	sub    $0x1,%rax
    7a85:	0f 88 45 02 00 00    	js     7cd0 <__scif_pollfd+0x310>
	return mask;
}
    7a8b:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    7a92:	44 89 e0             	mov    %r12d,%eax
    7a95:	5b                   	pop    %rbx
    7a96:	41 5c                	pop    %r12
    7a98:	41 5d                	pop    %r13
    7a9a:	41 5e                	pop    %r14
    7a9c:	41 5f                	pop    %r15
    7a9e:	5d                   	pop    %rbp
    7a9f:	c3                   	retq   
		}
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
		if (ep->state != SCIFEP_CONNECTED &&
    7aa0:	8b 03                	mov    (%rbx),%eax
    7aa2:	83 f8 04             	cmp    $0x4,%eax
    7aa5:	0f 84 b5 02 00 00    	je     7d60 <__scif_pollfd+0x3a0>
		    ep->state != SCIFEP_LISTENING &&
    7aab:	8b 03                	mov    (%rbx),%eax
		}
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
		if (ep->state != SCIFEP_CONNECTED &&
    7aad:	83 f8 03             	cmp    $0x3,%eax
    7ab0:	74 0d                	je     7abf <__scif_pollfd+0xff>
		    ep->state != SCIFEP_LISTENING &&
		    ep->state != SCIFEP_DISCONNECTED) {
    7ab2:	8b 03                	mov    (%rbx),%eax
			mask |= SCIF_POLLERR;
    7ab4:	41 bc 08 00 00 00    	mov    $0x8,%r12d
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
		if (ep->state != SCIFEP_CONNECTED &&
		    ep->state != SCIFEP_LISTENING &&
    7aba:	83 f8 09             	cmp    $0x9,%eax
    7abd:	75 89                	jne    7a48 <__scif_pollfd+0x88>
    7abf:	48 89 d6             	mov    %rdx,%rsi
    7ac2:	4c 89 ef             	mov    %r13,%rdi
    7ac5:	e8 00 00 00 00       	callq  7aca <__scif_pollfd+0x10a>
			mask |= SCIF_POLLERR;
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
    7aca:	48 8d b3 e8 01 00 00 	lea    0x1e8(%rbx),%rsi
	unsigned long key;
} poll_table;

static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{
	if (p && wait_address)
    7ad1:	4d 85 f6             	test   %r14,%r14
    7ad4:	74 0e                	je     7ae4 <__scif_pollfd+0x124>
    7ad6:	48 85 f6             	test   %rsi,%rsi
    7ad9:	74 09                	je     7ae4 <__scif_pollfd+0x124>
		p->qproc(filp, wait_address, p);
    7adb:	4c 89 f2             	mov    %r14,%rdx
    7ade:	4c 89 ff             	mov    %r15,%rdi
    7ae1:	41 ff 16             	callq  *(%r14)
		spin_lock_irqsave(&ep->lock, sflags);
    7ae4:	4c 89 ef             	mov    %r13,%rdi
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7ae7:	45 31 e4             	xor    %r12d,%r12d
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7aea:	e8 00 00 00 00       	callq  7aef <__scif_pollfd+0x12f>
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7aef:	be 01 00 00 00       	mov    $0x1,%esi
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7af4:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7afb:	48 8b 43 10          	mov    0x10(%rbx),%rax
    7aff:	48 8d 78 48          	lea    0x48(%rax),%rdi
    7b03:	e8 00 00 00 00       	callq  7b08 <__scif_pollfd+0x148>
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
    7b08:	48 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%rdx
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7b0f:	85 c0                	test   %eax,%eax
    7b11:	41 0f 95 c4          	setne  %r12b
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
    7b15:	4d 85 f6             	test   %r14,%r14
    7b18:	0f 84 d2 00 00 00    	je     7bf0 <__scif_pollfd+0x230>
    7b1e:	49 8b 46 08          	mov    0x8(%r14),%rax
    7b22:	e9 ff fe ff ff       	jmpq   7a26 <__scif_pollfd+0x66>
    7b27:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    7b2e:	00 00 
	}

return_scif_poll:
	/* If the endpoint is in the diconnected state then return hangup instead of error */
	if (ep->state == SCIFEP_DISCONNECTED) {
		mask &= ~SCIF_POLLERR;
    7b30:	41 83 e4 f7          	and    $0xfffffff7,%r12d
		mask |= SCIF_POLLHUP;
    7b34:	41 83 cc 10          	or     $0x10,%r12d
    7b38:	e9 16 ff ff ff       	jmpq   7a53 <__scif_pollfd+0x93>
    7b3d:	0f 1f 00             	nopl   (%rax)
	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);

	/* Is it OK to use wait->key?? */
	if (ep->state == SCIFEP_LISTENING) {
		if (!wait || wait->key & SCIF_POLLIN) {
    7b40:	4d 85 f6             	test   %r14,%r14
    7b43:	0f 84 87 02 00 00    	je     7dd0 <__scif_pollfd+0x410>
    7b49:	41 f6 46 08 01       	testb  $0x1,0x8(%r14)
			poll_wait(f, &ep->conwq, wait);
			spin_lock_irqsave(&ep->lock, sflags);
			if (ep->conreqcnt)
				mask |= SCIF_POLLIN;
		} else {
			mask |= SCIF_POLLERR;
    7b4e:	41 bc 08 00 00 00    	mov    $0x8,%r12d
	micscif_inc_node_refcnt(ep->remote_dev, 1);
	spin_lock_irqsave(&ep->lock, sflags);

	/* Is it OK to use wait->key?? */
	if (ep->state == SCIFEP_LISTENING) {
		if (!wait || wait->key & SCIF_POLLIN) {
    7b54:	0f 84 ee fe ff ff    	je     7a48 <__scif_pollfd+0x88>
    7b5a:	48 89 d6             	mov    %rdx,%rsi
    7b5d:	4c 89 ef             	mov    %r13,%rdi
    7b60:	e8 00 00 00 00       	callq  7b65 <__scif_pollfd+0x1a5>
	unsigned long key;
} poll_table;

static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{
	if (p && wait_address)
    7b65:	48 89 de             	mov    %rbx,%rsi
    7b68:	48 81 c6 88 01 00 00 	add    $0x188,%rsi
    7b6f:	0f 85 43 02 00 00    	jne    7db8 <__scif_pollfd+0x3f8>
			spin_unlock_irqrestore(&ep->lock, sflags);
			poll_wait(f, &ep->conwq, wait);
			spin_lock_irqsave(&ep->lock, sflags);
    7b75:	4c 89 ef             	mov    %r13,%rdi
			if (ep->conreqcnt)
    7b78:	45 31 e4             	xor    %r12d,%r12d
	/* Is it OK to use wait->key?? */
	if (ep->state == SCIFEP_LISTENING) {
		if (!wait || wait->key & SCIF_POLLIN) {
			spin_unlock_irqrestore(&ep->lock, sflags);
			poll_wait(f, &ep->conwq, wait);
			spin_lock_irqsave(&ep->lock, sflags);
    7b7b:	e8 00 00 00 00       	callq  7b80 <__scif_pollfd+0x1c0>
    7b80:	48 89 c2             	mov    %rax,%rdx
			if (ep->conreqcnt)
    7b83:	8b 83 58 01 00 00    	mov    0x158(%rbx),%eax
    7b89:	85 c0                	test   %eax,%eax
    7b8b:	41 0f 95 c4          	setne  %r12b
    7b8f:	e9 b4 fe ff ff       	jmpq   7a48 <__scif_pollfd+0x88>
    7b94:	0f 1f 40 00          	nopl   0x0(%rax)
		}
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
		if (ep->state != SCIFEP_CONNECTED &&
    7b98:	8b 03                	mov    (%rbx),%eax
    7b9a:	83 f8 04             	cmp    $0x4,%eax
    7b9d:	74 0b                	je     7baa <__scif_pollfd+0x1ea>
		    ep->state != SCIFEP_LISTENING &&
    7b9f:	8b 03                	mov    (%rbx),%eax
		}
		goto return_scif_poll;
	}

	if (!wait || wait->key & SCIF_POLLIN) {
		if (ep->state != SCIFEP_CONNECTED &&
    7ba1:	83 f8 03             	cmp    $0x3,%eax
    7ba4:	0f 85 08 ff ff ff    	jne    7ab2 <__scif_pollfd+0xf2>
    7baa:	48 89 d6             	mov    %rdx,%rsi
    7bad:	4c 89 ef             	mov    %r13,%rdi
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7bb0:	45 31 e4             	xor    %r12d,%r12d
    7bb3:	e8 00 00 00 00       	callq  7bb8 <__scif_pollfd+0x1f8>
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7bb8:	4c 89 ef             	mov    %r13,%rdi
    7bbb:	e8 00 00 00 00       	callq  7bc0 <__scif_pollfd+0x200>
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7bc0:	be 01 00 00 00       	mov    $0x1,%esi
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7bc5:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7bcc:	48 8b 43 10          	mov    0x10(%rbx),%rax
    7bd0:	48 8d 78 48          	lea    0x48(%rax),%rdi
    7bd4:	e8 00 00 00 00       	callq  7bd9 <__scif_pollfd+0x219>
    7bd9:	48 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%rdx
    7be0:	85 c0                	test   %eax,%eax
    7be2:	41 0f 95 c4          	setne  %r12b
    7be6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    7bed:	00 00 00 
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
		if (ep->state != SCIFEP_CONNECTED &&
    7bf0:	8b 03                	mov    (%rbx),%eax
    7bf2:	83 f8 04             	cmp    $0x4,%eax
    7bf5:	74 0b                	je     7c02 <__scif_pollfd+0x242>
		    ep->state != SCIFEP_LISTENING) {
    7bf7:	8b 03                	mov    (%rbx),%eax
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
			mask |= SCIF_POLLIN;
	}

	if (!wait || wait->key & SCIF_POLLOUT) {
		if (ep->state != SCIFEP_CONNECTED &&
    7bf9:	83 f8 03             	cmp    $0x3,%eax
    7bfc:	0f 85 3e fe ff ff    	jne    7a40 <__scif_pollfd+0x80>
    7c02:	48 89 d6             	mov    %rdx,%rsi
    7c05:	4c 89 ef             	mov    %r13,%rdi
    7c08:	e8 00 00 00 00       	callq  7c0d <__scif_pollfd+0x24d>
    7c0d:	0f 1f 00             	nopl   (%rax)
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->sendwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7c10:	4c 89 ef             	mov    %r13,%rdi
    7c13:	e8 00 00 00 00       	callq  7c18 <__scif_pollfd+0x258>
    7c18:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
		if (micscif_rb_space(&ep->qp_info.qp->outbound_q))
    7c1f:	48 8b 43 10          	mov    0x10(%rbx),%rax
    7c23:	48 8d 78 18          	lea    0x18(%rax),%rdi
    7c27:	e8 00 00 00 00       	callq  7c2c <__scif_pollfd+0x26c>
			mask |= SCIF_POLLOUT;
    7c2c:	44 89 e1             	mov    %r12d,%ecx
    7c2f:	48 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%rdx
    7c36:	83 c9 04             	or     $0x4,%ecx
    7c39:	85 c0                	test   %eax,%eax
    7c3b:	44 0f 45 e1          	cmovne %ecx,%r12d
    7c3f:	e9 04 fe ff ff       	jmpq   7a48 <__scif_pollfd+0x88>
    7c44:	0f 1f 40 00          	nopl   0x0(%rax)
    7c48:	48 89 d6             	mov    %rdx,%rsi
    7c4b:	4c 89 ef             	mov    %r13,%rdi
    7c4e:	e8 00 00 00 00       	callq  7c53 <__scif_pollfd+0x293>
			mask |= SCIF_POLLERR;
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->sendwq, wait);
    7c53:	48 8d b3 d0 01 00 00 	lea    0x1d0(%rbx),%rsi
    7c5a:	4d 85 f6             	test   %r14,%r14
    7c5d:	74 b1                	je     7c10 <__scif_pollfd+0x250>
    7c5f:	48 85 f6             	test   %rsi,%rsi
    7c62:	74 ac                	je     7c10 <__scif_pollfd+0x250>
		p->qproc(filp, wait_address, p);
    7c64:	4c 89 f2             	mov    %r14,%rdx
    7c67:	4c 89 ff             	mov    %r15,%rdi
    7c6a:	41 ff 16             	callq  *(%r14)
    7c6d:	eb a1                	jmp    7c10 <__scif_pollfd+0x250>
    7c6f:	90                   	nop
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    7c70:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    7c77:	49 8d bd 88 01 00 00 	lea    0x188(%r13),%rdi
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7c7e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    7c85:	00 00 80 
    7c88:	48 39 f1             	cmp    %rsi,%rcx
    7c8b:	0f 84 4f 01 00 00    	je     7de0 <__scif_pollfd+0x420>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7c91:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7c95:	48 89 c8             	mov    %rcx,%rax
    7c98:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    7c9f:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7ca1:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7ca4:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7ca7:	0f 84 47 fd ff ff    	je     79f4 <__scif_pollfd+0x34>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7cad:	48 39 f2             	cmp    %rsi,%rdx
    7cb0:	0f 84 2a 01 00 00    	je     7de0 <__scif_pollfd+0x420>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7cb6:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7cba:	48 89 d0             	mov    %rdx,%rax
    7cbd:	f0 48 0f b1 0f       	lock cmpxchg %rcx,(%rdi)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7cc2:	48 39 c2             	cmp    %rax,%rdx
    7cc5:	0f 84 29 fd ff ff    	je     79f4 <__scif_pollfd+0x34>
    7ccb:	48 89 c2             	mov    %rax,%rdx
    7cce:	eb dd                	jmp    7cad <__scif_pollfd+0x2ed>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    7cd0:	48 8b 45 08          	mov    0x8(%rbp),%rax
    7cd4:	48 89 d9             	mov    %rbx,%rcx
    7cd7:	ba a7 00 00 00       	mov    $0xa7,%edx
    7cdc:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    7ce3:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    7cea:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    7cf1:	44 0f b7 03          	movzwl (%rbx),%r8d
    7cf5:	48 89 04 24          	mov    %rax,(%rsp)
    7cf9:	31 c0                	xor    %eax,%eax
    7cfb:	e8 00 00 00 00       	callq  7d00 <__scif_pollfd+0x340>
    7d00:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7d07:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    7d0e:	00 00 80 
    7d11:	48 39 f1             	cmp    %rsi,%rcx
    7d14:	0f 84 71 fd ff ff    	je     7a8b <__scif_pollfd+0xcb>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7d1a:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7d1e:	48 89 c8             	mov    %rcx,%rax
    7d21:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    7d28:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7d2a:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7d2d:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7d30:	0f 84 55 fd ff ff    	je     7a8b <__scif_pollfd+0xcb>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    7d36:	48 39 f2             	cmp    %rsi,%rdx
    7d39:	0f 84 4c fd ff ff    	je     7a8b <__scif_pollfd+0xcb>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    7d3f:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    7d43:	48 89 d0             	mov    %rdx,%rax
    7d46:	f0 49 0f b1 4d 00    	lock cmpxchg %rcx,0x0(%r13)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    7d4c:	48 39 c2             	cmp    %rax,%rdx
    7d4f:	0f 84 36 fd ff ff    	je     7a8b <__scif_pollfd+0xcb>
    7d55:	48 89 c2             	mov    %rax,%rdx
    7d58:	eb dc                	jmp    7d36 <__scif_pollfd+0x376>
    7d5a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    7d60:	48 89 d6             	mov    %rdx,%rsi
    7d63:	4c 89 ef             	mov    %r13,%rdi
    7d66:	e8 00 00 00 00       	callq  7d6b <__scif_pollfd+0x3ab>
	unsigned long key;
} poll_table;

static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{
	if (p && wait_address)
    7d6b:	48 89 de             	mov    %rbx,%rsi
    7d6e:	48 81 c6 e8 01 00 00 	add    $0x1e8,%rsi
    7d75:	0f 85 60 fd ff ff    	jne    7adb <__scif_pollfd+0x11b>
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7d7b:	4c 89 ef             	mov    %r13,%rdi
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7d7e:	45 31 e4             	xor    %r12d,%r12d
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7d81:	e8 00 00 00 00       	callq  7d86 <__scif_pollfd+0x3c6>
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7d86:	be 01 00 00 00       	mov    $0x1,%esi
			goto return_scif_poll;
		}

		spin_unlock_irqrestore(&ep->lock, sflags);
		poll_wait(f, &ep->recvwq, wait);
		spin_lock_irqsave(&ep->lock, sflags);
    7d8b:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
		if (micscif_rb_count(&ep->qp_info.qp->inbound_q, 1))
    7d92:	48 8b 43 10          	mov    0x10(%rbx),%rax
    7d96:	48 8d 78 48          	lea    0x48(%rax),%rdi
    7d9a:	e8 00 00 00 00       	callq  7d9f <__scif_pollfd+0x3df>
    7d9f:	48 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%rdx
    7da6:	85 c0                	test   %eax,%eax
    7da8:	41 0f 95 c4          	setne  %r12b
    7dac:	e9 6d fd ff ff       	jmpq   7b1e <__scif_pollfd+0x15e>
    7db1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
		p->qproc(filp, wait_address, p);
    7db8:	4c 89 f2             	mov    %r14,%rdx
    7dbb:	4c 89 ff             	mov    %r15,%rdi
    7dbe:	41 ff 16             	callq  *(%r14)
    7dc1:	e9 af fd ff ff       	jmpq   7b75 <__scif_pollfd+0x1b5>
    7dc6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    7dcd:	00 00 00 
    7dd0:	48 89 d6             	mov    %rdx,%rsi
    7dd3:	4c 89 ef             	mov    %r13,%rdi
    7dd6:	e8 00 00 00 00       	callq  7ddb <__scif_pollfd+0x41b>
    7ddb:	e9 95 fd ff ff       	jmpq   7b75 <__scif_pollfd+0x1b5>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    7de0:	4d 8d a5 68 01 00 00 	lea    0x168(%r13),%r12
    7de7:	4c 89 e7             	mov    %r12,%rdi
    7dea:	e8 00 00 00 00       	callq  7def <__scif_pollfd+0x42f>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    7def:	41 8b 45 04          	mov    0x4(%r13),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    7df3:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    7df6:	83 fa 01             	cmp    $0x1,%edx
    7df9:	76 1a                	jbe    7e15 <__scif_pollfd+0x455>
    7dfb:	83 f8 01             	cmp    $0x1,%eax
    7dfe:	74 15                	je     7e15 <__scif_pollfd+0x455>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    7e00:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    7e07:	48 85 c0             	test   %rax,%rax
    7e0a:	78 16                	js     7e22 <__scif_pollfd+0x462>
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
    7e0c:	f0 49 83 85 88 01 00 	lock addq $0x1,0x188(%r13)
    7e13:	00 01 
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
bail_out:
		mutex_unlock(&dev->sd_lock);
    7e15:	4c 89 e7             	mov    %r12,%rdi
    7e18:	e8 00 00 00 00       	callq  7e1d <__scif_pollfd+0x45d>
    7e1d:	e9 d2 fb ff ff       	jmpq   79f4 <__scif_pollfd+0x34>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    7e22:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 7e28 <__scif_pollfd+0x468>
			notif_msg.dst.node = SCIF_HOST_NODE;
    7e28:	31 d2                	xor    %edx,%edx
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    7e2a:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    7e31:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    7e38:	66 89 55 a8          	mov    %dx,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    7e3c:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    7e40:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    7e42:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    7e49:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    7e4d:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    7e51:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    7e56:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    7e5a:	e8 00 00 00 00       	callq  7e5f <__scif_pollfd+0x49f>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    7e5f:	49 8b 85 b0 01 00 00 	mov    0x1b0(%r13),%rax
    7e66:	48 83 f8 02          	cmp    $0x2,%rax
    7e6a:	74 19                	je     7e85 <__scif_pollfd+0x4c5>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    7e6c:	48 83 f8 03          	cmp    $0x3,%rax
    7e70:	75 9a                	jne    7e0c <__scif_pollfd+0x44c>
				dev->sd_state = SCIFDEV_RUNNING;
    7e72:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    7e79:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    7e7a:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    7e81:	00 7f 
    7e83:	eb 87                	jmp    7e0c <__scif_pollfd+0x44c>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    7e85:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    7e8c:	30 c0                	xor    %al,%al
    7e8e:	b9 0a 00 00 00       	mov    $0xa,%ecx
    7e93:	f3 ab                	rep stos %eax,%es:(%rdi)
    7e95:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    7e9c:	00 
    7e9d:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    7ea4:	00 00 
    7ea6:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    7eaa:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    7eb1:	48 83 c0 18          	add    $0x18,%rax
    7eb5:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    7eb9:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    7ebd:	49 8d 85 98 01 00 00 	lea    0x198(%r13),%rax
    7ec4:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    7ecb:	eb 08                	jmp    7ed5 <__scif_pollfd+0x515>
    7ecd:	0f 1f 00             	nopl   (%rax)
    7ed0:	e8 00 00 00 00       	callq  7ed5 <__scif_pollfd+0x515>
    7ed5:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    7edc:	ba 02 00 00 00       	mov    $0x2,%edx
    7ee1:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    7ee8:	e8 00 00 00 00       	callq  7eed <__scif_pollfd+0x52d>
    7eed:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    7ef4:	02 
    7ef5:	74 d9                	je     7ed0 <__scif_pollfd+0x510>
    7ef7:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    7efe:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    7f05:	e8 00 00 00 00       	callq  7f0a <__scif_pollfd+0x54a>
    7f0a:	49 8b 85 b0 01 00 00 	mov    0x1b0(%r13),%rax
    7f11:	e9 56 ff ff ff       	jmpq   7e6c <__scif_pollfd+0x4ac>
    7f16:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    7f1d:	00 00 00 

0000000000007f20 <scif_pollfd>:
	return ret;
}
EXPORT_SYMBOL(scif_unregister);

unsigned int scif_pollfd(struct file *f, poll_table *wait, scif_epd_t epd)
{
    7f20:	55                   	push   %rbp
    7f21:	48 89 e5             	mov    %rsp,%rbp
    7f24:	41 56                	push   %r14
    7f26:	41 55                	push   %r13
    7f28:	41 54                	push   %r12
    7f2a:	53                   	push   %rbx
    7f2b:	e8 00 00 00 00       	callq  7f30 <scif_pollfd+0x10>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    7f30:	48 8d 9a 70 01 00 00 	lea    0x170(%rdx),%rbx
    7f37:	49 89 fd             	mov    %rdi,%r13
    7f3a:	49 89 f6             	mov    %rsi,%r14
    7f3d:	49 89 d4             	mov    %rdx,%r12
    7f40:	48 89 df             	mov    %rbx,%rdi
    7f43:	e8 00 00 00 00       	callq  7f48 <scif_pollfd+0x28>
	unsigned int ret;
	get_kref_count(epd);
	ret = __scif_pollfd(f, wait, (struct endpt *)epd);
    7f48:	4c 89 e2             	mov    %r12,%rdx
    7f4b:	4c 89 f6             	mov    %r14,%rsi
    7f4e:	4c 89 ef             	mov    %r13,%rdi
    7f51:	e8 00 00 00 00       	callq  7f56 <scif_pollfd+0x36>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    7f56:	48 89 df             	mov    %rbx,%rdi
    7f59:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    7f60:	41 89 c4             	mov    %eax,%r12d
    7f63:	e8 00 00 00 00       	callq  7f68 <scif_pollfd+0x48>
	put_kref_count(epd);
	return ret;
}
    7f68:	5b                   	pop    %rbx
    7f69:	44 89 e0             	mov    %r12d,%eax
    7f6c:	41 5c                	pop    %r12
    7f6e:	41 5d                	pop    %r13
    7f70:	41 5e                	pop    %r14
    7f72:	5d                   	pop    %rbp
    7f73:	c3                   	retq   
    7f74:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
    7f7b:	00 00 00 00 00 

0000000000007f80 <scif_mmap>:
 *	Upon successful completion, scif_mmap() returns zero
 *	else an apt error is returned as documented in scif.h.
 */
int
scif_mmap(struct vm_area_struct *vma, scif_epd_t epd)
{
    7f80:	55                   	push   %rbp
    7f81:	48 89 e5             	mov    %rsp,%rbp
    7f84:	41 57                	push   %r15
    7f86:	41 56                	push   %r14
    7f88:	41 55                	push   %r13
    7f8a:	41 54                	push   %r12
    7f8c:	53                   	push   %rbx
    7f8d:	48 81 ec c8 00 00 00 	sub    $0xc8,%rsp
    7f94:	e8 00 00 00 00       	callq  7f99 <scif_mmap+0x19>
	uint64_t start_offset = ((vma)->vm_pgoff) << PAGE_SHIFT;
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT);
	int err;
	struct vma_pvt *vmapvt;

	pr_debug("SCIFAPI mmap: ep %p %s start_offset 0x%llx nr_pages 0x%x\n", 
    7f99:	8b 06                	mov    (%rsi),%eax
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    7f9b:	8b 06                	mov    (%rsi),%eax
scif_mmap(struct vm_area_struct *vma, scif_epd_t epd)
{
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
	struct endpt *ep = (struct endpt *)epd;
	uint64_t start_offset = ((vma)->vm_pgoff) << PAGE_SHIFT;
    7f9d:	4c 8b a7 90 00 00 00 	mov    0x90(%rdi),%r12
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT);
    7fa4:	4c 8b 6f 10          	mov    0x10(%rdi),%r13
 */
int
scif_mmap(struct vm_area_struct *vma, scif_epd_t epd)
{
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
    7fa8:	48 c7 85 40 ff ff ff 	movq   $0x0,-0xc0(%rbp)
    7faf:	00 00 00 00 
	struct endpt *ep = (struct endpt *)epd;
	uint64_t start_offset = ((vma)->vm_pgoff) << PAGE_SHIFT;
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT);
    7fb3:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
    7fb7:	83 f8 09             	cmp    $0x9,%eax
    7fba:	74 24                	je     7fe0 <scif_mmap+0x60>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    7fbc:	8b 16                	mov    (%rsi),%edx
		return -ENOTCONN;
    7fbe:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    7fc3:	83 fa 04             	cmp    $0x4,%edx
    7fc6:	74 28                	je     7ff0 <scif_mmap+0x70>
		kfree(vmapvt);
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		micscif_rma_put_task(ep, nr_pages);
	}
	return err;
}
    7fc8:	48 81 c4 c8 00 00 00 	add    $0xc8,%rsp
    7fcf:	5b                   	pop    %rbx
    7fd0:	41 5c                	pop    %r12
    7fd2:	41 5d                	pop    %r13
    7fd4:	41 5e                	pop    %r14
    7fd6:	41 5f                	pop    %r15
    7fd8:	5d                   	pop    %rbp
    7fd9:	c3                   	retq   
    7fda:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;
    7fe0:	b8 98 ff ff ff       	mov    $0xffffff98,%eax
    7fe5:	eb e1                	jmp    7fc8 <scif_mmap+0x48>
    7fe7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    7fee:	00 00 
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    7ff0:	48 8b 86 48 01 00 00 	mov    0x148(%rsi),%rax
    7ff7:	8b 50 04             	mov    0x4(%rax),%edx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    7ffa:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    7fff:	83 ea 02             	sub    $0x2,%edx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    8002:	83 fa 01             	cmp    $0x1,%edx
    8005:	77 c1                	ja     7fc8 <scif_mmap+0x48>
    8007:	83 be 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rsi)
    800e:	75 b8                	jne    7fc8 <scif_mmap+0x48>
{
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
	struct endpt *ep = (struct endpt *)epd;
	uint64_t start_offset = ((vma)->vm_pgoff) << PAGE_SHIFT;
	int nr_pages = (int)( (((vma)->vm_end) - ((vma)->vm_start)) >> PAGE_SHIFT);
    8010:	49 29 cd             	sub    %rcx,%r13
    8013:	48 89 f3             	mov    %rsi,%rbx
    8016:	49 89 ff             	mov    %rdi,%r15
    8019:	49 c1 ed 0c          	shr    $0xc,%r13
    801d:	4c 89 ad 38 ff ff ff 	mov    %r13,-0xc8(%rbp)
		ep, scif_ep_states[ep->state], start_offset, nr_pages);

	if ((err = verify_epd(ep)))
		return err;

	might_sleep();
    8024:	e8 00 00 00 00       	callq  8029 <scif_mmap+0xa9>

	if ((err = micscif_rma_get_task(ep, nr_pages)))
    8029:	44 89 ee             	mov    %r13d,%esi
    802c:	48 89 df             	mov    %rbx,%rdi
    802f:	e8 00 00 00 00       	callq  8034 <scif_mmap+0xb4>
    8034:	85 c0                	test   %eax,%eax
    8036:	75 90                	jne    7fc8 <scif_mmap+0x48>
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return kmalloc_caches[index];
    8038:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 803f <scif_mmap+0xbf>
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
    803f:	48 85 ff             	test   %rdi,%rdi
    8042:	0f 84 a8 02 00 00    	je     82f0 <scif_mmap+0x370>
				return ZERO_SIZE_PTR;

			return kmem_cache_alloc_trace(s, flags, size);
    8048:	ba 18 00 00 00       	mov    $0x18,%edx
    804d:	be d0 80 00 00       	mov    $0x80d0,%esi
    8052:	e8 00 00 00 00       	callq  8057 <scif_mmap+0xd7>
		return err;

	if (!(vmapvt = kzalloc(sizeof(*vmapvt), GFP_KERNEL))) {
    8057:	48 85 c0             	test   %rax,%rax
    805a:	49 89 c6             	mov    %rax,%r14
    805d:	0f 84 69 03 00 00    	je     83cc <scif_mmap+0x44c>
		micscif_rma_put_task(ep, nr_pages);
		return -ENOMEM;
	}

	vmapvt->ep = ep;
	kref_init(&vmapvt->ref);
    8063:	49 8d 7e 14          	lea    0x14(%r14),%rdi
	if (!(vmapvt = kzalloc(sizeof(*vmapvt), GFP_KERNEL))) {
		micscif_rma_put_task(ep, nr_pages);
		return -ENOMEM;
	}

	vmapvt->ep = ep;
    8067:	49 89 1e             	mov    %rbx,(%r14)
scif_mmap(struct vm_area_struct *vma, scif_epd_t epd)
{
	struct micscif_rma_req req;
	struct reg_range_t *window = NULL;
	struct endpt *ep = (struct endpt *)epd;
	uint64_t start_offset = ((vma)->vm_pgoff) << PAGE_SHIFT;
    806a:	49 c1 e4 0c          	shl    $0xc,%r12
		micscif_rma_put_task(ep, nr_pages);
		return -ENOMEM;
	}

	vmapvt->ep = ep;
	kref_init(&vmapvt->ref);
    806e:	e8 00 00 00 00       	callq  8073 <scif_mmap+0xf3>

	micscif_create_node_dep(ep->remote_dev, nr_pages);
    8073:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
    807a:	8b b5 38 ff ff ff    	mov    -0xc8(%rbp),%esi
    8080:	e8 00 00 00 00       	callq  8085 <scif_mmap+0x105>

	req.out_window = &window;
    8085:	48 8d 85 40 ff ff ff 	lea    -0xc0(%rbp),%rax
	req.offset = start_offset;
    808c:	4c 89 65 a8          	mov    %r12,-0x58(%rbp)
	req.nr_bytes = ((vma)->vm_end) - ((vma)->vm_start);
	req.prot = ((vma)->vm_flags) & (VM_READ | VM_WRITE);
	req.type = WINDOW_PARTIAL;
	req.head = &ep->rma_info.remote_reg_list;

	micscif_inc_node_refcnt(ep->remote_dev, 1);
    8090:	4c 8b ab 48 01 00 00 	mov    0x148(%rbx),%r13
	vmapvt->ep = ep;
	kref_init(&vmapvt->ref);

	micscif_create_node_dep(ep->remote_dev, nr_pages);

	req.out_window = &window;
    8097:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
	req.offset = start_offset;
	req.nr_bytes = ((vma)->vm_end) - ((vma)->vm_start);
    809b:	49 8b 47 10          	mov    0x10(%r15),%rax
    809f:	49 2b 47 08          	sub    0x8(%r15),%rax
	req.prot = ((vma)->vm_flags) & (VM_READ | VM_WRITE);
	req.type = WINDOW_PARTIAL;
    80a3:	c7 45 bc 00 00 00 00 	movl   $0x0,-0x44(%rbp)

	micscif_create_node_dep(ep->remote_dev, nr_pages);

	req.out_window = &window;
	req.offset = start_offset;
	req.nr_bytes = ((vma)->vm_end) - ((vma)->vm_start);
    80aa:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
	req.prot = ((vma)->vm_flags) & (VM_READ | VM_WRITE);
    80ae:	41 8b 47 30          	mov    0x30(%r15),%eax
    80b2:	83 e0 03             	and    $0x3,%eax
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    80b5:	4d 85 ed             	test   %r13,%r13
    80b8:	89 45 b8             	mov    %eax,-0x48(%rbp)
	req.type = WINDOW_PARTIAL;
	req.head = &ep->rma_info.remote_reg_list;
    80bb:	48 8d 43 38          	lea    0x38(%rbx),%rax
    80bf:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    80c3:	0f 85 47 02 00 00    	jne    8310 <scif_mmap+0x390>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);
    80c9:	4c 8d ab 80 00 00 00 	lea    0x80(%rbx),%r13
    80d0:	4c 89 ef             	mov    %r13,%rdi
    80d3:	e8 00 00 00 00       	callq  80d8 <scif_mmap+0x158>
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
    80d8:	48 8d 7d a0          	lea    -0x60(%rbp),%rdi
    80dc:	e8 00 00 00 00       	callq  80e1 <scif_mmap+0x161>
    80e1:	85 c0                	test   %eax,%eax
    80e3:	0f 85 29 05 00 00    	jne    8612 <scif_mmap+0x692>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	RMA_MAGIC(window);
    80e9:	48 8b bd 40 ff ff ff 	mov    -0xc0(%rbp),%rdi
    80f0:	48 b8 1f 5c 00 00 00 	movabs $0x5c1f000000005c1f,%rax
    80f7:	00 1f 5c 
    80fa:	48 39 47 18          	cmp    %rax,0x18(%rdi)
    80fe:	0f 85 6c 02 00 00    	jne    8370 <scif_mmap+0x3f0>
    8104:	48 8b 83 48 01 00 00 	mov    0x148(%rbx),%rax
 *
 * Returns true if the SCIF Device passed is the self aka Loopback SCIF device.
 */
static inline int is_self_scifdev(struct micscif_dev *dev)
{
	return dev->sd_node == ms_info.mi_nodeid;
    810b:	8b 15 00 00 00 00    	mov    0x0(%rip),%edx        # 8111 <scif_mmap+0x191>
    8111:	0f b7 00             	movzwl (%rax),%eax

	/* Default prot for loopback */
	if (!is_self_scifdev(ep->remote_dev)) {
    8114:	39 d0                	cmp    %edx,%eax
    8116:	74 1a                	je     8132 <scif_mmap+0x1b2>
#ifdef _MIC_SCIF_
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#else
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
    8118:	49 8b 7f 28          	mov    0x28(%r15),%rdi
    811c:	e8 00 00 00 00       	callq  8121 <scif_mmap+0x1a1>
    8121:	48 8b bd 40 ff ff ff 	mov    -0xc0(%rbp),%rdi
    8128:	8b 15 00 00 00 00    	mov    0x0(%rip),%edx        # 812e <scif_mmap+0x1ae>
    812e:	49 89 47 28          	mov    %rax,0x28(%r15)
	 * We do not want to copy this VMA automatically on a fork(),
	 * expand this VMA due to mremap() or swap out these pages since
	 * the VMA is actually backed by physical pages in the remote
	 * node's physical memory and not via a struct page.
	 */
	vma->vm_flags |= VM_DONTCOPY | VM_DONTEXPAND | VM_RESERVED | VM_PFNMAP;
    8132:	49 8b 47 30          	mov    0x30(%r15),%rax
    8136:	48 89 c1             	mov    %rax,%rcx
    8139:	48 81 c9 00 04 0e 00 	or     $0xe0400,%rcx
    8140:	49 89 4f 30          	mov    %rcx,0x30(%r15)
    8144:	48 8b 8b 48 01 00 00 	mov    0x148(%rbx),%rcx
    814b:	0f b7 09             	movzwl (%rcx),%ecx

	if (!is_self_scifdev(ep->remote_dev))
    814e:	39 ca                	cmp    %ecx,%edx
    8150:	74 0a                	je     815c <scif_mmap+0x1dc>
		((vma)->vm_flags) |= VM_IO;
    8152:	48 0d 00 44 0e 00    	or     $0xe4400,%rax
    8158:	49 89 47 30          	mov    %rax,0x30(%r15)

	/* Map this range of windows */
	if ((err = micscif_rma_list_mmap(window,
    815c:	8b 95 38 ff ff ff    	mov    -0xc8(%rbp),%edx
    8162:	4c 89 f9             	mov    %r15,%rcx
    8165:	4c 89 e6             	mov    %r12,%rsi
    8168:	e8 00 00 00 00       	callq  816d <scif_mmap+0x1ed>
    816d:	85 c0                	test   %eax,%eax
    816f:	41 89 c2             	mov    %eax,%r10d
    8172:	0f 85 f5 02 00 00    	jne    846d <scif_mmap+0x4ed>
	 * For 1 page sized VMAs the kernel (remap_pfn_range) replaces the
	 * offset in the VMA with the pfn, so in that case save off the
	 * original offset, since the page sized VMA can't be split into
	 * smaller VMAs the offset is not going to change.
	 */
	if (nr_pages == 1) {
    8178:	83 bd 38 ff ff ff 01 	cmpl   $0x1,-0xc8(%rbp)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	/* Set up the driver call back */
	vma->vm_ops = &micscif_vm_ops;
	((vma)->vm_private_data) = vmapvt;
    817f:	4d 89 b7 a0 00 00 00 	mov    %r14,0xa0(%r15)
			start_offset, nr_pages, vma))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	/* Set up the driver call back */
	vma->vm_ops = &micscif_vm_ops;
    8186:	49 c7 87 88 00 00 00 	movq   $0x0,0x88(%r15)
    818d:	00 00 00 00 
	 * For 1 page sized VMAs the kernel (remap_pfn_range) replaces the
	 * offset in the VMA with the pfn, so in that case save off the
	 * original offset, since the page sized VMA can't be split into
	 * smaller VMAs the offset is not going to change.
	 */
	if (nr_pages == 1) {
    8191:	0f 84 69 01 00 00    	je     8300 <scif_mmap+0x380>
		vmapvt->offset = start_offset;
		vmapvt->valid_offset = true;
	}
	err = 0;
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    8197:	4c 89 ef             	mov    %r13,%rdi
    819a:	44 89 95 30 ff ff ff 	mov    %r10d,-0xd0(%rbp)
    81a1:	e8 00 00 00 00       	callq  81a6 <scif_mmap+0x226>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    81a6:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    81ad:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
    81b4:	4d 85 e4             	test   %r12,%r12
    81b7:	74 1f                	je     81d8 <scif_mmap+0x258>
		if (unlikely((atomic_long_sub_return(cnt, 
    81b9:	4d 8d bc 24 88 01 00 	lea    0x188(%r12),%r15
    81c0:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    81c1:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    81c8:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    81cf:	01 00 00 
    81d2:	48 83 e8 01          	sub    $0x1,%rax
    81d6:	78 07                	js     81df <scif_mmap+0x25f>
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    81d8:	31 c0                	xor    %eax,%eax
    81da:	e9 e9 fd ff ff       	jmpq   7fc8 <scif_mmap+0x48>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    81df:	48 8b 45 08          	mov    0x8(%rbp),%rax
    81e3:	4c 89 e1             	mov    %r12,%rcx
    81e6:	ba a7 00 00 00       	mov    $0xa7,%edx
    81eb:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    81f2:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    81f9:	00 
    81fa:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8201:	44 89 95 30 ff ff ff 	mov    %r10d,-0xd0(%rbp)
    8208:	45 0f b7 04 24       	movzwl (%r12),%r8d
    820d:	48 89 04 24          	mov    %rax,(%rsp)
    8211:	31 c0                	xor    %eax,%eax
    8213:	e8 00 00 00 00       	callq  8218 <scif_mmap+0x298>
    8218:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    821f:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8220:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    8227:	00 00 80 
    822a:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
    8231:	48 39 c1             	cmp    %rax,%rcx
    8234:	74 a2                	je     81d8 <scif_mmap+0x258>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8236:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    823a:	48 89 c8             	mov    %rcx,%rax
    823d:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    8244:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8247:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    824a:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    824d:	74 89                	je     81d8 <scif_mmap+0x258>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    824f:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8256:	00 00 80 
    8259:	48 39 f2             	cmp    %rsi,%rdx
    825c:	74 15                	je     8273 <scif_mmap+0x2f3>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    825e:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8262:	48 89 d0             	mov    %rdx,%rax
    8265:	f0 49 0f b1 0f       	lock cmpxchg %rcx,(%r15)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    826a:	48 39 c2             	cmp    %rax,%rdx
    826d:	0f 85 b1 03 00 00    	jne    8624 <scif_mmap+0x6a4>
	if (err) {
    8273:	45 85 d2             	test   %r10d,%r10d
    8276:	0f 84 5c ff ff ff    	je     81d8 <scif_mmap+0x258>
		micscif_destroy_node_dep(ep->remote_dev, nr_pages);
    827c:	4c 8b bd 38 ff ff ff 	mov    -0xc8(%rbp),%r15
    8283:	44 89 95 30 ff ff ff 	mov    %r10d,-0xd0(%rbp)
    828a:	48 8b bb 48 01 00 00 	mov    0x148(%rbx),%rdi
    8291:	44 89 fe             	mov    %r15d,%esi
    8294:	e8 00 00 00 00       	callq  8299 <scif_mmap+0x319>
		kfree(vmapvt);
    8299:	4c 89 f7             	mov    %r14,%rdi
    829c:	e8 00 00 00 00       	callq  82a1 <scif_mmap+0x321>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    82a1:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
    82a8:	31 c0                	xor    %eax,%eax
    82aa:	ba 62 0b 00 00       	mov    $0xb62,%edx
    82af:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    82b6:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    82bd:	44 89 d1             	mov    %r10d,%ecx
    82c0:	44 89 95 38 ff ff ff 	mov    %r10d,-0xc8(%rbp)
    82c7:	e8 00 00 00 00       	callq  82cc <scif_mmap+0x34c>
		micscif_rma_put_task(ep, nr_pages);
    82cc:	44 89 fe             	mov    %r15d,%esi
    82cf:	48 89 df             	mov    %rbx,%rdi
    82d2:	e8 00 00 00 00       	callq  82d7 <scif_mmap+0x357>
    82d7:	44 8b 95 38 ff ff ff 	mov    -0xc8(%rbp),%r10d
    82de:	44 89 d0             	mov    %r10d,%eax
    82e1:	e9 e2 fc ff ff       	jmpq   7fc8 <scif_mmap+0x48>
    82e6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    82ed:	00 00 00 

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;
    82f0:	41 be 10 00 00 00    	mov    $0x10,%r14d
    82f6:	e9 68 fd ff ff       	jmpq   8063 <scif_mmap+0xe3>
    82fb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	 * offset in the VMA with the pfn, so in that case save off the
	 * original offset, since the page sized VMA can't be split into
	 * smaller VMAs the offset is not going to change.
	 */
	if (nr_pages == 1) {
		vmapvt->offset = start_offset;
    8300:	4d 89 66 08          	mov    %r12,0x8(%r14)
		vmapvt->valid_offset = true;
    8304:	41 c6 46 10 01       	movb   $0x1,0x10(%r14)
    8309:	e9 89 fe ff ff       	jmpq   8197 <scif_mmap+0x217>
    830e:	66 90                	xchg   %ax,%ax
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    8310:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8317:	4d 8d 95 88 01 00 00 	lea    0x188(%r13),%r10
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    831e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8325:	00 00 80 
    8328:	48 39 f1             	cmp    %rsi,%rcx
    832b:	74 45                	je     8372 <scif_mmap+0x3f2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    832d:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8331:	48 89 c8             	mov    %rcx,%rax
    8334:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    833b:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    833d:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8340:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8343:	0f 84 80 fd ff ff    	je     80c9 <scif_mmap+0x149>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8349:	48 39 f2             	cmp    %rsi,%rdx
    834c:	74 24                	je     8372 <scif_mmap+0x3f2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    834e:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8352:	48 89 d0             	mov    %rdx,%rax
    8355:	f0 49 0f b1 0a       	lock cmpxchg %rcx,(%r10)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    835a:	48 39 c2             	cmp    %rax,%rdx
    835d:	0f 84 66 fd ff ff    	je     80c9 <scif_mmap+0x149>
    8363:	48 89 c2             	mov    %rax,%rdx
    8366:	eb e1                	jmp    8349 <scif_mmap+0x3c9>
    8368:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    836f:	00 
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
		goto error;
	}
	RMA_MAGIC(window);
    8370:	0f 0b                	ud2    
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    8372:	49 8d 85 68 01 00 00 	lea    0x168(%r13),%rax
    8379:	4c 89 95 30 ff ff ff 	mov    %r10,-0xd0(%rbp)
    8380:	48 89 c7             	mov    %rax,%rdi
    8383:	48 89 85 28 ff ff ff 	mov    %rax,-0xd8(%rbp)
    838a:	e8 00 00 00 00       	callq  838f <scif_mmap+0x40f>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    838f:	41 8b 45 04          	mov    0x4(%r13),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    8393:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    8396:	83 fa 01             	cmp    $0x1,%edx
    8399:	76 20                	jbe    83bb <scif_mmap+0x43b>
    839b:	83 f8 01             	cmp    $0x1,%eax
    839e:	4c 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10
    83a5:	74 14                	je     83bb <scif_mmap+0x43b>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    83a7:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    83ae:	48 85 c0             	test   %rax,%rax
    83b1:	78 31                	js     83e4 <scif_mmap+0x464>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    83b3:	4c 89 d7             	mov    %r10,%rdi
    83b6:	e8 35 81 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    83bb:	48 8b bd 28 ff ff ff 	mov    -0xd8(%rbp),%rdi
    83c2:	e8 00 00 00 00       	callq  83c7 <scif_mmap+0x447>
    83c7:	e9 fd fc ff ff       	jmpq   80c9 <scif_mmap+0x149>

	if ((err = micscif_rma_get_task(ep, nr_pages)))
		return err;

	if (!(vmapvt = kzalloc(sizeof(*vmapvt), GFP_KERNEL))) {
		micscif_rma_put_task(ep, nr_pages);
    83cc:	8b b5 38 ff ff ff    	mov    -0xc8(%rbp),%esi
    83d2:	48 89 df             	mov    %rbx,%rdi
    83d5:	e8 00 00 00 00       	callq  83da <scif_mmap+0x45a>
		return -ENOMEM;
    83da:	b8 f4 ff ff ff       	mov    $0xfffffff4,%eax
    83df:	e9 e4 fb ff ff       	jmpq   7fc8 <scif_mmap+0x48>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    83e4:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 83ea <scif_mmap+0x46a>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    83ea:	31 d2                	xor    %edx,%edx
    83ec:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    83f3:	c7 85 7c ff ff ff 2e 	movl   $0x2e,-0x84(%rbp)
    83fa:	00 00 00 
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    83fd:	66 c7 85 78 ff ff ff 	movw   $0x0,-0x88(%rbp)
    8404:	00 00 
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    8406:	48 8d b5 74 ff ff ff 	lea    -0x8c(%rbp),%rsi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    840d:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    8414:	02 00 00 00 
    8418:	4c 89 95 30 ff ff ff 	mov    %r10,-0xd0(%rbp)
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    841f:	66 89 85 74 ff ff ff 	mov    %ax,-0x8c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    8426:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    842b:	48 89 45 80          	mov    %rax,-0x80(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    842f:	e8 00 00 00 00       	callq  8434 <scif_mmap+0x4b4>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    8434:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    843b:	02 
    843c:	4c 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10
    8443:	0f 84 13 01 00 00    	je     855c <scif_mmap+0x5dc>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    8449:	49 83 bd b0 01 00 00 	cmpq   $0x3,0x1b0(%r13)
    8450:	03 
    8451:	0f 85 5c ff ff ff    	jne    83b3 <scif_mmap+0x433>
				dev->sd_state = SCIFDEV_RUNNING;
    8457:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    845e:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    845f:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    8466:	00 7f 
    8468:	e9 46 ff ff ff       	jmpq   83b3 <scif_mmap+0x433>
		((vma)->vm_flags) |= VM_IO;

	/* Map this range of windows */
	if ((err = micscif_rma_list_mmap(window,
			start_offset, nr_pages, vma))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    846d:	89 c1                	mov    %eax,%ecx
    846f:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    8475:	ba 4b 0b 00 00       	mov    $0xb4b,%edx
    847a:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    8481:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8488:	31 c0                	xor    %eax,%eax
    848a:	e8 00 00 00 00       	callq  848f <scif_mmap+0x50f>
		goto error;
    848f:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
		vmapvt->offset = start_offset;
		vmapvt->valid_offset = true;
	}
	err = 0;
error:
	mutex_unlock(&ep->rma_info.rma_lock);
    8496:	4c 89 ef             	mov    %r13,%rdi
    8499:	44 89 95 30 ff ff ff 	mov    %r10d,-0xd0(%rbp)
    84a0:	e8 00 00 00 00       	callq  84a5 <scif_mmap+0x525>
	micscif_dec_node_refcnt(ep->remote_dev, 1);
    84a5:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    84ac:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
    84b3:	4d 85 e4             	test   %r12,%r12
    84b6:	0f 84 c0 fd ff ff    	je     827c <scif_mmap+0x2fc>
		if (unlikely((atomic_long_sub_return(cnt, 
    84bc:	4d 8d bc 24 88 01 00 	lea    0x188(%r12),%r15
    84c3:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    84c4:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    84cb:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    84d2:	01 00 00 
    84d5:	48 83 e8 01          	sub    $0x1,%rax
    84d9:	0f 89 9d fd ff ff    	jns    827c <scif_mmap+0x2fc>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    84df:	48 8b 45 08          	mov    0x8(%rbp),%rax
    84e3:	4c 89 e1             	mov    %r12,%rcx
    84e6:	ba a7 00 00 00       	mov    $0xa7,%edx
    84eb:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    84f2:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    84f9:	00 
    84fa:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8501:	44 89 95 30 ff ff ff 	mov    %r10d,-0xd0(%rbp)
    8508:	45 0f b7 04 24       	movzwl (%r12),%r8d
    850d:	48 89 04 24          	mov    %rax,(%rsp)
    8511:	31 c0                	xor    %eax,%eax
    8513:	e8 00 00 00 00       	callq  8518 <scif_mmap+0x598>
    8518:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    851f:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8520:	48 b8 00 00 00 00 00 	movabs $0x8000000000000000,%rax
    8527:	00 00 80 
    852a:	44 8b 95 30 ff ff ff 	mov    -0xd0(%rbp),%r10d
    8531:	48 39 c1             	cmp    %rax,%rcx
    8534:	0f 84 42 fd ff ff    	je     827c <scif_mmap+0x2fc>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    853a:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    853e:	48 89 c8             	mov    %rcx,%rax
    8541:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    8548:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    854b:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    854e:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8551:	0f 84 25 fd ff ff    	je     827c <scif_mmap+0x2fc>
    8557:	e9 f3 fc ff ff       	jmpq   824f <scif_mmap+0x2cf>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    855c:	48 8d bd 48 ff ff ff 	lea    -0xb8(%rbp),%rdi
    8563:	31 c0                	xor    %eax,%eax
    8565:	b9 0a 00 00 00       	mov    $0xa,%ecx
    856a:	f3 ab                	rep stos %eax,%es:(%rdi)
    856c:	48 c7 85 58 ff ff ff 	movq   $0x0,-0xa8(%rbp)
    8573:	00 00 00 00 
    8577:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    857e:	00 00 
    8580:	48 89 85 50 ff ff ff 	mov    %rax,-0xb0(%rbp)
    8587:	48 8d 85 48 ff ff ff 	lea    -0xb8(%rbp),%rax
    858e:	48 83 c0 18          	add    $0x18,%rax
    8592:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    8599:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    85a0:	49 8d 85 98 01 00 00 	lea    0x198(%r13),%rax
    85a7:	48 89 85 30 ff ff ff 	mov    %rax,-0xd0(%rbp)
    85ae:	eb 0c                	jmp    85bc <scif_mmap+0x63c>
    85b0:	e8 00 00 00 00       	callq  85b5 <scif_mmap+0x635>
    85b5:	4c 8b 95 20 ff ff ff 	mov    -0xe0(%rbp),%r10
    85bc:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    85c3:	ba 02 00 00 00       	mov    $0x2,%edx
    85c8:	4c 89 95 20 ff ff ff 	mov    %r10,-0xe0(%rbp)
    85cf:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    85d6:	e8 00 00 00 00       	callq  85db <scif_mmap+0x65b>
    85db:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    85e2:	02 
    85e3:	4c 8b 95 20 ff ff ff 	mov    -0xe0(%rbp),%r10
    85ea:	74 c4                	je     85b0 <scif_mmap+0x630>
    85ec:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    85f3:	48 8d b5 48 ff ff ff 	lea    -0xb8(%rbp),%rsi
    85fa:	4c 89 95 20 ff ff ff 	mov    %r10,-0xe0(%rbp)
    8601:	e8 00 00 00 00       	callq  8606 <scif_mmap+0x686>
    8606:	4c 8b 95 20 ff ff ff 	mov    -0xe0(%rbp),%r10
    860d:	e9 37 fe ff ff       	jmpq   8449 <scif_mmap+0x4c9>

	micscif_inc_node_refcnt(ep->remote_dev, 1);
	mutex_lock(&ep->rma_info.rma_lock);
	/* Does a valid window exist? */
	if ((err = micscif_query_window(&req))) {
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    8612:	89 c1                	mov    %eax,%ecx
    8614:	89 85 30 ff ff ff    	mov    %eax,-0xd0(%rbp)
    861a:	ba 29 0b 00 00       	mov    $0xb29,%edx
    861f:	e9 56 fe ff ff       	jmpq   847a <scif_mmap+0x4fa>
    8624:	48 89 c2             	mov    %rax,%rdx
    8627:	e9 2d fc ff ff       	jmpq   8259 <scif_mmap+0x2d9>
    862c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000008630 <__scif_fence_mark>:
 * Return Values
 *	Upon successful completion, scif_fence_mark() returns 0;
 *	else an apt error is returned as documented in scif.h.
 */
int __scif_fence_mark(scif_epd_t epd, int flags, int *mark)
{
    8630:	55                   	push   %rbp
    8631:	48 89 e5             	mov    %rsp,%rbp
    8634:	41 57                	push   %r15
    8636:	41 56                	push   %r14
    8638:	41 55                	push   %r13
    863a:	41 54                	push   %r12
    863c:	53                   	push   %rbx
    863d:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    8644:	e8 00 00 00 00       	callq  8649 <__scif_fence_mark+0x19>
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;

	pr_debug("SCIFAPI fence_mark: ep %p %s flags 0x%x mark 0x%x\n", 
    8649:	8b 07                	mov    (%rdi),%eax
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    864b:	8b 07                	mov    (%rdi),%eax
    864d:	83 f8 09             	cmp    $0x9,%eax
    8650:	74 1e                	je     8670 <__scif_fence_mark+0x40>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8652:	8b 0f                	mov    (%rdi),%ecx
		return -ENOTCONN;
    8654:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8659:	83 f9 04             	cmp    $0x4,%ecx
    865c:	74 22                	je     8680 <__scif_fence_mark+0x50>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);

	pr_debug("SCIFAPI fence_mark: ep %p %s flags 0x%x mark 0x%x err %d\n", 
		ep, scif_ep_states[ep->state], flags, *mark, err);
	return err;
}
    865e:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    8665:	5b                   	pop    %rbx
    8666:	41 5c                	pop    %r12
    8668:	41 5d                	pop    %r13
    866a:	41 5e                	pop    %r14
    866c:	41 5f                	pop    %r15
    866e:	5d                   	pop    %rbp
    866f:	c3                   	retq   
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;
    8670:	b8 98 ff ff ff       	mov    $0xffffff98,%eax
    8675:	eb e7                	jmp    865e <__scif_fence_mark+0x2e>
    8677:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    867e:	00 00 
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    8680:	48 8b 9f 48 01 00 00 	mov    0x148(%rdi),%rbx
    8687:	8b 43 04             	mov    0x4(%rbx),%eax
    868a:	8d 48 fe             	lea    -0x2(%rax),%ecx
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    868d:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
    8692:	83 f9 01             	cmp    $0x1,%ecx
    8695:	77 c7                	ja     865e <__scif_fence_mark+0x2e>
    8697:	83 bf 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rdi)
    869e:	75 be                	jne    865e <__scif_fence_mark+0x2e>

	if ((err = verify_epd(ep)))
		return err;

	/* Invalid flags? */
	if (flags & ~(SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER))
    86a0:	89 f0                	mov    %esi,%eax
    86a2:	83 e0 fc             	and    $0xfffffffc,%eax
    86a5:	0f 85 b5 00 00 00    	jne    8760 <__scif_fence_mark+0x130>
		return -EINVAL;

	/* At least one of init self or peer RMA should be set */
	if (!(flags & (SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER)))
    86ab:	85 f6                	test   %esi,%esi
    86ad:	0f 84 ad 00 00 00    	je     8760 <__scif_fence_mark+0x130>
		return -EINVAL;

	/* Exactly one of init self or peer RMA should be set but not both */
	if ((flags & SCIF_FENCE_INIT_SELF) && (flags & SCIF_FENCE_INIT_PEER))
    86b3:	83 fe 03             	cmp    $0x3,%esi
    86b6:	0f 84 a4 00 00 00    	je     8760 <__scif_fence_mark+0x130>
 *
 * Returns true if the SCIF Device passed is the self aka Loopback SCIF device.
 */
static inline int is_self_scifdev(struct micscif_dev *dev)
{
	return dev->sd_node == ms_info.mi_nodeid;
    86bc:	0f b7 0b             	movzwl (%rbx),%ecx
#ifndef _MIC_SCIF_
	/*
	 * Host Loopback does not need to use DMA.
	 * Return a valid mark to be symmetric.
	 */
	if (is_self_scifdev(ep->remote_dev)) {
    86bf:	3b 0d 00 00 00 00    	cmp    0x0(%rip),%ecx        # 86c5 <__scif_fence_mark+0x95>
    86c5:	0f 84 85 00 00 00    	je     8750 <__scif_fence_mark+0x120>
    86cb:	89 f0                	mov    %esi,%eax
    86cd:	49 89 d4             	mov    %rdx,%r12
    86d0:	49 89 fd             	mov    %rdi,%r13
		*mark = HOST_LOOPB_MAGIC_MARK;
		return 0;
	}
#endif

	if (flags & SCIF_FENCE_INIT_SELF) {
    86d3:	a8 01                	test   $0x1,%al
    86d5:	75 59                	jne    8730 <__scif_fence_mark+0x100>
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    86d7:	48 85 db             	test   %rbx,%rbx
    86da:	0f 85 90 00 00 00    	jne    8770 <__scif_fence_mark+0x140>
		if ((*mark = micscif_fence_mark(epd)) < 0)
			err = *mark;
	} else {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_mark(ep, mark);
    86e0:	4c 89 e6             	mov    %r12,%rsi
    86e3:	4c 89 ef             	mov    %r13,%rdi
    86e6:	e8 00 00 00 00       	callq  86eb <__scif_fence_mark+0xbb>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    86eb:	4d 8b a5 48 01 00 00 	mov    0x148(%r13),%r12
	if (flags & SCIF_FENCE_INIT_SELF) {
		if ((*mark = micscif_fence_mark(epd)) < 0)
			err = *mark;
	} else {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_mark(ep, mark);
    86f2:	89 c3                	mov    %eax,%ebx
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    86f4:	4d 85 e4             	test   %r12,%r12
    86f7:	74 23                	je     871c <__scif_fence_mark+0xec>
		if (unlikely((atomic_long_sub_return(cnt, 
    86f9:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    8700:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    8701:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    8708:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    870f:	01 00 00 
    8712:	48 83 e8 01          	sub    $0x1,%rax
    8716:	0f 88 b4 00 00 00    	js     87d0 <__scif_fence_mark+0x1a0>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	}
	if (err)
    871c:	85 db                	test   %ebx,%ebx
    871e:	0f 85 dc 01 00 00    	jne    8900 <__scif_fence_mark+0x2d0>
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);

	pr_debug("SCIFAPI fence_mark: ep %p %s flags 0x%x mark 0x%x err %d\n", 
    8724:	41 8b 45 00          	mov    0x0(%r13),%eax
		ep, scif_ep_states[ep->state], flags, *mark, err);
	return err;
    8728:	89 d8                	mov    %ebx,%eax
    872a:	e9 2f ff ff ff       	jmpq   865e <__scif_fence_mark+0x2e>
    872f:	90                   	nop
		return 0;
	}
#endif

	if (flags & SCIF_FENCE_INIT_SELF) {
		if ((*mark = micscif_fence_mark(epd)) < 0)
    8730:	e8 00 00 00 00       	callq  8735 <__scif_fence_mark+0x105>
    8735:	85 c0                	test   %eax,%eax
    8737:	89 c3                	mov    %eax,%ebx
    8739:	41 89 04 24          	mov    %eax,(%r12)
    873d:	0f 88 bd 01 00 00    	js     8900 <__scif_fence_mark+0x2d0>
    8743:	31 db                	xor    %ebx,%ebx
    8745:	eb dd                	jmp    8724 <__scif_fence_mark+0xf4>
    8747:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    874e:	00 00 
	/*
	 * Host Loopback does not need to use DMA.
	 * Return a valid mark to be symmetric.
	 */
	if (is_self_scifdev(ep->remote_dev)) {
		*mark = HOST_LOOPB_MAGIC_MARK;
    8750:	c7 02 ad de 00 00    	movl   $0xdead,(%rdx)
		return 0;
    8756:	e9 03 ff ff ff       	jmpq   865e <__scif_fence_mark+0x2e>
    875b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
	if (flags & ~(SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER))
		return -EINVAL;

	/* At least one of init self or peer RMA should be set */
	if (!(flags & (SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER)))
		return -EINVAL;
    8760:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    8765:	e9 f4 fe ff ff       	jmpq   865e <__scif_fence_mark+0x2e>
    876a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    8770:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8777:	4c 8d bb 88 01 00 00 	lea    0x188(%rbx),%r15
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    877e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8785:	00 00 80 
    8788:	48 39 f1             	cmp    %rsi,%rcx
    878b:	0f 84 cf 00 00 00    	je     8860 <__scif_fence_mark+0x230>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8791:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8795:	48 89 c8             	mov    %rcx,%rax
    8798:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    879f:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    87a1:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    87a4:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    87a7:	0f 84 33 ff ff ff    	je     86e0 <__scif_fence_mark+0xb0>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    87ad:	48 39 f2             	cmp    %rsi,%rdx
    87b0:	0f 84 aa 00 00 00    	je     8860 <__scif_fence_mark+0x230>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    87b6:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    87ba:	48 89 d0             	mov    %rdx,%rax
    87bd:	f0 49 0f b1 0f       	lock cmpxchg %rcx,(%r15)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    87c2:	48 39 c2             	cmp    %rax,%rdx
    87c5:	0f 84 15 ff ff ff    	je     86e0 <__scif_fence_mark+0xb0>
    87cb:	48 89 c2             	mov    %rax,%rdx
    87ce:	eb dd                	jmp    87ad <__scif_fence_mark+0x17d>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    87d0:	48 8b 45 08          	mov    0x8(%rbp),%rax
    87d4:	4c 89 e1             	mov    %r12,%rcx
    87d7:	ba a7 00 00 00       	mov    $0xa7,%edx
    87dc:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    87e3:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    87ea:	00 
    87eb:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    87f2:	45 0f b7 04 24       	movzwl (%r12),%r8d
    87f7:	48 89 04 24          	mov    %rax,(%rsp)
    87fb:	31 c0                	xor    %eax,%eax
    87fd:	e8 00 00 00 00       	callq  8802 <__scif_fence_mark+0x1d2>
    8802:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    8809:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    880a:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8811:	00 00 80 
    8814:	48 39 f1             	cmp    %rsi,%rcx
    8817:	0f 84 ff fe ff ff    	je     871c <__scif_fence_mark+0xec>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    881d:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8821:	48 89 c8             	mov    %rcx,%rax
    8824:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    882b:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    882e:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8831:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8834:	0f 84 e2 fe ff ff    	je     871c <__scif_fence_mark+0xec>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    883a:	48 39 f2             	cmp    %rsi,%rdx
    883d:	0f 84 d9 fe ff ff    	je     871c <__scif_fence_mark+0xec>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8843:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8847:	48 89 d0             	mov    %rdx,%rax
    884a:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    884f:	48 39 c2             	cmp    %rax,%rdx
    8852:	0f 84 c4 fe ff ff    	je     871c <__scif_fence_mark+0xec>
    8858:	48 89 c2             	mov    %rax,%rdx
    885b:	eb dd                	jmp    883a <__scif_fence_mark+0x20a>
    885d:	0f 1f 00             	nopl   (%rax)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    8860:	4c 8d b3 68 01 00 00 	lea    0x168(%rbx),%r14
    8867:	4c 89 f7             	mov    %r14,%rdi
    886a:	e8 00 00 00 00       	callq  886f <__scif_fence_mark+0x23f>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    886f:	8b 43 04             	mov    0x4(%rbx),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    8872:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    8875:	83 fa 01             	cmp    $0x1,%edx
    8878:	76 19                	jbe    8893 <__scif_fence_mark+0x263>
    887a:	83 f8 01             	cmp    $0x1,%eax
    887d:	74 14                	je     8893 <__scif_fence_mark+0x263>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    887f:	48 8b 83 88 01 00 00 	mov    0x188(%rbx),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    8886:	48 85 c0             	test   %rax,%rax
    8889:	78 15                	js     88a0 <__scif_fence_mark+0x270>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    888b:	4c 89 ff             	mov    %r15,%rdi
    888e:	e8 5d 7c ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    8893:	4c 89 f7             	mov    %r14,%rdi
    8896:	e8 00 00 00 00       	callq  889b <__scif_fence_mark+0x26b>
    889b:	e9 40 fe ff ff       	jmpq   86e0 <__scif_fence_mark+0xb0>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    88a0:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 88a6 <__scif_fence_mark+0x276>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    88a6:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    88ad:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    88b1:	31 d2                	xor    %edx,%edx
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    88b3:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    88b9:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    88c0:	48 c7 83 b0 01 00 00 	movq   $0x2,0x1b0(%rbx)
    88c7:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    88cb:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    88cf:	0f b7 03             	movzwl (%rbx),%eax
    88d2:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    88d6:	e8 00 00 00 00       	callq  88db <__scif_fence_mark+0x2ab>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    88db:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    88e2:	02 
    88e3:	74 3c                	je     8921 <__scif_fence_mark+0x2f1>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    88e5:	48 83 bb b0 01 00 00 	cmpq   $0x3,0x1b0(%rbx)
    88ec:	03 
    88ed:	75 9c                	jne    888b <__scif_fence_mark+0x25b>
				dev->sd_state = SCIFDEV_RUNNING;
    88ef:	c7 43 04 02 00 00 00 	movl   $0x2,0x4(%rbx)
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    88f6:	f0 80 a3 8f 01 00 00 	lock andb $0x7f,0x18f(%rbx)
    88fd:	7f 
    88fe:	eb 8b                	jmp    888b <__scif_fence_mark+0x25b>
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_mark(ep, mark);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	}
	if (err)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    8900:	89 d9                	mov    %ebx,%ecx
    8902:	ba d5 0b 00 00       	mov    $0xbd5,%edx
    8907:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    890e:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8915:	31 c0                	xor    %eax,%eax
    8917:	e8 00 00 00 00       	callq  891c <__scif_fence_mark+0x2ec>
    891c:	e9 03 fe ff ff       	jmpq   8724 <__scif_fence_mark+0xf4>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    8921:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    8928:	31 c0                	xor    %eax,%eax
    892a:	b9 0a 00 00 00       	mov    $0xa,%ecx
    892f:	f3 ab                	rep stos %eax,%es:(%rdi)
    8931:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    8938:	00 
    8939:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    8940:	00 00 
    8942:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    8946:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    894d:	48 83 c0 18          	add    $0x18,%rax
    8951:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    8955:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    8959:	48 8d 83 98 01 00 00 	lea    0x198(%rbx),%rax
    8960:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    8967:	eb 0c                	jmp    8975 <__scif_fence_mark+0x345>
    8969:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    8970:	e8 00 00 00 00       	callq  8975 <__scif_fence_mark+0x345>
    8975:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    897c:	ba 02 00 00 00       	mov    $0x2,%edx
    8981:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    8988:	e8 00 00 00 00       	callq  898d <__scif_fence_mark+0x35d>
    898d:	48 83 bb b0 01 00 00 	cmpq   $0x2,0x1b0(%rbx)
    8994:	02 
    8995:	74 d9                	je     8970 <__scif_fence_mark+0x340>
    8997:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    899e:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    89a5:	e8 00 00 00 00       	callq  89aa <__scif_fence_mark+0x37a>
    89aa:	e9 36 ff ff ff       	jmpq   88e5 <__scif_fence_mark+0x2b5>
    89af:	90                   	nop

00000000000089b0 <scif_fence_mark>:
		ep, scif_ep_states[ep->state], flags, *mark, err);
	return err;
}

int scif_fence_mark(scif_epd_t epd, int flags, int *mark)
{
    89b0:	55                   	push   %rbp
    89b1:	48 89 e5             	mov    %rsp,%rbp
    89b4:	41 56                	push   %r14
    89b6:	41 55                	push   %r13
    89b8:	41 54                	push   %r12
    89ba:	53                   	push   %rbx
    89bb:	e8 00 00 00 00       	callq  89c0 <scif_fence_mark+0x10>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    89c0:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    89c7:	49 89 fc             	mov    %rdi,%r12
    89ca:	41 89 f5             	mov    %esi,%r13d
    89cd:	49 89 d6             	mov    %rdx,%r14
    89d0:	48 89 df             	mov    %rbx,%rdi
    89d3:	e8 00 00 00 00       	callq  89d8 <scif_fence_mark+0x28>
	int ret;
	get_kref_count(epd);
	ret = __scif_fence_mark(epd, flags, mark);
    89d8:	4c 89 f2             	mov    %r14,%rdx
    89db:	44 89 ee             	mov    %r13d,%esi
    89de:	4c 89 e7             	mov    %r12,%rdi
    89e1:	e8 00 00 00 00       	callq  89e6 <scif_fence_mark+0x36>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    89e6:	48 89 df             	mov    %rbx,%rdi
    89e9:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    89f0:	41 89 c4             	mov    %eax,%r12d
    89f3:	e8 00 00 00 00       	callq  89f8 <scif_fence_mark+0x48>
	put_kref_count(epd);
	return ret;
}
    89f8:	5b                   	pop    %rbx
    89f9:	44 89 e0             	mov    %r12d,%eax
    89fc:	41 5c                	pop    %r12
    89fe:	41 5d                	pop    %r13
    8a00:	41 5e                	pop    %r14
    8a02:	5d                   	pop    %rbp
    8a03:	c3                   	retq   
    8a04:	66 66 66 2e 0f 1f 84 	data32 data32 nopw %cs:0x0(%rax,%rax,1)
    8a0b:	00 00 00 00 00 

0000000000008a10 <__scif_fence_wait>:
 * Return Values
 *	Upon successful completion, scif_fence_wait() returns 0;
 *	else an apt error is returned as documented in scif.h.
 */
int __scif_fence_wait(scif_epd_t epd, int mark)
{
    8a10:	55                   	push   %rbp
    8a11:	48 89 e5             	mov    %rsp,%rbp
    8a14:	41 57                	push   %r15
    8a16:	41 56                	push   %r14
    8a18:	41 55                	push   %r13
    8a1a:	41 54                	push   %r12
    8a1c:	53                   	push   %rbx
    8a1d:	48 81 ec 88 00 00 00 	sub    $0x88,%rsp
    8a24:	e8 00 00 00 00       	callq  8a29 <__scif_fence_wait+0x19>
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;
    8a29:	41 bc 98 ff ff ff    	mov    $0xffffff98,%r12d
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;

	pr_debug("SCIFAPI fence_wait: ep %p %s mark 0x%x\n", 
    8a2f:	8b 07                	mov    (%rdi),%eax
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    8a31:	8b 07                	mov    (%rdi),%eax
    8a33:	83 f8 09             	cmp    $0x9,%eax
    8a36:	74 0a                	je     8a42 <__scif_fence_wait+0x32>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8a38:	8b 07                	mov    (%rdi),%eax
		return -ENOTCONN;
    8a3a:	41 b4 95             	mov    $0x95,%r12b
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8a3d:	83 f8 04             	cmp    $0x4,%eax
    8a40:	74 1e                	je     8a60 <__scif_fence_wait+0x50>
	}

	if (err < 0)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	return err;
}
    8a42:	48 81 c4 88 00 00 00 	add    $0x88,%rsp
    8a49:	44 89 e0             	mov    %r12d,%eax
    8a4c:	5b                   	pop    %rbx
    8a4d:	41 5c                	pop    %r12
    8a4f:	41 5d                	pop    %r13
    8a51:	41 5e                	pop    %r14
    8a53:	41 5f                	pop    %r15
    8a55:	5d                   	pop    %rbp
    8a56:	c3                   	retq   
    8a57:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    8a5e:	00 00 
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    8a60:	4c 8b b7 48 01 00 00 	mov    0x148(%rdi),%r14
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    8a67:	41 b4 ed             	mov    $0xed,%r12b
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    8a6a:	41 8b 46 04          	mov    0x4(%r14),%eax
    8a6e:	83 e8 02             	sub    $0x2,%eax
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    8a71:	83 f8 01             	cmp    $0x1,%eax
    8a74:	77 cc                	ja     8a42 <__scif_fence_wait+0x32>
    8a76:	83 bf 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rdi)
    8a7d:	75 c3                	jne    8a42 <__scif_fence_wait+0x32>
 *
 * Returns true if the SCIF Device passed is the self aka Loopback SCIF device.
 */
static inline int is_self_scifdev(struct micscif_dev *dev)
{
	return dev->sd_node == ms_info.mi_nodeid;
    8a7f:	41 0f b7 06          	movzwl (%r14),%eax
	/*
	 * Host Loopback does not need to use DMA.
	 * The only valid mark provided is 0 so simply
	 * return success if the mark is valid.
	 */
	if (is_self_scifdev(ep->remote_dev)) {
    8a83:	3b 05 00 00 00 00    	cmp    0x0(%rip),%eax        # 8a89 <__scif_fence_wait+0x79>
    8a89:	74 67                	je     8af2 <__scif_fence_wait+0xe2>
			return 0;
		else
			return -EINVAL;
	}
#endif
	if (mark & SCIF_REMOTE_FENCE) {
    8a8b:	41 89 f4             	mov    %esi,%r12d
    8a8e:	41 89 f5             	mov    %esi,%r13d
    8a91:	48 89 fb             	mov    %rdi,%rbx
    8a94:	41 81 e4 00 00 00 40 	and    $0x40000000,%r12d
    8a9b:	74 73                	je     8b10 <__scif_fence_wait+0x100>
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8a9d:	4d 85 f6             	test   %r14,%r14
    8aa0:	0f 85 aa 00 00 00    	jne    8b50 <__scif_fence_wait+0x140>
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_wait(epd, mark);
    8aa6:	44 89 ee             	mov    %r13d,%esi
    8aa9:	48 89 df             	mov    %rbx,%rdi
    8aac:	e8 00 00 00 00       	callq  8ab1 <__scif_fence_wait+0xa1>
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    8ab1:	48 8b 9b 48 01 00 00 	mov    0x148(%rbx),%rbx
			return -EINVAL;
	}
#endif
	if (mark & SCIF_REMOTE_FENCE) {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_wait(epd, mark);
    8ab8:	41 89 c5             	mov    %eax,%r13d
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    8abb:	48 85 db             	test   %rbx,%rbx
    8abe:	74 21                	je     8ae1 <__scif_fence_wait+0xd1>
		if (unlikely((atomic_long_sub_return(cnt, 
    8ac0:	4c 8d a3 88 01 00 00 	lea    0x188(%rbx),%r12
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    8ac7:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    8ace:	f0 48 0f c1 93 88 01 	lock xadd %rdx,0x188(%rbx)
    8ad5:	00 00 
    8ad7:	48 83 ea 01          	sub    $0x1,%rdx
    8adb:	0f 88 cf 00 00 00    	js     8bb0 <__scif_fence_wait+0x1a0>
		err = dma_mark_wait(epd->rma_info.dma_chan, mark, true);
		if (!err && atomic_read(&ep->rma_info.tw_refcount))
			queue_work(ms_info.mi_misc_wq, &ms_info.mi_misc_work);
	}

	if (err < 0)
    8ae1:	45 85 ed             	test   %r13d,%r13d
    8ae4:	0f 88 fa 01 00 00    	js     8ce4 <__scif_fence_wait+0x2d4>
    8aea:	45 89 ec             	mov    %r13d,%r12d
    8aed:	e9 50 ff ff ff       	jmpq   8a42 <__scif_fence_wait+0x32>
	 */
	if (is_self_scifdev(ep->remote_dev)) {
		if (HOST_LOOPB_MAGIC_MARK == mark)
			return 0;
		else
			return -EINVAL;
    8af2:	81 fe ad de 00 00    	cmp    $0xdead,%esi
    8af8:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    8afd:	41 bc 00 00 00 00    	mov    $0x0,%r12d
    8b03:	44 0f 45 e0          	cmovne %eax,%r12d
    8b07:	e9 36 ff ff ff       	jmpq   8a42 <__scif_fence_wait+0x32>
    8b0c:	0f 1f 40 00          	nopl   0x0(%rax)
	if (mark & SCIF_REMOTE_FENCE) {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_wait(epd, mark);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	} else {
		err = dma_mark_wait(epd->rma_info.dma_chan, mark, true);
    8b10:	48 8b bf 28 01 00 00 	mov    0x128(%rdi),%rdi
    8b17:	ba 01 00 00 00       	mov    $0x1,%edx
    8b1c:	e8 00 00 00 00       	callq  8b21 <__scif_fence_wait+0x111>
		if (!err && atomic_read(&ep->rma_info.tw_refcount))
    8b21:	85 c0                	test   %eax,%eax
	if (mark & SCIF_REMOTE_FENCE) {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_wait(epd, mark);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	} else {
		err = dma_mark_wait(epd->rma_info.dma_chan, mark, true);
    8b23:	41 89 c5             	mov    %eax,%r13d
		if (!err && atomic_read(&ep->rma_info.tw_refcount))
    8b26:	75 b9                	jne    8ae1 <__scif_fence_wait+0xd1>
    8b28:	8b 83 e8 00 00 00    	mov    0xe8(%rbx),%eax
    8b2e:	85 c0                	test   %eax,%eax
    8b30:	0f 84 0c ff ff ff    	je     8a42 <__scif_fence_wait+0x32>
			queue_work(ms_info.mi_misc_wq, &ms_info.mi_misc_work);
    8b36:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 8b3d <__scif_fence_wait+0x12d>
    8b3d:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    8b44:	e8 00 00 00 00       	callq  8b49 <__scif_fence_wait+0x139>
    8b49:	e9 f4 fe ff ff       	jmpq   8a42 <__scif_fence_wait+0x32>
    8b4e:	66 90                	xchg   %ax,%ax
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    8b50:	49 8b 8e 88 01 00 00 	mov    0x188(%r14),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8b57:	4d 8d be 88 01 00 00 	lea    0x188(%r14),%r15
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8b5e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8b65:	00 00 80 
    8b68:	48 39 f1             	cmp    %rsi,%rcx
    8b6b:	0f 84 cf 00 00 00    	je     8c40 <__scif_fence_wait+0x230>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8b71:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8b75:	48 89 c8             	mov    %rcx,%rax
    8b78:	f0 49 0f b1 96 88 01 	lock cmpxchg %rdx,0x188(%r14)
    8b7f:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8b81:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8b84:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8b87:	0f 84 19 ff ff ff    	je     8aa6 <__scif_fence_wait+0x96>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8b8d:	48 39 f2             	cmp    %rsi,%rdx
    8b90:	0f 84 aa 00 00 00    	je     8c40 <__scif_fence_wait+0x230>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8b96:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8b9a:	48 89 d0             	mov    %rdx,%rax
    8b9d:	f0 49 0f b1 0f       	lock cmpxchg %rcx,(%r15)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8ba2:	48 39 c2             	cmp    %rax,%rdx
    8ba5:	0f 84 fb fe ff ff    	je     8aa6 <__scif_fence_wait+0x96>
    8bab:	48 89 c2             	mov    %rax,%rdx
    8bae:	eb dd                	jmp    8b8d <__scif_fence_wait+0x17d>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    8bb0:	48 8b 45 08          	mov    0x8(%rbp),%rax
    8bb4:	48 89 d9             	mov    %rbx,%rcx
    8bb7:	ba a7 00 00 00       	mov    $0xa7,%edx
    8bbc:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    8bc3:	4c 8b 8b 88 01 00 00 	mov    0x188(%rbx),%r9
    8bca:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8bd1:	44 0f b7 03          	movzwl (%rbx),%r8d
    8bd5:	48 89 04 24          	mov    %rax,(%rsp)
    8bd9:	31 c0                	xor    %eax,%eax
    8bdb:	e8 00 00 00 00       	callq  8be0 <__scif_fence_wait+0x1d0>
    8be0:	48 8b 8b 88 01 00 00 	mov    0x188(%rbx),%rcx
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8be7:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    8bee:	00 00 80 
    8bf1:	48 39 f1             	cmp    %rsi,%rcx
    8bf4:	0f 84 e7 fe ff ff    	je     8ae1 <__scif_fence_wait+0xd1>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8bfa:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8bfe:	48 89 c8             	mov    %rcx,%rax
    8c01:	f0 48 0f b1 93 88 01 	lock cmpxchg %rdx,0x188(%rbx)
    8c08:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8c0a:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8c0d:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8c10:	0f 84 cb fe ff ff    	je     8ae1 <__scif_fence_wait+0xd1>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8c16:	48 39 f2             	cmp    %rsi,%rdx
    8c19:	0f 84 c2 fe ff ff    	je     8ae1 <__scif_fence_wait+0xd1>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    8c1f:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    8c23:	48 89 d0             	mov    %rdx,%rax
    8c26:	f0 49 0f b1 0c 24    	lock cmpxchg %rcx,(%r12)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    8c2c:	48 39 c2             	cmp    %rax,%rdx
    8c2f:	0f 84 ac fe ff ff    	je     8ae1 <__scif_fence_wait+0xd1>
    8c35:	48 89 c2             	mov    %rax,%rdx
    8c38:	eb dc                	jmp    8c16 <__scif_fence_wait+0x206>
    8c3a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    8c40:	4d 8d a6 68 01 00 00 	lea    0x168(%r14),%r12
    8c47:	4c 89 e7             	mov    %r12,%rdi
    8c4a:	e8 00 00 00 00       	callq  8c4f <__scif_fence_wait+0x23f>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    8c4f:	41 8b 46 04          	mov    0x4(%r14),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    8c53:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    8c56:	83 fa 01             	cmp    $0x1,%edx
    8c59:	76 19                	jbe    8c74 <__scif_fence_wait+0x264>
    8c5b:	83 f8 01             	cmp    $0x1,%eax
    8c5e:	74 14                	je     8c74 <__scif_fence_wait+0x264>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    8c60:	49 8b 86 88 01 00 00 	mov    0x188(%r14),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    8c67:	48 85 c0             	test   %rax,%rax
    8c6a:	78 15                	js     8c81 <__scif_fence_wait+0x271>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    8c6c:	4c 89 ff             	mov    %r15,%rdi
    8c6f:	e8 7c 78 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    8c74:	4c 89 e7             	mov    %r12,%rdi
    8c77:	e8 00 00 00 00       	callq  8c7c <__scif_fence_wait+0x26c>
    8c7c:	e9 25 fe ff ff       	jmpq   8aa6 <__scif_fence_wait+0x96>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    8c81:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 8c87 <__scif_fence_wait+0x277>
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    8c87:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    8c8e:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
    8c92:	31 d2                	xor    %edx,%edx
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    8c94:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    8c9a:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    8ca1:	49 c7 86 b0 01 00 00 	movq   $0x2,0x1b0(%r14)
    8ca8:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    8cac:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    8cb0:	41 0f b7 06          	movzwl (%r14),%eax
    8cb4:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    8cb8:	e8 00 00 00 00       	callq  8cbd <__scif_fence_wait+0x2ad>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    8cbd:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
    8cc4:	02 
    8cc5:	74 42                	je     8d09 <__scif_fence_wait+0x2f9>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    8cc7:	49 83 be b0 01 00 00 	cmpq   $0x3,0x1b0(%r14)
    8cce:	03 
    8ccf:	75 9b                	jne    8c6c <__scif_fence_wait+0x25c>
				dev->sd_state = SCIFDEV_RUNNING;
    8cd1:	41 c7 46 04 02 00 00 	movl   $0x2,0x4(%r14)
    8cd8:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    8cd9:	f0 41 80 a6 8f 01 00 	lock andb $0x7f,0x18f(%r14)
    8ce0:	00 7f 
    8ce2:	eb 88                	jmp    8c6c <__scif_fence_wait+0x25c>
	}

	if (err < 0)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    8ce4:	44 89 e9             	mov    %r13d,%ecx
    8ce7:	ba 14 0c 00 00       	mov    $0xc14,%edx
    8cec:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    8cf3:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8cfa:	31 c0                	xor    %eax,%eax
    8cfc:	45 89 ec             	mov    %r13d,%r12d
    8cff:	e8 00 00 00 00       	callq  8d04 <__scif_fence_wait+0x2f4>
    8d04:	e9 39 fd ff ff       	jmpq   8a42 <__scif_fence_wait+0x32>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    8d09:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    8d10:	31 c0                	xor    %eax,%eax
    8d12:	b9 0a 00 00 00       	mov    $0xa,%ecx
    8d17:	f3 ab                	rep stos %eax,%es:(%rdi)
    8d19:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    8d20:	00 
    8d21:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    8d28:	00 00 
    8d2a:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    8d2e:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    8d35:	48 83 c0 18          	add    $0x18,%rax
    8d39:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    8d3d:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    8d41:	49 8d 86 98 01 00 00 	lea    0x198(%r14),%rax
    8d48:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    8d4f:	eb 0c                	jmp    8d5d <__scif_fence_wait+0x34d>
    8d51:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    8d58:	e8 00 00 00 00       	callq  8d5d <__scif_fence_wait+0x34d>
    8d5d:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    8d64:	ba 02 00 00 00       	mov    $0x2,%edx
    8d69:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    8d70:	e8 00 00 00 00       	callq  8d75 <__scif_fence_wait+0x365>
    8d75:	49 83 be b0 01 00 00 	cmpq   $0x2,0x1b0(%r14)
    8d7c:	02 
    8d7d:	74 d9                	je     8d58 <__scif_fence_wait+0x348>
    8d7f:	48 8b bd 68 ff ff ff 	mov    -0x98(%rbp),%rdi
    8d86:	48 8d b5 78 ff ff ff 	lea    -0x88(%rbp),%rsi
    8d8d:	e8 00 00 00 00       	callq  8d92 <__scif_fence_wait+0x382>
    8d92:	e9 30 ff ff ff       	jmpq   8cc7 <__scif_fence_wait+0x2b7>
    8d97:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    8d9e:	00 00 

0000000000008da0 <scif_fence_wait>:
	return err;
}

int scif_fence_wait(scif_epd_t epd, int mark)
{
    8da0:	55                   	push   %rbp
    8da1:	48 89 e5             	mov    %rsp,%rbp
    8da4:	41 55                	push   %r13
    8da6:	41 54                	push   %r12
    8da8:	53                   	push   %rbx
    8da9:	48 83 ec 08          	sub    $0x8,%rsp
    8dad:	e8 00 00 00 00       	callq  8db2 <scif_fence_wait+0x12>
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    8db2:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    8db9:	49 89 fc             	mov    %rdi,%r12
    8dbc:	41 89 f5             	mov    %esi,%r13d
    8dbf:	48 89 df             	mov    %rbx,%rdi
    8dc2:	e8 00 00 00 00       	callq  8dc7 <scif_fence_wait+0x27>
	int ret;
	get_kref_count(epd);
	ret = __scif_fence_wait(epd, mark);
    8dc7:	44 89 ee             	mov    %r13d,%esi
    8dca:	4c 89 e7             	mov    %r12,%rdi
    8dcd:	e8 00 00 00 00       	callq  8dd2 <scif_fence_wait+0x32>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    8dd2:	48 89 df             	mov    %rbx,%rdi
    8dd5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    8ddc:	41 89 c4             	mov    %eax,%r12d
    8ddf:	e8 00 00 00 00       	callq  8de4 <scif_fence_wait+0x44>
	put_kref_count(epd);
	return ret;
}
    8de4:	48 83 c4 08          	add    $0x8,%rsp
    8de8:	44 89 e0             	mov    %r12d,%eax
    8deb:	5b                   	pop    %rbx
    8dec:	41 5c                	pop    %r12
    8dee:	41 5d                	pop    %r13
    8df0:	5d                   	pop    %rbp
    8df1:	c3                   	retq   
    8df2:	66 66 66 66 66 2e 0f 	data32 data32 data32 data32 nopw %cs:0x0(%rax,%rax,1)
    8df9:	1f 84 00 00 00 00 00 

0000000000008e00 <__scif_fence_signal>:
 * 	Upon successful completion, scif_fence_signal() returns 0;
 *	else an apt error is returned as documented in scif.h.
 */
int __scif_fence_signal(scif_epd_t epd, off_t loff, uint64_t lval,
				off_t roff, uint64_t rval, int flags)
{
    8e00:	55                   	push   %rbp
    8e01:	48 89 e5             	mov    %rsp,%rbp
    8e04:	41 57                	push   %r15
    8e06:	41 56                	push   %r14
    8e08:	41 55                	push   %r13
    8e0a:	41 54                	push   %r12
    8e0c:	53                   	push   %rbx
    8e0d:	48 81 ec a8 00 00 00 	sub    $0xa8,%rsp
    8e14:	e8 00 00 00 00       	callq  8e19 <__scif_fence_signal+0x19>
	struct endpt *ep = (struct endpt *)epd;
	int err = 0;

	pr_debug("SCIFAPI fence_signal: ep %p %s loff 0x%lx lval 0x%llx "
    8e19:	8b 07                	mov    (%rdi),%eax
 * Checks several generic error conditions and returns the
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
    8e1b:	8b 07                	mov    (%rdi),%eax
    8e1d:	83 f8 09             	cmp    $0x9,%eax
    8e20:	74 26                	je     8e48 <__scif_fence_signal+0x48>
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8e22:	44 8b 17             	mov    (%rdi),%r10d
		return -ENOTCONN;
    8e25:	b8 95 ff ff ff       	mov    $0xffffff95,%eax
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;

	if (ep->state != SCIFEP_CONNECTED)
    8e2a:	41 83 fa 04          	cmp    $0x4,%r10d
    8e2e:	74 20                	je     8e50 <__scif_fence_signal+0x50>
	if (err)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	else if (atomic_read(&ep->rma_info.tw_refcount))
		queue_work(ms_info.mi_misc_wq, &ms_info.mi_misc_work);
	return err;
}
    8e30:	48 81 c4 a8 00 00 00 	add    $0xa8,%rsp
    8e37:	5b                   	pop    %rbx
    8e38:	41 5c                	pop    %r12
    8e3a:	41 5d                	pop    %r13
    8e3c:	41 5e                	pop    %r14
    8e3e:	41 5f                	pop    %r15
    8e40:	5d                   	pop    %rbp
    8e41:	c3                   	retq   
    8e42:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * appropiate error.
 */
static inline int verify_epd(struct endpt *ep)
{
	if (ep->state == SCIFEP_DISCONNECTED)
		return -ECONNRESET;
    8e48:	b8 98 ff ff ff       	mov    $0xffffff98,%eax
    8e4d:	eb e1                	jmp    8e30 <__scif_fence_signal+0x30>
    8e4f:	90                   	nop
 * Returns true if the remote SCIF Device is running or sleeping for
 * this endpoint.
 */
static inline int scifdev_alive(struct endpt *ep)
{
	return (((SCIFDEV_RUNNING == ep->remote_dev->sd_state) ||
    8e50:	4c 8b 97 48 01 00 00 	mov    0x148(%rdi),%r10
    8e57:	41 8b 42 04          	mov    0x4(%r10),%eax
    8e5b:	44 8d 58 fe          	lea    -0x2(%rax),%r11d
		(SCIFDEV_SLEEPING == ep->remote_dev->sd_state)) &&
    8e5f:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
    8e64:	41 83 fb 01          	cmp    $0x1,%r11d
    8e68:	77 c6                	ja     8e30 <__scif_fence_signal+0x30>
    8e6a:	83 bf 5c 01 00 00 02 	cmpl   $0x2,0x15c(%rdi)
    8e71:	75 bd                	jne    8e30 <__scif_fence_signal+0x30>

	if ((err = verify_epd(ep)))
		return err;

	/* Invalid flags? */
	if (flags & ~(SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER |
    8e73:	41 f7 c1 cc ff ff ff 	test   $0xffffffcc,%r9d
    8e7a:	0f 85 d0 01 00 00    	jne    9050 <__scif_fence_signal+0x250>
			SCIF_SIGNAL_LOCAL | SCIF_SIGNAL_REMOTE))
		return -EINVAL;

	/* At least one of init self or peer RMA should be set */
	if (!(flags & (SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER)))
    8e80:	44 89 c8             	mov    %r9d,%eax
    8e83:	83 e0 03             	and    $0x3,%eax
    8e86:	0f 84 c4 01 00 00    	je     9050 <__scif_fence_signal+0x250>
		return -EINVAL;

	/* Exactly one of init self or peer RMA should be set but not both */
	if ((flags & SCIF_FENCE_INIT_SELF) && (flags & SCIF_FENCE_INIT_PEER))
    8e8c:	83 f8 03             	cmp    $0x3,%eax
    8e8f:	0f 84 bb 01 00 00    	je     9050 <__scif_fence_signal+0x250>
		return -EINVAL;

	/* At least one of SCIF_SIGNAL_LOCAL or SCIF_SIGNAL_REMOTE required */
	if (!(flags & (SCIF_SIGNAL_LOCAL | SCIF_SIGNAL_REMOTE)))
    8e95:	41 f6 c1 30          	test   $0x30,%r9b
    8e99:	0f 84 b1 01 00 00    	je     9050 <__scif_fence_signal+0x250>
		return -EINVAL;

	/* Only Dword offsets allowed */
	if ((flags & SCIF_SIGNAL_LOCAL) && (loff & (sizeof(uint32_t) - 1)))
    8e9f:	44 89 c8             	mov    %r9d,%eax
    8ea2:	83 e0 10             	and    $0x10,%eax
    8ea5:	74 0a                	je     8eb1 <__scif_fence_signal+0xb1>
    8ea7:	40 f6 c6 03          	test   $0x3,%sil
    8eab:	0f 85 9f 01 00 00    	jne    9050 <__scif_fence_signal+0x250>
		return -EINVAL;

	/* Only Dword aligned offsets allowed */
	if ((flags & SCIF_SIGNAL_REMOTE) && (roff & (sizeof(uint32_t) - 1)))
    8eb1:	45 89 ce             	mov    %r9d,%r14d
    8eb4:	41 83 e6 20          	and    $0x20,%r14d
    8eb8:	74 09                	je     8ec3 <__scif_fence_signal+0xc3>
    8eba:	f6 c1 03             	test   $0x3,%cl
    8ebd:	0f 85 8d 01 00 00    	jne    9050 <__scif_fence_signal+0x250>
		return -EINVAL;

	if (flags & SCIF_FENCE_INIT_PEER) {
    8ec3:	41 f6 c1 02          	test   $0x2,%r9b
    8ec7:	4d 89 c7             	mov    %r8,%r15
    8eca:	49 89 cc             	mov    %rcx,%r12
    8ecd:	49 89 d0             	mov    %rdx,%r8
    8ed0:	49 89 f5             	mov    %rsi,%r13
    8ed3:	48 89 fb             	mov    %rdi,%rbx
    8ed6:	0f 84 84 00 00 00    	je     8f60 <__scif_fence_signal+0x160>
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8edc:	4d 85 d2             	test   %r10,%r10
    8edf:	0f 85 bb 01 00 00    	jne    90a0 <__scif_fence_signal+0x2a0>
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_signal(epd, roff,
    8ee5:	4c 89 e9             	mov    %r13,%rcx
    8ee8:	4c 89 e6             	mov    %r12,%rsi
    8eeb:	4c 89 fa             	mov    %r15,%rdx
    8eee:	48 89 df             	mov    %rbx,%rdi
    8ef1:	e8 00 00 00 00       	callq  8ef6 <__scif_fence_signal+0xf6>
			rval, loff, lval, flags);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
    8ef6:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
	if ((flags & SCIF_SIGNAL_REMOTE) && (roff & (sizeof(uint32_t) - 1)))
		return -EINVAL;

	if (flags & SCIF_FENCE_INIT_PEER) {
		micscif_inc_node_refcnt(ep->remote_dev, 1);
		err = micscif_send_fence_signal(epd, roff,
    8efd:	41 89 c5             	mov    %eax,%r13d
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    8f00:	4d 85 e4             	test   %r12,%r12
    8f03:	74 23                	je     8f28 <__scif_fence_signal+0x128>
		if (unlikely((atomic_long_sub_return(cnt, 
    8f05:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    8f0c:	00 
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
    8f0d:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    8f14:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    8f1b:	01 00 00 
    8f1e:	48 83 e8 01          	sub    $0x1,%rax
    8f22:	0f 88 d8 01 00 00    	js     9100 <__scif_fence_signal+0x300>
					rval, RMA_WINDOW_PEER);
			micscif_dec_node_refcnt(ep->remote_dev, 1);
		}
	}
error_ret:
	if (err)
    8f28:	45 85 ed             	test   %r13d,%r13d
    8f2b:	0f 85 44 01 00 00    	jne    9075 <__scif_fence_signal+0x275>
    8f31:	8b 83 e8 00 00 00    	mov    0xe8(%rbx),%eax
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
	else if (atomic_read(&ep->rma_info.tw_refcount))
    8f37:	85 c0                	test   %eax,%eax
    8f39:	0f 84 f1 fe ff ff    	je     8e30 <__scif_fence_signal+0x30>
		queue_work(ms_info.mi_misc_wq, &ms_info.mi_misc_work);
    8f3f:	48 8b 3d 00 00 00 00 	mov    0x0(%rip),%rdi        # 8f46 <__scif_fence_signal+0x146>
    8f46:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    8f4d:	e8 00 00 00 00       	callq  8f52 <__scif_fence_signal+0x152>
    8f52:	31 c0                	xor    %eax,%eax
    8f54:	e9 d7 fe ff ff       	jmpq   8e30 <__scif_fence_signal+0x30>
    8f59:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
		err = micscif_send_fence_signal(epd, roff,
			rval, loff, lval, flags);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	} else {
		/* Local Signal in Local RAS */
		if (flags & SCIF_SIGNAL_LOCAL)
    8f60:	85 c0                	test   %eax,%eax
    8f62:	0f 85 f8 00 00 00    	jne    9060 <__scif_fence_signal+0x260>
			if ((err = micscif_prog_signal(epd, loff,
					lval, RMA_WINDOW_SELF)))
				goto error_ret;

		/* Signal in Remote RAS */
		if (flags & SCIF_SIGNAL_REMOTE) {
    8f68:	45 85 f6             	test   %r14d,%r14d
    8f6b:	74 c4                	je     8f31 <__scif_fence_signal+0x131>
			micscif_inc_node_refcnt(ep->remote_dev, 1);
    8f6d:	4c 8b ab 48 01 00 00 	mov    0x148(%rbx),%r13
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    8f74:	4d 85 ed             	test   %r13,%r13
    8f77:	0f 85 13 02 00 00    	jne    9190 <__scif_fence_signal+0x390>
			err = micscif_prog_signal(epd, roff,
    8f7d:	4c 89 e6             	mov    %r12,%rsi
    8f80:	b9 02 00 00 00       	mov    $0x2,%ecx
    8f85:	4c 89 fa             	mov    %r15,%rdx
    8f88:	48 89 df             	mov    %rbx,%rdi
    8f8b:	e8 00 00 00 00       	callq  8f90 <__scif_fence_signal+0x190>
					rval, RMA_WINDOW_PEER);
			micscif_dec_node_refcnt(ep->remote_dev, 1);
    8f90:	4c 8b a3 48 01 00 00 	mov    0x148(%rbx),%r12
				goto error_ret;

		/* Signal in Remote RAS */
		if (flags & SCIF_SIGNAL_REMOTE) {
			micscif_inc_node_refcnt(ep->remote_dev, 1);
			err = micscif_prog_signal(epd, roff,
    8f97:	41 89 c5             	mov    %eax,%r13d
 */
static __always_inline void
micscif_dec_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
    8f9a:	4d 85 e4             	test   %r12,%r12
    8f9d:	74 89                	je     8f28 <__scif_fence_signal+0x128>
		if (unlikely((atomic_long_sub_return(cnt, 
    8f9f:	4d 8d b4 24 88 01 00 	lea    0x188(%r12),%r14
    8fa6:	00 
    8fa7:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    8fae:	f0 49 0f c1 84 24 88 	lock xadd %rax,0x188(%r12)
    8fb5:	01 00 00 
    8fb8:	48 83 e8 01          	sub    $0x1,%rax
    8fbc:	0f 89 66 ff ff ff    	jns    8f28 <__scif_fence_signal+0x128>
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    8fc2:	48 8b 45 08          	mov    0x8(%rbp),%rax
    8fc6:	4c 89 e1             	mov    %r12,%rcx
    8fc9:	ba a7 00 00 00       	mov    $0xa7,%edx
    8fce:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    8fd5:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    8fdc:	00 
    8fdd:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    8fe4:	45 0f b7 04 24       	movzwl (%r12),%r8d
    8fe9:	48 89 04 24          	mov    %rax,(%rsp)
    8fed:	31 c0                	xor    %eax,%eax
    8fef:	e8 00 00 00 00       	callq  8ff4 <__scif_fence_signal+0x1f4>
    8ff4:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    8ffb:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    8ffc:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    9003:	00 00 80 
    9006:	48 39 f1             	cmp    %rsi,%rcx
    9009:	0f 84 19 ff ff ff    	je     8f28 <__scif_fence_signal+0x128>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    900f:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9013:	48 89 c8             	mov    %rcx,%rax
    9016:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    901d:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    9020:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9023:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    9026:	0f 84 fc fe ff ff    	je     8f28 <__scif_fence_signal+0x128>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    902c:	48 39 f2             	cmp    %rsi,%rdx
    902f:	0f 84 f3 fe ff ff    	je     8f28 <__scif_fence_signal+0x128>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    9035:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9039:	48 89 d0             	mov    %rdx,%rax
    903c:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    9041:	48 39 c2             	cmp    %rax,%rdx
    9044:	0f 84 de fe ff ff    	je     8f28 <__scif_fence_signal+0x128>
    904a:	48 89 c2             	mov    %rax,%rdx
    904d:	eb dd                	jmp    902c <__scif_fence_signal+0x22c>
    904f:	90                   	nop
			SCIF_SIGNAL_LOCAL | SCIF_SIGNAL_REMOTE))
		return -EINVAL;

	/* At least one of init self or peer RMA should be set */
	if (!(flags & (SCIF_FENCE_INIT_SELF | SCIF_FENCE_INIT_PEER)))
		return -EINVAL;
    9050:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
    9055:	e9 d6 fd ff ff       	jmpq   8e30 <__scif_fence_signal+0x30>
    905a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
			rval, loff, lval, flags);
		micscif_dec_node_refcnt(ep->remote_dev, 1);
	} else {
		/* Local Signal in Local RAS */
		if (flags & SCIF_SIGNAL_LOCAL)
			if ((err = micscif_prog_signal(epd, loff,
    9060:	b9 01 00 00 00       	mov    $0x1,%ecx
    9065:	e8 00 00 00 00       	callq  906a <__scif_fence_signal+0x26a>
    906a:	85 c0                	test   %eax,%eax
    906c:	41 89 c5             	mov    %eax,%r13d
    906f:	0f 84 f3 fe ff ff    	je     8f68 <__scif_fence_signal+0x168>
			micscif_dec_node_refcnt(ep->remote_dev, 1);
		}
	}
error_ret:
	if (err)
		printk(KERN_ERR "%s %d err %d\n", __func__, __LINE__, err);
    9075:	44 89 e9             	mov    %r13d,%ecx
    9078:	ba 6f 0c 00 00       	mov    $0xc6f,%edx
    907d:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    9084:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    908b:	31 c0                	xor    %eax,%eax
    908d:	e8 00 00 00 00       	callq  9092 <__scif_fence_signal+0x292>
    9092:	44 89 e8             	mov    %r13d,%eax
    9095:	e9 96 fd ff ff       	jmpq   8e30 <__scif_fence_signal+0x30>
    909a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    90a0:	49 8b 92 88 01 00 00 	mov    0x188(%r10),%rdx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    90a7:	4d 8d b2 88 01 00 00 	lea    0x188(%r10),%r14
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    90ae:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    90b5:	00 00 80 
    90b8:	48 39 f2             	cmp    %rsi,%rdx
    90bb:	0f 84 2f 01 00 00    	je     91f0 <__scif_fence_signal+0x3f0>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    90c1:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    90c5:	48 89 d0             	mov    %rdx,%rax
    90c8:	f0 49 0f b1 8a 88 01 	lock cmpxchg %rcx,0x188(%r10)
    90cf:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    90d1:	48 39 d0             	cmp    %rdx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    90d4:	48 89 c1             	mov    %rax,%rcx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    90d7:	0f 84 08 fe ff ff    	je     8ee5 <__scif_fence_signal+0xe5>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    90dd:	48 39 f1             	cmp    %rsi,%rcx
    90e0:	0f 84 0a 01 00 00    	je     91f0 <__scif_fence_signal+0x3f0>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    90e6:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    90ea:	48 89 c8             	mov    %rcx,%rax
    90ed:	f0 49 0f b1 16       	lock cmpxchg %rdx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    90f2:	48 39 c8             	cmp    %rcx,%rax
    90f5:	0f 84 ea fd ff ff    	je     8ee5 <__scif_fence_signal+0xe5>
    90fb:	48 89 c1             	mov    %rax,%rcx
    90fe:	eb dd                	jmp    90dd <__scif_fence_signal+0x2dd>
{
#ifdef SCIF_ENABLE_PM
	if (dev) {
		if (unlikely((atomic_long_sub_return(cnt, 
			&dev->scif_ref_cnt)) < 0)) {
			printk(KERN_ERR "%s %d dec dev %p node %d ref %ld "
    9100:	48 8b 45 08          	mov    0x8(%rbp),%rax
    9104:	4c 89 e1             	mov    %r12,%rcx
    9107:	ba a7 00 00 00       	mov    $0xa7,%edx
    910c:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    9113:	4d 8b 8c 24 88 01 00 	mov    0x188(%r12),%r9
    911a:	00 
    911b:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    9122:	45 0f b7 04 24       	movzwl (%r12),%r8d
    9127:	48 89 04 24          	mov    %rax,(%rsp)
    912b:	31 c0                	xor    %eax,%eax
    912d:	e8 00 00 00 00       	callq  9132 <__scif_fence_signal+0x332>
    9132:	49 8b 8c 24 88 01 00 	mov    0x188(%r12),%rcx
    9139:	00 
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    913a:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    9141:	00 00 80 
    9144:	48 39 f1             	cmp    %rsi,%rcx
    9147:	0f 84 db fd ff ff    	je     8f28 <__scif_fence_signal+0x128>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    914d:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9151:	48 89 c8             	mov    %rcx,%rax
    9154:	f0 49 0f b1 94 24 88 	lock cmpxchg %rdx,0x188(%r12)
    915b:	01 00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    915e:	48 39 c1             	cmp    %rax,%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9161:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    9164:	0f 84 be fd ff ff    	je     8f28 <__scif_fence_signal+0x128>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    916a:	48 39 f2             	cmp    %rsi,%rdx
    916d:	0f 84 b5 fd ff ff    	je     8f28 <__scif_fence_signal+0x128>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    9173:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    9177:	48 89 d0             	mov    %rdx,%rax
    917a:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    917f:	48 39 d0             	cmp    %rdx,%rax
    9182:	0f 84 a0 fd ff ff    	je     8f28 <__scif_fence_signal+0x128>
    9188:	48 89 c2             	mov    %rax,%rdx
    918b:	eb dd                	jmp    916a <__scif_fence_signal+0x36a>
    918d:	0f 1f 00             	nopl   (%rax)
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
    9190:	49 8b 8d 88 01 00 00 	mov    0x188(%r13),%rcx
 */
static __always_inline void
micscif_inc_node_refcnt(struct micscif_dev *dev, long cnt)
{
#ifdef SCIF_ENABLE_PM
	if (unlikely(dev && !atomic_long_add_unless(&dev->scif_ref_cnt, 
    9197:	4d 8d b5 88 01 00 00 	lea    0x188(%r13),%r14
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    919e:	48 be 00 00 00 00 00 	movabs $0x8000000000000000,%rsi
    91a5:	00 00 80 
    91a8:	48 39 f1             	cmp    %rsi,%rcx
    91ab:	0f 84 f1 00 00 00    	je     92a2 <__scif_fence_signal+0x4a2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    91b1:	48 8d 51 01          	lea    0x1(%rcx),%rdx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    91b5:	48 89 c8             	mov    %rcx,%rax
    91b8:	f0 49 0f b1 95 88 01 	lock cmpxchg %rdx,0x188(%r13)
    91bf:	00 00 
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    91c1:	48 39 c8             	cmp    %rcx,%rax
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    91c4:	48 89 c2             	mov    %rax,%rdx
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    91c7:	0f 84 b0 fd ff ff    	je     8f7d <__scif_fence_signal+0x17d>
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
    91cd:	48 39 f2             	cmp    %rsi,%rdx
    91d0:	0f 84 cc 00 00 00    	je     92a2 <__scif_fence_signal+0x4a2>
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
    91d6:	48 8d 4a 01          	lea    0x1(%rdx),%rcx
#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
    91da:	48 89 d0             	mov    %rdx,%rax
    91dd:	f0 49 0f b1 0e       	lock cmpxchg %rcx,(%r14)
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
    91e2:	48 39 c2             	cmp    %rax,%rdx
    91e5:	0f 84 92 fd ff ff    	je     8f7d <__scif_fence_signal+0x17d>
    91eb:	48 89 c2             	mov    %rax,%rdx
    91ee:	eb dd                	jmp    91cd <__scif_fence_signal+0x3cd>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    91f0:	49 8d 82 68 01 00 00 	lea    0x168(%r10),%rax
    91f7:	44 89 8d 50 ff ff ff 	mov    %r9d,-0xb0(%rbp)
    91fe:	48 89 c7             	mov    %rax,%rdi
    9201:	4c 89 85 58 ff ff ff 	mov    %r8,-0xa8(%rbp)
    9208:	4c 89 95 68 ff ff ff 	mov    %r10,-0x98(%rbp)
    920f:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    9216:	e8 00 00 00 00       	callq  921b <__scif_fence_signal+0x41b>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    921b:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
    9222:	4c 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%r8
    9229:	44 8b 8d 50 ff ff ff 	mov    -0xb0(%rbp),%r9d
    9230:	41 8b 42 04          	mov    0x4(%r10),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    9234:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    9237:	83 fa 01             	cmp    $0x1,%edx
    923a:	76 39                	jbe    9275 <__scif_fence_signal+0x475>
    923c:	83 f8 01             	cmp    $0x1,%eax
    923f:	74 34                	je     9275 <__scif_fence_signal+0x475>
}

static __always_inline int constant_test_bit(unsigned int nr, const volatile unsigned long *addr)
{
	return ((1UL << (nr % BITS_PER_LONG)) &
		(addr[nr / BITS_PER_LONG])) != 0;
    9241:	49 8b 82 88 01 00 00 	mov    0x188(%r10),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    9248:	48 85 c0             	test   %rax,%rax
    924b:	0f 88 a1 00 00 00    	js     92f2 <__scif_fence_signal+0x4f2>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    9251:	4c 89 f7             	mov    %r14,%rdi
    9254:	44 89 8d 58 ff ff ff 	mov    %r9d,-0xa8(%rbp)
    925b:	4c 89 85 68 ff ff ff 	mov    %r8,-0x98(%rbp)
    9262:	e8 89 72 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
    9267:	44 8b 8d 58 ff ff ff 	mov    -0xa8(%rbp),%r9d
    926e:	4c 8b 85 68 ff ff ff 	mov    -0x98(%rbp),%r8
bail_out:
		mutex_unlock(&dev->sd_lock);
    9275:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    927c:	44 89 8d 58 ff ff ff 	mov    %r9d,-0xa8(%rbp)
    9283:	4c 89 85 68 ff ff ff 	mov    %r8,-0x98(%rbp)
    928a:	e8 00 00 00 00       	callq  928f <__scif_fence_signal+0x48f>
    928f:	44 8b 8d 58 ff ff ff 	mov    -0xa8(%rbp),%r9d
    9296:	4c 8b 85 68 ff ff ff 	mov    -0x98(%rbp),%r8
    929d:	e9 43 fc ff ff       	jmpq   8ee5 <__scif_fence_signal+0xe5>
		cnt, SCIF_NODE_IDLE))) {
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
    92a2:	49 8d 85 68 01 00 00 	lea    0x168(%r13),%rax
    92a9:	48 89 c7             	mov    %rax,%rdi
    92ac:	48 89 85 58 ff ff ff 	mov    %rax,-0xa8(%rbp)
    92b3:	e8 00 00 00 00       	callq  92b8 <__scif_fence_signal+0x4b8>
		if (SCIFDEV_STOPPED == dev->sd_state ||
    92b8:	41 8b 45 04          	mov    0x4(%r13),%eax
			SCIFDEV_STOPPING == dev->sd_state ||
    92bc:	8d 50 fc             	lea    -0x4(%rax),%edx
		/*
		 * This code path would not be entered unless the remote
		 * SCIF device has actually been put to sleep by the host.
		 */
		mutex_lock(&dev->sd_lock);
		if (SCIFDEV_STOPPED == dev->sd_state ||
    92bf:	83 fa 01             	cmp    $0x1,%edx
    92c2:	76 1d                	jbe    92e1 <__scif_fence_signal+0x4e1>
    92c4:	83 e8 01             	sub    $0x1,%eax
    92c7:	74 18                	je     92e1 <__scif_fence_signal+0x4e1>
    92c9:	49 8b 85 88 01 00 00 	mov    0x188(%r13),%rax
			SCIFDEV_STOPPING == dev->sd_state ||
			SCIFDEV_INIT == dev->sd_state)
			goto bail_out;
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
    92d0:	48 85 c0             	test   %rax,%rax
    92d3:	0f 88 b1 00 00 00    	js     938a <__scif_fence_signal+0x58a>
				clear_bit(SCIF_NODE_MAGIC_BIT, 
					&dev->scif_ref_cnt.counter);
			}
		}
		/* The ref count was not added if the node was idle. */
		atomic_long_add(cnt, &dev->scif_ref_cnt);
    92d9:	4c 89 f7             	mov    %r14,%rdi
    92dc:	e8 0f 72 ff ff       	callq  4f0 <atomic_long_add.constprop.17>
bail_out:
		mutex_unlock(&dev->sd_lock);
    92e1:	48 8b bd 58 ff ff ff 	mov    -0xa8(%rbp),%rdi
    92e8:	e8 00 00 00 00       	callq  92ed <__scif_fence_signal+0x4ed>
    92ed:	e9 8b fc ff ff       	jmpq   8f7d <__scif_fence_signal+0x17d>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    92f2:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 92f8 <__scif_fence_signal+0x4f8>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    92f8:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    92fa:	49 c7 82 b0 01 00 00 	movq   $0x2,0x1b0(%r10)
    9301:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    9305:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    930c:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    9313:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    9317:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
    931d:	44 89 8d 50 ff ff ff 	mov    %r9d,-0xb0(%rbp)
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    9324:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    9328:	41 0f b7 02          	movzwl (%r10),%eax
    932c:	4c 89 85 58 ff ff ff 	mov    %r8,-0xa8(%rbp)
    9333:	4c 89 95 68 ff ff ff 	mov    %r10,-0x98(%rbp)
    933a:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    933e:	e8 00 00 00 00       	callq  9343 <__scif_fence_signal+0x543>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    9343:	4c 8b 95 68 ff ff ff 	mov    -0x98(%rbp),%r10
    934a:	4c 8b 85 58 ff ff ff 	mov    -0xa8(%rbp),%r8
    9351:	44 8b 8d 50 ff ff ff 	mov    -0xb0(%rbp),%r9d
    9358:	49 83 ba b0 01 00 00 	cmpq   $0x2,0x1b0(%r10)
    935f:	02 
    9360:	0f 84 93 00 00 00    	je     93f9 <__scif_fence_signal+0x5f9>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    9366:	49 83 ba b0 01 00 00 	cmpq   $0x3,0x1b0(%r10)
    936d:	03 
    936e:	0f 85 dd fe ff ff    	jne    9251 <__scif_fence_signal+0x451>
				dev->sd_state = SCIFDEV_RUNNING;
    9374:	41 c7 42 04 02 00 00 	movl   $0x2,0x4(%r10)
    937b:	00 
 */
static __always_inline void
clear_bit(int nr, volatile unsigned long *addr)
{
	if (IS_IMMEDIATE(nr)) {
		asm volatile(LOCK_PREFIX "andb %1,%0"
    937c:	f0 41 80 a2 8f 01 00 	lock andb $0x7f,0x18f(%r10)
    9383:	00 7f 
    9385:	e9 c7 fe ff ff       	jmpq   9251 <__scif_fence_signal+0x451>
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    938a:	8b 05 00 00 00 00    	mov    0x0(%rip),%eax        # 9390 <__scif_fence_signal+0x590>
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    9390:	31 d2                	xor    %edx,%edx
		if (test_bit(SCIF_NODE_MAGIC_BIT, 
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
    9392:	49 c7 85 b0 01 00 00 	movq   $0x2,0x1b0(%r13)
    9399:	02 00 00 00 
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    939d:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
			&dev->scif_ref_cnt.counter)) {
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
    93a4:	c7 45 ac 2e 00 00 00 	movl   $0x2e,-0x54(%rbp)
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    93ab:	48 8d 75 a4          	lea    -0x5c(%rbp),%rsi
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
			notif_msg.dst.node = SCIF_HOST_NODE;
    93af:	66 c7 45 a8 00 00    	movw   $0x0,-0x58(%rbp)
			/* Notify host that the remote node must be woken */
			struct nodemsg notif_msg;

			dev->sd_wait_status = OP_IN_PROGRESS;
			notif_msg.uop = SCIF_NODE_WAKE_UP;
			notif_msg.src.node = ms_info.mi_nodeid;
    93b5:	66 89 45 a4          	mov    %ax,-0x5c(%rbp)
			notif_msg.dst.node = SCIF_HOST_NODE;
			notif_msg.payload[0] = dev->sd_node;
    93b9:	41 0f b7 45 00       	movzwl 0x0(%r13),%eax
    93be:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
			/* No error handling for Host SCIF device */
			micscif_nodeqp_send(&scif_dev[SCIF_HOST_NODE],
    93c2:	e8 00 00 00 00       	callq  93c7 <__scif_fence_signal+0x5c7>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    93c7:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    93ce:	02 
    93cf:	0f 84 1b 01 00 00    	je     94f0 <__scif_fence_signal+0x6f0>
				dev->sd_wait_status != OP_IN_PROGRESS);
			/*
			 * Aieee! The host could not wake up the remote node.
			 * Bail out for now.
			 */
			if (dev->sd_wait_status == OP_COMPLETED) {
    93d5:	49 83 bd b0 01 00 00 	cmpq   $0x3,0x1b0(%r13)
    93dc:	03 
    93dd:	0f 85 f6 fe ff ff    	jne    92d9 <__scif_fence_signal+0x4d9>
				dev->sd_state = SCIFDEV_RUNNING;
    93e3:	41 c7 45 04 02 00 00 	movl   $0x2,0x4(%r13)
    93ea:	00 
    93eb:	f0 41 80 a5 8f 01 00 	lock andb $0x7f,0x18f(%r13)
    93f2:	00 7f 
    93f4:	e9 e0 fe ff ff       	jmpq   92d9 <__scif_fence_signal+0x4d9>
			/*
			 * A timeout is not required since only the cards can
			 * initiate this message. The Host is expected to be alive.
			 * If the host has crashed then so will the cards.
			 */
			wait_event(dev->sd_wq, 
    93f9:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    9400:	31 c0                	xor    %eax,%eax
    9402:	b9 0a 00 00 00       	mov    $0xa,%ecx
    9407:	f3 ab                	rep stos %eax,%es:(%rdi)
    9409:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    9410:	00 
    9411:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    9418:	00 00 
    941a:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    941e:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    9425:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    942c:	48 83 c0 18          	add    $0x18,%rax
    9430:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    9434:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    9438:	49 8d 82 98 01 00 00 	lea    0x198(%r10),%rax
    943f:	48 89 85 58 ff ff ff 	mov    %rax,-0xa8(%rbp)
    9446:	eb 1a                	jmp    9462 <__scif_fence_signal+0x662>
    9448:	e8 00 00 00 00       	callq  944d <__scif_fence_signal+0x64d>
    944d:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    9454:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    945b:	44 8b 8d 44 ff ff ff 	mov    -0xbc(%rbp),%r9d
    9462:	48 8b b5 68 ff ff ff 	mov    -0x98(%rbp),%rsi
    9469:	ba 02 00 00 00       	mov    $0x2,%edx
    946e:	44 89 8d 44 ff ff ff 	mov    %r9d,-0xbc(%rbp)
    9475:	48 8b bd 58 ff ff ff 	mov    -0xa8(%rbp),%rdi
    947c:	4c 89 85 48 ff ff ff 	mov    %r8,-0xb8(%rbp)
    9483:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    948a:	e8 00 00 00 00       	callq  948f <__scif_fence_signal+0x68f>
    948f:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    9496:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    949d:	44 8b 8d 44 ff ff ff 	mov    -0xbc(%rbp),%r9d
    94a4:	49 83 ba b0 01 00 00 	cmpq   $0x2,0x1b0(%r10)
    94ab:	02 
    94ac:	74 9a                	je     9448 <__scif_fence_signal+0x648>
    94ae:	48 8b b5 68 ff ff ff 	mov    -0x98(%rbp),%rsi
    94b5:	44 89 8d 44 ff ff ff 	mov    %r9d,-0xbc(%rbp)
    94bc:	48 8b bd 58 ff ff ff 	mov    -0xa8(%rbp),%rdi
    94c3:	4c 89 85 48 ff ff ff 	mov    %r8,-0xb8(%rbp)
    94ca:	4c 89 95 50 ff ff ff 	mov    %r10,-0xb0(%rbp)
    94d1:	e8 00 00 00 00       	callq  94d6 <__scif_fence_signal+0x6d6>
    94d6:	44 8b 8d 44 ff ff ff 	mov    -0xbc(%rbp),%r9d
    94dd:	4c 8b 85 48 ff ff ff 	mov    -0xb8(%rbp),%r8
    94e4:	4c 8b 95 50 ff ff ff 	mov    -0xb0(%rbp),%r10
    94eb:	e9 76 fe ff ff       	jmpq   9366 <__scif_fence_signal+0x566>
    94f0:	48 8d bd 78 ff ff ff 	lea    -0x88(%rbp),%rdi
    94f7:	31 c0                	xor    %eax,%eax
    94f9:	b9 0a 00 00 00       	mov    $0xa,%ecx
    94fe:	f3 ab                	rep stos %eax,%es:(%rdi)
    9500:	48 c7 45 88 00 00 00 	movq   $0x0,-0x78(%rbp)
    9507:	00 
    9508:	65 48 8b 04 25 00 00 	mov    %gs:0x0,%rax
    950f:	00 00 
    9511:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    9515:	48 8d 85 78 ff ff ff 	lea    -0x88(%rbp),%rax
    951c:	48 89 85 68 ff ff ff 	mov    %rax,-0x98(%rbp)
    9523:	48 83 c0 18          	add    $0x18,%rax
    9527:	48 89 45 90          	mov    %rax,-0x70(%rbp)
    952b:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    952f:	49 8d 85 98 01 00 00 	lea    0x198(%r13),%rax
    9536:	48 89 85 60 ff ff ff 	mov    %rax,-0xa0(%rbp)
    953d:	48 8b b5 68 ff ff ff 	mov    -0x98(%rbp),%rsi
    9544:	ba 02 00 00 00       	mov    $0x2,%edx
    9549:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    9550:	e8 00 00 00 00       	callq  9555 <__scif_fence_signal+0x755>
    9555:	49 83 bd b0 01 00 00 	cmpq   $0x2,0x1b0(%r13)
    955c:	02 
    955d:	75 07                	jne    9566 <__scif_fence_signal+0x766>
    955f:	e8 00 00 00 00       	callq  9564 <__scif_fence_signal+0x764>
    9564:	eb d7                	jmp    953d <__scif_fence_signal+0x73d>
    9566:	48 8b b5 68 ff ff ff 	mov    -0x98(%rbp),%rsi
    956d:	48 8b bd 60 ff ff ff 	mov    -0xa0(%rbp),%rdi
    9574:	e8 00 00 00 00       	callq  9579 <__scif_fence_signal+0x779>
    9579:	e9 57 fe ff ff       	jmpq   93d5 <__scif_fence_signal+0x5d5>
    957e:	66 90                	xchg   %ax,%ax

0000000000009580 <scif_fence_signal>:
	return err;
}

int scif_fence_signal(scif_epd_t epd, off_t loff, uint64_t lval,
				off_t roff, uint64_t rval, int flags)
{
    9580:	55                   	push   %rbp
    9581:	48 89 e5             	mov    %rsp,%rbp
    9584:	41 57                	push   %r15
    9586:	41 56                	push   %r14
    9588:	41 55                	push   %r13
    958a:	41 54                	push   %r12
    958c:	53                   	push   %rbx
    958d:	48 83 ec 18          	sub    $0x18,%rsp
    9591:	e8 00 00 00 00       	callq  9596 <scif_fence_signal+0x16>
    9596:	49 89 ff             	mov    %rdi,%r15
    9599:	49 89 f4             	mov    %rsi,%r12
    959c:	49 89 d5             	mov    %rdx,%r13
    959f:	4c 89 45 c0          	mov    %r8,-0x40(%rbp)
 * to synchronize calling this API with put_kref_count.
 */
static __always_inline void
get_kref_count(scif_epd_t epd)
{
	kref_get(&(epd->ref_count));
    95a3:	48 8d 9f 70 01 00 00 	lea    0x170(%rdi),%rbx
    95aa:	49 89 ce             	mov    %rcx,%r14
    95ad:	44 89 4d cc          	mov    %r9d,-0x34(%rbp)
    95b1:	48 89 df             	mov    %rbx,%rdi
    95b4:	e8 00 00 00 00       	callq  95b9 <scif_fence_signal+0x39>
	int ret;
	get_kref_count(epd);
	ret = __scif_fence_signal(epd, loff, lval, roff, rval, flags);
    95b9:	44 8b 4d cc          	mov    -0x34(%rbp),%r9d
    95bd:	4c 89 f1             	mov    %r14,%rcx
    95c0:	4c 89 ea             	mov    %r13,%rdx
    95c3:	4c 8b 45 c0          	mov    -0x40(%rbp),%r8
    95c7:	4c 89 e6             	mov    %r12,%rsi
    95ca:	4c 89 ff             	mov    %r15,%rdi
    95cd:	e8 00 00 00 00       	callq  95d2 <scif_fence_signal+0x52>
 * to synchronize calling this API with get_kref_count.
 */
static __always_inline void
put_kref_count(scif_epd_t epd)
{
	kref_put(&(epd->ref_count), scif_ref_rel);
    95d2:	48 89 df             	mov    %rbx,%rdi
    95d5:	48 c7 c6 00 00 00 00 	mov    $0x0,%rsi
    95dc:	41 89 c4             	mov    %eax,%r12d
    95df:	e8 00 00 00 00       	callq  95e4 <scif_fence_signal+0x64>
	put_kref_count(epd);
	return ret;
}
    95e4:	48 83 c4 18          	add    $0x18,%rsp
    95e8:	44 89 e0             	mov    %r12d,%eax
    95eb:	5b                   	pop    %rbx
    95ec:	41 5c                	pop    %r12
    95ee:	41 5d                	pop    %r13
    95f0:	41 5e                	pop    %r14
    95f2:	41 5f                	pop    %r15
    95f4:	5d                   	pop    %rbp
    95f5:	c3                   	retq   
    95f6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    95fd:	00 00 00 

0000000000009600 <micscif_pci_dev>:
 * @node: node ID
 *
 *  Return the pci_dev associated with a node.
 */
int micscif_pci_dev(uint16_t node, struct pci_dev **pdev)
{
    9600:	55                   	push   %rbp
    9601:	48 89 e5             	mov    %rsp,%rbp
    9604:	53                   	push   %rbx
    9605:	48 83 ec 08          	sub    $0x8,%rsp
    9609:	e8 00 00 00 00       	callq  960e <micscif_pci_dev+0xe>
#ifdef _MIC_SCIF_
       /* This *is* a PCI device, therefore no pdev to return. */
       return -ENODEV;
#else
	 mic_ctx_t *mic_ctx = get_per_dev_ctx(node - 1);
    960e:	83 ef 01             	sub    $0x1,%edi
 * @node: node ID
 *
 *  Return the pci_dev associated with a node.
 */
int micscif_pci_dev(uint16_t node, struct pci_dev **pdev)
{
    9611:	48 89 f3             	mov    %rsi,%rbx
#ifdef _MIC_SCIF_
       /* This *is* a PCI device, therefore no pdev to return. */
       return -ENODEV;
#else
	 mic_ctx_t *mic_ctx = get_per_dev_ctx(node - 1);
    9614:	0f b7 ff             	movzwl %di,%edi
    9617:	e8 00 00 00 00       	callq  961c <micscif_pci_dev+0x1c>
	*pdev = mic_ctx->bi_pdev;
    961c:	48 8b 80 a8 04 00 00 	mov    0x4a8(%rax),%rax
    9623:	48 89 03             	mov    %rax,(%rbx)
	return 0;
#endif
}
    9626:	48 83 c4 08          	add    $0x8,%rsp
    962a:	31 c0                	xor    %eax,%eax
    962c:	5b                   	pop    %rbx
    962d:	5d                   	pop    %rbp
    962e:	c3                   	retq   
    962f:	90                   	nop

0000000000009630 <micscif_pci_info>:
 * @node: node ID
 *
 * Populate the pci device info pointer associated with a node.
 */
int micscif_pci_info(uint16_t node, struct scif_pci_info *dev)
{
    9630:	55                   	push   %rbp
    9631:	48 89 e5             	mov    %rsp,%rbp
    9634:	53                   	push   %rbx
    9635:	48 83 ec 08          	sub    $0x8,%rsp
    9639:	e8 00 00 00 00       	callq  963e <micscif_pci_info+0xe>
	int i;
	mic_ctx_t *mic_ctx = get_per_dev_ctx(node - 1);
    963e:	83 ef 01             	sub    $0x1,%edi
 * @node: node ID
 *
 * Populate the pci device info pointer associated with a node.
 */
int micscif_pci_info(uint16_t node, struct scif_pci_info *dev)
{
    9641:	48 89 f3             	mov    %rsi,%rbx
	int i;
	mic_ctx_t *mic_ctx = get_per_dev_ctx(node - 1);
    9644:	0f b7 ff             	movzwl %di,%edi
    9647:	e8 00 00 00 00       	callq  964c <micscif_pci_info+0x1c>
	struct pci_dev *pdev;

	if (!mic_ctx)
    964c:	48 85 c0             	test   %rax,%rax
    964f:	74 70                	je     96c1 <micscif_pci_info+0x91>
		return -ENODEV;

	dev->pdev = pdev = mic_ctx->bi_pdev;
    9651:	48 8b 88 a8 04 00 00 	mov    0x4a8(%rax),%rcx
    9658:	48 8d 53 08          	lea    0x8(%rbx),%rdx
    965c:	48 8d b3 90 00 00 00 	lea    0x90(%rbx),%rsi
    9663:	48 89 0b             	mov    %rcx,(%rbx)
    9666:	48 81 c1 00 03 00 00 	add    $0x300,%rcx
    966d:	eb 1b                	jmp    968a <micscif_pci_info+0x5a>
    966f:	90                   	nop
			dev->va[i] = NULL;
			continue;
		}
		if (pci_resource_flags(pdev, i) & IORESOURCE_PREFETCH) {
			/* TODO: Change comparison check for KNL. */
			if (pci_resource_start(pdev, i) == mic_ctx->aper.pa)
    9670:	48 3b 78 20          	cmp    0x20(%rax),%rdi
    9674:	74 42                	je     96b8 <micscif_pci_info+0x88>
				dev->va[i] = mic_ctx->aper.va;
			else
				dev->va[i] = NULL;
    9676:	48 c7 02 00 00 00 00 	movq   $0x0,(%rdx)
    967d:	48 83 c2 08          	add    $0x8,%rdx
    9681:	48 83 c1 38          	add    $0x38,%rcx

	if (!mic_ctx)
		return -ENODEV;

	dev->pdev = pdev = mic_ctx->bi_pdev;
	for (i = 0; i < PCI_NUM_RESOURCES; i++) {
    9685:	48 39 f2             	cmp    %rsi,%rdx
    9688:	74 22                	je     96ac <micscif_pci_info+0x7c>
		if (!pci_resource_start(pdev, i)) {
    968a:	48 8b 39             	mov    (%rcx),%rdi
    968d:	48 85 ff             	test   %rdi,%rdi
    9690:	74 e4                	je     9676 <micscif_pci_info+0x46>
			dev->va[i] = NULL;
			continue;
		}
		if (pci_resource_flags(pdev, i) & IORESOURCE_PREFETCH) {
    9692:	f6 41 19 20          	testb  $0x20,0x19(%rcx)
    9696:	75 d8                	jne    9670 <micscif_pci_info+0x40>
			if (pci_resource_start(pdev, i) == mic_ctx->aper.pa)
				dev->va[i] = mic_ctx->aper.va;
			else
				dev->va[i] = NULL;
		} else {
			dev->va[i] = mic_ctx->mmio.va;
    9698:	48 8b 38             	mov    (%rax),%rdi
    969b:	48 83 c2 08          	add    $0x8,%rdx
    969f:	48 83 c1 38          	add    $0x38,%rcx
    96a3:	48 89 7a f8          	mov    %rdi,-0x8(%rdx)

	if (!mic_ctx)
		return -ENODEV;

	dev->pdev = pdev = mic_ctx->bi_pdev;
	for (i = 0; i < PCI_NUM_RESOURCES; i++) {
    96a7:	48 39 f2             	cmp    %rsi,%rdx
    96aa:	75 de                	jne    968a <micscif_pci_info+0x5a>
				dev->va[i] = NULL;
		} else {
			dev->va[i] = mic_ctx->mmio.va;
		}
	}
	return 0;
    96ac:	31 c0                	xor    %eax,%eax
}
    96ae:	48 83 c4 08          	add    $0x8,%rsp
    96b2:	5b                   	pop    %rbx
    96b3:	5d                   	pop    %rbp
    96b4:	c3                   	retq   
    96b5:	0f 1f 00             	nopl   (%rax)
			continue;
		}
		if (pci_resource_flags(pdev, i) & IORESOURCE_PREFETCH) {
			/* TODO: Change comparison check for KNL. */
			if (pci_resource_start(pdev, i) == mic_ctx->aper.pa)
				dev->va[i] = mic_ctx->aper.va;
    96b8:	48 8b 78 18          	mov    0x18(%rax),%rdi
    96bc:	48 89 3a             	mov    %rdi,(%rdx)
    96bf:	eb bc                	jmp    967d <micscif_pci_info+0x4d>
	int i;
	mic_ctx_t *mic_ctx = get_per_dev_ctx(node - 1);
	struct pci_dev *pdev;

	if (!mic_ctx)
		return -ENODEV;
    96c1:	b8 ed ff ff ff       	mov    $0xffffffed,%eax
    96c6:	eb e6                	jmp    96ae <micscif_pci_info+0x7e>
    96c8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    96cf:	00 

00000000000096d0 <scif_pci_info>:
 * Return Values
 *     Upon successful completion, scif_pci_info() returns 0; otherwise the
 *	an appropriate error is returned as documented in scif.h.
 */
int scif_pci_info(uint16_t node, struct scif_pci_info *dev)
{
    96d0:	55                   	push   %rbp
    96d1:	48 89 e5             	mov    %rsp,%rbp
    96d4:	e8 00 00 00 00       	callq  96d9 <scif_pci_info+0x9>
#ifdef _MIC_SCIF_
	return -EINVAL;
#else
	if (node > ms_info.mi_maxid)
    96d9:	0f b7 ff             	movzwl %di,%edi
    96dc:	3b 3d 00 00 00 00    	cmp    0x0(%rip),%edi        # 96e2 <scif_pci_info+0x12>
    96e2:	77 3c                	ja     9720 <scif_pci_info+0x50>
		return -EINVAL;

	if ((scif_dev[node].sd_state == SCIFDEV_NOTPRESENT) ||
    96e4:	48 63 c7             	movslq %edi,%rax
    96e7:	48 89 c2             	mov    %rax,%rdx
    96ea:	48 c1 e0 09          	shl    $0x9,%rax
    96ee:	48 c1 e2 05          	shl    $0x5,%rdx
    96f2:	48 29 d0             	sub    %rdx,%rax
    96f5:	8b 90 00 00 00 00    	mov    0x0(%rax),%edx
    96fb:	85 d2                	test   %edx,%edx
    96fd:	74 19                	je     9718 <scif_pci_info+0x48>
 *
 * Returns true if the SCIF Device passed is the self aka Loopback SCIF device.
 */
static inline int is_self_scifdev(struct micscif_dev *dev)
{
	return dev->sd_node == ms_info.mi_nodeid;
    96ff:	0f b7 80 00 00 00 00 	movzwl 0x0(%rax),%eax
    9706:	3b 05 00 00 00 00    	cmp    0x0(%rip),%eax        # 970c <scif_pci_info+0x3c>
    970c:	74 0a                	je     9718 <scif_pci_info+0x48>
	    is_self_scifdev(&scif_dev[node]))
		return -ENODEV;

	return micscif_pci_info(node, dev);
    970e:	e8 00 00 00 00       	callq  9713 <scif_pci_info+0x43>
#endif
}
    9713:	5d                   	pop    %rbp
    9714:	c3                   	retq   
    9715:	0f 1f 00             	nopl   (%rax)
	if (node > ms_info.mi_maxid)
		return -EINVAL;

	if ((scif_dev[node].sd_state == SCIFDEV_NOTPRESENT) ||
	    is_self_scifdev(&scif_dev[node]))
		return -ENODEV;
    9718:	b8 ed ff ff ff       	mov    $0xffffffed,%eax

	return micscif_pci_info(node, dev);
#endif
}
    971d:	5d                   	pop    %rbp
    971e:	c3                   	retq   
    971f:	90                   	nop
{
#ifdef _MIC_SCIF_
	return -EINVAL;
#else
	if (node > ms_info.mi_maxid)
		return -EINVAL;
    9720:	b8 ea ff ff ff       	mov    $0xffffffea,%eax
	    is_self_scifdev(&scif_dev[node]))
		return -ENODEV;

	return micscif_pci_info(node, dev);
#endif
}
    9725:	5d                   	pop    %rbp
    9726:	c3                   	retq   
    9727:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    972e:	00 00 

0000000000009730 <print_ep_state>:
/*
 * DEBUG helper functions
 */
void
print_ep_state(struct endpt *ep, char *label)
{
    9730:	55                   	push   %rbp
    9731:	48 89 e5             	mov    %rsp,%rbp
    9734:	e8 00 00 00 00       	callq  9739 <print_ep_state+0x9>
	if (ep)
    9739:	48 85 ff             	test   %rdi,%rdi
    973c:	74 22                	je     9760 <print_ep_state+0x30>
		printk("%s: EP %p state %s\n", 
			label, ep, scif_ep_states[ep->state]);
    973e:	8b 07                	mov    (%rdi),%eax
 */
void
print_ep_state(struct endpt *ep, char *label)
{
	if (ep)
		printk("%s: EP %p state %s\n", 
    9740:	48 89 fa             	mov    %rdi,%rdx
    9743:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    974a:	48 8b 0c c5 00 00 00 	mov    0x0(,%rax,8),%rcx
    9751:	00 
    9752:	31 c0                	xor    %eax,%eax
    9754:	e8 00 00 00 00       	callq  9759 <print_ep_state+0x29>
			label, ep, scif_ep_states[ep->state]);
	else
		printk("%s: EP %p\n state ?\n", label, ep);
}
    9759:	5d                   	pop    %rbp
    975a:	c3                   	retq   
    975b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
{
	if (ep)
		printk("%s: EP %p state %s\n", 
			label, ep, scif_ep_states[ep->state]);
	else
		printk("%s: EP %p\n state ?\n", label, ep);
    9760:	31 d2                	xor    %edx,%edx
    9762:	48 c7 c7 00 00 00 00 	mov    $0x0,%rdi
    9769:	31 c0                	xor    %eax,%eax
    976b:	e8 00 00 00 00       	callq  9770 <print_ep_state+0x40>
}
    9770:	5d                   	pop    %rbp
    9771:	c3                   	retq   
