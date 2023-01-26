# Weenix-Kernel-Implementation
 
 **NOTE:**
 - For full weenix kernel project, please contact me at svregala@gmail.com (I'm required to keep the whole project description on private).
 - Additionally, the posted code is to showcase the files that my team and I completed: other supplemental and helper files are not displayed.

- K1, K2, and K3 indicate the 3 different stages my team and I completed.
- **K1 covers processes and threads. The following files completed by my team were**: 
   - kernel/main/kmain.c: ("PROCS: bootstrap");
   - kernel/main/kmain.c: ("PROCS: initproc_create");
   - kernel/main/kmain.c: ("PROCS: initproc_run"); #first procedure for process 1 (init)

   - kernel/proc/kmutex.c: ("PROCS: kmutex_init"); #copy the textbook code for mutex and change it
   - kernel/proc/kmutex.c: ("PROCS: kmutex_lock");
   - kernel/proc/kmutex.c: ("PROCS: kmutex_lock_cancellable"); #cancel me when I’m waiting for mutex to unlock
   - kernel/proc/kmutex.c: ("PROCS: kmutex_unlock");

   - kernel/proc/kthread.c: ("PROCS: kthread_create");
   - kernel/proc/kthread.c: ("PROCS: kthread_cancel");
   - kernel/proc/kthread.c: ("PROCS: kthread_exit");

   - kernel/proc/proc.c: ("PROCS: proc_create");
   - kernel/proc/proc.c: ("PROCS: proc_cleanup");
   - kernel/proc/proc.c: ("PROCS: proc_kill"); #find the thread in the process and try to kill it
   - kernel/proc/proc.c: ("PROCS: proc_kill_all"); #ask all the processes to die except for 0 (IDLE), 1 (INIT) and 2
   - kernel/proc/proc.c: ("PROCS: proc_thread_exited");
   - kernel/proc/proc.c: ("PROCS: do_waitpid"); #wait system call
   - kernel/proc/proc.c: ("PROCS: do_exit"); #exit system call

   - kernel/proc/sched.c: ("PROCS: sched_cancellable_sleep_on"); #pthread_condition_wait(), wake me up, cancel me
   - kernel/proc/sched.c: ("PROCS: sched_cancel");
   - kernel/proc/sched.c: ("PROCS: sched_switch");
   - kernel/proc/sched.c: ("PROCS: sched_make_runnable"); #all threads that are ready to run will wait inside runq

   - kernel/proc/sched_helper.c: ("PROCS: sched_sleep_on"); #wake me up, don’t cancel me
   - kernel/proc/sched_helper.c: ("PROCS: sched_wakeup_on"); #pthread_condition_signal(), move it into runq
   - kernel/proc/sched_helper.c: ("PROCS: sched_broadcast_on"); #pthread_condition_broadcast()


- **K2 covers the virtual file system (VFS) layer. The following files completed were**:
   - main/kmain.c:218	 idleproc_run() 	VFS 
   - main/kmain.c:223	 idleproc_run() 	VFS 
   
   - fs/vnode.c:460		special_file_read() 	VFS
   - fs/vnode.c:473 	special_file_write() 	VFS
   
   - fs/vfs_syscall.c:68	 do_read() 		VFS 
   - fs/vfs_syscall.c:83	 do_write() 		VFS
   - fs/vfs_syscall.c:97 	do_close() 		VFS 
   - fs/vfs_syscall.c:120 	do_dup()		 VFS 
   - fs/vfs_syscall.c:136 	do_dup2()		 VFS 
   - fs/vfs_syscall.c:168 	do_mknod() 		VFS 
   - fs/vfs_syscall.c:189 	do_mkdir() 		VFS 
   - fs/vfs_syscall.c:214 	do_rmdir() 		VFS 
   - fs/vfs_syscall.c:235 	do_unlink() 		VFS
   - fs/vfs_syscall.c:263 	do_link() 		VFS
   - fs/vfs_syscall.c:278 	do_rename() 		VFS 
   - fs/vfs_syscall.c:298 	do_chdir() 		VFS
   - fs/vfs_syscall.c:320 	do_getdent() 		VFS
   - fs/vfs_syscall.c:337 	do_lseek() 		VFS 
   - fs/vfs_syscall.c:357 	do_stat() 		VFS 
   
   - fs/namev.c:45 		lookup() 		VFS 
   - fs/namev.c:72 		dir_namev() 		VFS 
   - fs/namev.c:90 		open_namev() 	VFS    
   - fs/open.c:94		 do_open() 		VFS

- **K3 covers virtual memory (VM). The following files completed were**:
   - mm/pframe.c:359 		pframe_get() 			VM
   - mm/pframe.c:379 		pframe_pin() 			VM
   - mm/pframe.c:395 		pframe_unpin() 		VM
   
   - api/syscall.c:77 		sys_read() 			VM
   - api/syscall.c:87 		sys_write() 			VM
   - api/syscall.c:103 		sys_getdents() 		VM
   - api/access.c:144 		addr_perm() 			VM
   - api/access.c:160 		range_perm() 			VM
   - proc/fork.c:77 			do_fork() 			VM
   
   - vm/pagefault.c:72 		handle_pagefault() 		VM
   - vm/shadow.c:73 		shadow_init() 			VM
   - vm/shadow.c:85 		shadow_create() 		VM
   - vm/shadow.c:97 		shadow_ref() 			VM
   - vm/shadow.c:111 		shadow_put() 			VM
   - vm/shadow.c:126 		shadow_lookuppage() 	VM
   - vm/shadow.c:144 		shadow_fillpage() 		VM
   - vm/shadow.c:153		shadow_dirtypage() 		VM
   - vm/shadow.c:160		shadow_cleanpage() 		VM
   - proc/kthread.c:161		kthread_clone() 		VM
   
   - fs/vnode.c:487 		special_file_mmap() 		VM
   - fs/vnode.c:499 		special_file_fillpage() 	VM
   - fs/vnode.c:511 		special_file_dirtypage() 	VM
   - fs/vnode.c:523 		special_file_cleanpage() 	VM
   
   - vm/vmmap.c:129 		vmmap_create() 		VM
   - vm/vmmap.c:138 		vmmap_destroy() 		VM
   - vm/vmmap.c:148 		vmmap_insert() 		VM
   - vm/vmmap.c:161 		vmmap_find_range() 		VM
   - vm/vmmap.c:182 		vmmap_clone() 		VM
   - vm/vmmap.c:215 		vmmap_map() 		VM
   - vm/vmmap.c:251 		vmmap_remove() 		VM
   - vm/vmmap.c:262 		vmmap_is_range_empty() 	VM
   - vm/vmmap.c:277 		vmmap_read() 		VM
   - vm/vmmap.c:292 		vmmap_write() 		VM
   - vm/brk.c:76 			do_brk() 			VM

   - vm/anon.c:60 			anon_init() 			VM
   - vm/anon.c:72 			anon_create() 		VM
   - vm/anon.c:84 			anon_ref() 			VM
   - vm/anon.c:98 			anon_put() 			VM
   - vm/anon.c:106 		anon_lookuppage() 		VM
   - vm/anon.c:115 		anon_fillpage() 		VM
   - vm/anon.c:122 		anon_dirtypage() 		VM
   - vm/anon.c:129 		anon_cleanpage() 		VM
   
   - vm/mmap.c:55 		do_mmap() 			VM
   - vm/mmap.c:70 		do_munmap() 		VM