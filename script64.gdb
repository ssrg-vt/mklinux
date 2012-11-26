
# scripts (tested on 64 bit machiens)
# Antonio Barbalace 2012
#

define dump_page_flags_ext
  set $_f=$arg0
  	
  if ($_f & 0x4000)
    printf "head,"
  end
  if ($_f & 0x8000)
    printf "tail,"
  end
  if ($_f & 0x010000)
	printf "swapcache,"
  end
  if ($_f & 0x020000)
	printf "mappedtodisk,"
  end
  if ($_f & 0x040000)
    printf "reclaim,"
  end
  if ($_f & 0x080000)
    printf "swapbacked,"
  end
  if ($_f & 0x100000)
    printf "unevictable,"
  end
end

define dump_page_flags_noext
  set $_f=$arg0

  if ($_f & 0x4000)
    printf "compound,"
  end
  if ($_f & 0x8000)
	printf "swapcache,"
  end
  if ($_f & 0x010000)
	printf "mappedtodisk,"
  end
  if ($_f & 0x020000)
    printf "reclaim,"
  end
  if ($_f & 0x040000)
    printf "swapbacked,"
  end
  if ($_f & 0x080000)
    printf "unevictable,"
  end
end

define dump_page_flags
  set $_e=(struct page *)$arg0
  set $_f=$_e->flags
  if ($_f & 0x01)
    printf "locked,"
  end
  if ($_f & 0x02)
    printf "error,"
  end
  if ($_f & 0x04)
    printf "referenced,"
  end
  if ($_f & 0x08)
    printf "uptodate,"
  end
  if ($_f & 0x10) 
    print "dirty,"
  end
  if ($_f & 0x20)
    printf "lru,"
  end
  if ($_f & 0x40)
    printf "active,"
  end
  if ($_f & 0x80)
    printf "slab,"
  end
  if ($_f & 0x0100)
    printf "owner_priv_1,"
  end
  if ($_f & 0x0200)
    printf "arch_1,"
  end
  if ($_f & 0x0400)
    printf "reserved,"
  end
  if ($_f & 0x0800)
    printf "private,"
  end
  if ($_f & 0x1000)
    printf "private_2,"
  end
  if ($_f & 0x2000)
    printf "writeback,"	
  end
  dump_page_flags_ext $_f
  printf "\n"
end

define dump_page_mapping_print
  set $_e=(struct page *)$arg0
  set $_f=(unsigned long)$_e->mapping
  if ($_f & 0x01)
    printf "ANON,"
    if ($_f & 0x02)
      printf "KSM,"
    end
    printf "\n"
    print *((struct anon_vma*)($_f & ~ 0x03))
  else
    printf "FILE,"
    printf "\n"
    print *((struct address_space*)($_f))
  end
end 

define dump_page_mapping
  set $_e=(struct page *)$arg0
  set $_f=(unsigned long)$_e->mapping
  if ($_f & 0x01)
    printf "ANON,"
    if ($_f & 0x02)
      printf "KSM,"
    end
  else
    printf "FILE,"
  end
  printf "\n"
end 

define walk_pages
 set $_i=$arg0
 set $_l=$arg1
 set $_addr=(struct page *)$arg2 
 printf "%d %d %p\n", $_i, $_l, $_addr
 while ($_i < $_l)
  if ( ((struct page *)$_addr)[$_i].mapping != 0 )
   set $__arg= &((struct page *)$_addr)[$_i]
 
   printf "****** id: %d (struct page*)%p\n", $_i, $__arg
   printf "flags(%lx): ", ($__arg)->flags
   dump_page_flags $__arg
   printf "mapping(%p): ", (void *)(((long)($__arg)->mapping) & ~0x03)
   dump_page_mapping $__arg
  end
  set $_i=$_i+1
 end
 echo "end"
end

define scan_first
 set $_i=$arg0
 set $_l=$arg1
 set $_addr=(struct page *)$arg2 
 printf "%d %d %p\n", $_i, $_l, $_addr
 while ($_i < $_l)
  if ( ((struct page *)$_addr)[$_i].mapping != 0 )
   printf "***** %d *****\n", $_i 
   print ((struct page *)$_addr)[$_i]
  end
  set $_i=$_i+1
 end
 echo "end"
end

#one struct anon_vma is mapped to one struct anon_vma_chain that is connected to oh
define walk_same_vma

define walk_same_anon_vma

define dinitcall
  # as ps first argument is relocation on 32bit
  set $offset=$arg0
  set $s=(char *)&__early_initcall_end
  set $s=(initcall_t *)($s + $offset)
  set $e=(char *)&__initcall_end
  set $e=(initcall_t *)($e + $offset)
  while $s!=$e
    print *((initcall_t *)$s)
	set $s=$s+1
  end
end

define dearly
  # as ps first argument is relocation on 32bit
  set $offset=$arg0
  set $s=(char *)&__initcall_start
  set $s=(initcall_t *)($s + $offset)
  set $e=(char *)&__early_initcall_end
  set $e=(initcall_t *)($e + $offset)
  while $s!=$e
    print *((initcall_t *)$s)
	set $s=$s+1
  end
end


define task_struct_show
   # task_struct addr and PID
   set $st=(struct task_struct *)$arg0
   #printf "0x%08X %5d ", $st, $st->pid
   printf "%p %5d ", $st, $st->pid

   # State
   if ($st->state == 0)
     printf "Running   "
   else
     if ($st->state == 1)
       printf "Sleeping  "
     else
       if ($st->state == 2)
         printf "Disksleep "
       else
         if ($st->state == 4)
           printf "Zombie    "
         else
           if ($st->state == 8)
             printf "sTopped   "
           else
             if ($st->state == 16)
               printf "Wpaging   "
             else
               printf "%2d        ", $st->state
             end
           end
         end
       end
     end
   end

   # User NIP
   #printf "0x%08X ", $st->thread.ip #not working on 64bit
   
   # Display the kernel stack pointer
   #printf "0x%08X ", $st->thread.sp
   printf "%p ", $st->thread.sp
   
   # Display the processor id
   printf "cpu %d ", ((struct thread_info *)($st)->stack)->cpu
   #printf "on(cpu:%d,rq:%d) ", ($st)->on_cpu, ($st)->on_rq
   #printf "prio:%d:%d:%d:%d ", $st->prio, $st->static_prio, $st->normal_prio, $st->rt_priority
   
   # Scheduling info
 if ($st->sched_class == &stop_sched_class)
   printf "STOP "
 else 
   if ($st->sched_class == &rt_sched_class)
     printf "RT    %d ", $st->rt.rt_rq->rq->cpu
   else
     if ($st->sched_class == &fair_sched_class)
       printf "FAIR %d ", $st->se.cfs_rq->rq->cpu
     else
       if ($st->sched_class == &idle_sched_class)
         printf "IDLE "
       else
         printf "none "
       end
     end
   end
 end
      
   # comm
   printf "%s\n", $st->comm
end

define find_next_task
  # Given a task address, find the next task in the linked list
  set $t = (struct task_struct *)$arg0
  set $off=( (char *)&($t->tasks) - (char *)$t)
  set $t=(struct task_struct *)( (char *)$t->tasks.next - $off)
end

define ps
  # first argument handle eventual relocation on 32bit
  set $offset=$arg0
  set $t=(char *)&init_task
  set $t=$t + $offset
  print/x $t
  task_struct_show $t
  find_next_task $t
  # Walk the list
  while ((char *)&init_task +$offset)!=$t
    # Display useful info about each task
    task_struct_show $t
    find_next_task $t
  end
end
document ps
this version works only in kernel space, if the actual kernel is in user space this dumper does not work
we will solve this issue in a further version
this version support kernel relocation the only argument is the kernel relocation offset 
end

#from Documentation/kdump/gdbmacro.txt
define bttnobp
  set $tasks_off=((size_t)&((struct task_struct *)0)->tasks)
  set $pid_off=((size_t)&((struct task_struct *)0)->pids[1].node.next)
  set $init_t=&init_task
  set $next_t=(((char *)($init_t->tasks).next) - $tasks_off)
  
  while ($next_t != $init_t)
    set $next_t=(struct task_struct *)$next_t
    printf "\npid %d; comm %s:\n", $next_t.pid, $next_t.comm
    printf "===================\n"
    set var $stackp = $next_t.thread.sp
    set var $stack_top = ($stackp & ~4095) + 4096

    while ($stackp < $stack_top)
      if (*($stackp) > _stext && *($stackp) < _sinittext)
        #info line *($stackp)
        info symbol *($stackp)
      end
      set $stackp += 8
    end

    set $next_t=(char *)($next_t->tasks.next) - $tasks_off
  end
end

# The following code is not clear why it exists
# it dumps all the stack changing pid?!?! walk per pid instead of task list
    set $next_th=(((char *)$next_t->pids[1].node.next) - $pid_off)
    while ($next_th != $next_t)
      set $next_th=(struct task_struct *)$next_th
      printf "\npid %d; comm %s:\n", $next_t.pid, $next_t.comm
      printf "===================\n"
      set var $stackp = $next_t.thread.sp
      set var $stack_top = ($stackp & ~4095) + 4096

      while ($stackp < $stack_top)
        if (*($stackp) > _stext && *($stackp) < _sinittext)
          info symbol *($stackp)
        end
        set $stackp += 8
      end
      set $next_th=(((char *)$next_th->pids[1].node.next) - $pid_off)
    end
    
    

define btt
  set $tasks_off=((size_t)&((struct task_struct *)0)->tasks)
  set $pid_off=((size_t)&((struct task_struct *)0)->pids[1].node.next)
  set $init_t=&init_task
  set $next_t=(((char *)($init_t->tasks).next) - $tasks_off)

  while ($next_t != $init_t)
    set $next_t=(struct task_struct *)$next_t
    printf "\npid %d; comm %s:\n", $next_t.pid, $next_t.comm
    printf "===================\n"
    set var $stackp = $next_t.thread.sp
    set var $stack_top = ($stackp & ~4095) + 4096
    set var $stack_bot = ($stackp & ~4095)

    set $stackp = *($stackp)
    while (($stackp < $stack_top) && ($stackp > $stack_bot))
      set var $addr = *($stackp + 8)
      info symbol $addr
      set $stackp = *($stackp)
    end

    set $next_th=(((char *)$next_t->pids[1].node.next) - $pid_off)
    while ($next_th != $next_t)
      set $next_th=(struct task_struct *)$next_th
      printf "\npid %d; comm %s:\n", $next_t.pid, $next_t.comm
      printf "===================\n"
      set var $stackp = $next_t.thread.sp
      set var $stack_top = ($stackp & ~4095) + 4096
      set var $stack_bot = ($stackp & ~4095)

      set $stackp = *($stackp)
      while (($stackp < $stack_top) && ($stackp > $stack_bot))
        set var $addr = *($stackp + 8)
        info symbol $addr
        set $stackp = *($stackp)
      end
      set $next_th=(((char *)$next_th->pids[1].node.next) - $pid_off)
    end
    set $next_t=(char *)($next_t->tasks.next) - $tasks_off
  end
end

define btpid
  set var $pid = $arg0
  set $tasks_off=((size_t)&((struct task_struct *)0)->tasks)
  set $pid_off=((size_t)&((struct task_struct *)0)->pids[1].node.next)
  set $init_t=&init_task
  set $next_t=(((char *)($init_t->tasks).next) - $tasks_off)
  set var $pid_task = 0

  while ($next_t != $init_t)
    set $next_t=(struct task_struct *)$next_t

    if ($next_t.pid == $pid)
      set $pid_task = $next_t
    end

    set $next_t=(char *)($next_t->tasks.next) - $tasks_off
  end

  printf "\npid %d; comm %s; sp %p:\n", $pid_task.pid, $pid_task.comm, $pid_task.thread.sp
  printf "===================\n"
  set var $stackp = $pid_task.thread.sp
  set var $stack_top = ($stackp & ~8181) + 8182
  #set var $stack_bot = ($stackp & ~4095)

  #set $stackp = *($stackp)
  #while (($stackp < $stack_top) && ($stackp > $stack_bot))
#    set var $addr = *($stackp + 8)
#    info symbol $addr
#    set $stackp = *($stackp)
  #end
  
    while ($stackp < $stack_top)
      if (*($stackp) > _stext && *($stackp) < _sinittext)
        printf " %p - %p \n", *($stackp), $stackp
        info symbol *($stackp)
        info line *(*($stackp))  
      end
      set $stackp += 4
    end
  
end

define dmesg
  set $__log_buf = (char*)$arg0
  set $log_start = *(unsigned int*)$arg1
  set $log_end = *(unsigned int*)$arg2
  set $x = $log_start
  printf "%p %d %d %d\n", $__log_buf, $log_start, $log_end, $x 
  while ($x < $log_end)
    set $c = (char)(($__log_buf)[$x++])
    printf "%c" , $c
  end
  printf "\n"
end
document dmesg 
dmesg __log_buf log_start log_end
Print the content of the kernel message buffer
from elinux.org
end

define trapinfo
  set var $pid = $arg0
  set $tasks_off=((size_t)&((struct task_struct *)0)->tasks)
  set $init_t=&init_task
  set $next_t=(((char *)($init_t->tasks).next) - $tasks_off)
  set var $pid_task = 0

  while ($next_t != $init_t)
    set $next_t=(struct task_struct *)$next_t

    if ($next_t.pid == $pid)
      set $pid_task = $next_t
    end

    set $next_t=(char *)($next_t->tasks.next) - $tasks_off
  end

  printf "Trapno %ld, cr2 0x%lx, error_code %ld\n", $pid_task.thread.trap_no, \
    $pid_task.thread.cr2, $pid_task.thread.error_code
    
end
document trapinfo
	Run info threads and lookup pid of thread #1
	'trapinfo <pid>' will tell you by which trap & possibly
	address the kernel panicked.
end


