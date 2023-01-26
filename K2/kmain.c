#include "types.h"
#include "globals.h"
#include "kernel.h"
#include "errno.h"

#include "util/gdb.h"
#include "util/init.h"
#include "util/debug.h"
#include "util/string.h"
#include "util/printf.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/pagetable.h"
#include "mm/pframe.h"

#include "vm/vmmap.h"
#include "vm/shadowd.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "main/acpi.h"
#include "main/apic.h"
#include "main/interrupt.h"
#include "main/gdt.h"

#include "proc/sched.h"
#include "proc/proc.h"
#include "proc/kthread.h"

#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "drivers/disk/ata.h"
#include "drivers/tty/virtterm.h"
#include "drivers/pci.h"

#include "api/exec.h"
#include "api/syscall.h"

#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/fcntl.h"
#include "fs/stat.h"

#include "test/kshell/kshell.h"
#include "test/s5fs_test.h"

GDB_DEFINE_HOOK(boot)
GDB_DEFINE_HOOK(initialized)
GDB_DEFINE_HOOK(shutdown)

static void      *bootstrap(int arg1, void *arg2);
static void      *idleproc_run(int arg1, void *arg2);
static kthread_t *initproc_create(void);
static void      *initproc_run(int arg1, void *arg2);
static void       hard_shutdown(void);

static context_t bootstrap_context;
extern int gdb_wait;

extern void *sunghan_test(int, void*);
extern void *sunghan_deadlock_test(int, void*);
extern void *faber_thread_test(int, void*);

extern void *vfstest_main(int, void*);
extern int faber_fs_thread_test(kshell_t *ksh, int argc, char **argv);
extern int faber_directory_test(kshell_t *ksh, int argc, char **argv);

/**
 * This is the first real C function ever called. It performs a lot of
 * hardware-specific initialization, then creates a pseudo-context to
 * execute the bootstrap function in.
 */
void
kmain()
{
        GDB_CALL_HOOK(boot);

        dbg_init();
        dbgq(DBG_CORE, "Kernel binary:\n");
        dbgq(DBG_CORE, "  text: 0x%p-0x%p\n", &kernel_start_text, &kernel_end_text);
        dbgq(DBG_CORE, "  data: 0x%p-0x%p\n", &kernel_start_data, &kernel_end_data);
        dbgq(DBG_CORE, "  bss:  0x%p-0x%p\n", &kernel_start_bss, &kernel_end_bss);

        page_init();

        pt_init();
        slab_init();
        pframe_init();

        acpi_init();
        apic_init();
        pci_init();
        intr_init();

        gdt_init();

        /* initialize slab allocators */
#ifdef __VM__
        anon_init();
        shadow_init();
#endif
        vmmap_init();
        proc_init();
        kthread_init();

#ifdef __DRIVERS__
        bytedev_init();
        blockdev_init();
#endif

        void *bstack = page_alloc();
        pagedir_t *bpdir = pt_get();
        KASSERT(NULL != bstack && "Ran out of memory while booting.");
        /* This little loop gives gdb a place to synch up with weenix.  In the
         * past the weenix command started qemu was started with -S which
         * allowed gdb to connect and start before the boot loader ran, but
         * since then a bug has appeared where breakpoints fail if gdb connects
         * before the boot loader runs.  See
         *
         * https://bugs.launchpad.net/qemu/+bug/526653
         *
         * This loop (along with an additional command in init.gdb setting
         * gdb_wait to 0) sticks weenix at a known place so gdb can join a
         * running weenix, set gdb_wait to zero  and catch the breakpoint in
         * bootstrap below.  See Config.mk for how to set GDBWAIT correctly.
         *
         * DANGER: if GDBWAIT != 0, and gdb is not running, this loop will never
         * exit and weenix will not run.  Make SURE the GDBWAIT is set the way
         * you expect.
         */
        while (gdb_wait) ;
        context_setup(&bootstrap_context, bootstrap, 0, NULL, bstack, PAGE_SIZE, bpdir);
        context_make_active(&bootstrap_context);

        panic("\nReturned to kmain()!!!\n");
}

/**
 * Clears all interrupts and halts, meaning that we will never run
 * again.
 */
static void
hard_shutdown()
{
#ifdef __DRIVERS__
        vt_print_shutdown();
#endif
        __asm__ volatile("cli; hlt");
}

/**
 * This function is called from kmain, however it is not running in a
 * thread context yet. It should create the idle process which will
 * start executing idleproc_run() in a real thread context.  To start
 * executing in the new process's context call context_make_active(),
 * passing in the appropriate context. This function should _NOT_
 * return.
 *
 * Note: Don't forget to set curproc and curthr appropriately.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
bootstrap(int arg1, void *arg2)
{
        /* If the next line is removed/altered in your submission, 20 points will be deducted. */
        dbgq(DBG_TEST, "SIGNATURE: 53616c7465645f5ff8726377eea4fabb4159b64253e1f1c4e006f1a94be3d25e9f4e8b73e8f22a44d7c20831cae646c1\n");
        /* necessary to finalize page table information */
        pt_template_init();

        //NOT_YET_IMPLEMENTED("PROCS: bootstrap");
        curproc = proc_create("IDLE");
        KASSERT(NULL != curproc);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");

        KASSERT(PID_IDLE == curproc->p_pid);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");

        curthr = kthread_create(curproc, idleproc_run, NULL, NULL);
        KASSERT(NULL != curthr);
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");

        KASSERT(curproc && curthr && "Cannot create thread or process");
        dbg(DBG_PRINT, "(GRADING1A 1.a)\n");

        // make context active, call corresponding function --> kt->kt_ctx;
        context_make_active(&(curthr->kt_ctx));

        panic("weenix returned to bootstrap()!!! BAD!!!\n");
        return NULL;
}

/**
 * Once we're inside of idleproc_run(), we are executing in the context of the
 * first process-- a real context, so we can finally begin running
 * meaningful code.
 *
 * This is the body of process 0. It should initialize all that we didn't
 * already initialize in kmain(), launch the init process (initproc_run),
 * wait for the init process to exit, then halt the machine.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
idleproc_run(int arg1, void *arg2)
{
        int status;
        pid_t child;

        /* create init proc */
        kthread_t *initthr = initproc_create();
        init_call_all();
        GDB_CALL_HOOK(initialized);

        /* Create other kernel threads (in order) */

#ifdef __VFS__
        /* Once you have VFS remember to set the current working directory
         * of the idle and init processes */
        //NOT_YET_IMPLEMENTED("VFS: idleproc_run");
        /*
        set working directory
        */
        curproc->p_cwd = vfs_root_vn;
        vref(vfs_root_vn);

        initthr->kt_proc->p_cwd = vfs_root_vn;
        vref(vfs_root_vn);

        /* Here you need to make the null, zero, and tty devices using mknod */
        /* You can't do this until you have VFS, check the include/drivers/dev.h
         * file for macros with the device ID's you will need to pass to mknod */
        //NOT_YET_IMPLEMENTED("VFS: idleproc_run");
        /*
        call mkdir to create directory and mknod to make device files
        always start root directory
        call it dev directory
        */
        do_mkdir("/dev");
        dbg(DBG_PRINT, "(GRADING2A)\n");
        do_mknod("/dev/null", S_IFCHR, MKDEVID(1, 0));
        dbg(DBG_PRINT, "(GRADING2A)\n");
        do_mknod("/dev/zero", S_IFCHR, MKDEVID(1, 1));
        dbg(DBG_PRINT, "(GRADING2A)\n");
        do_mknod("/dev/tty0", S_IFCHR, MKDEVID(2, 0));
        dbg(DBG_PRINT, "(GRADING2A)\n");


#endif

        /* Finally, enable interrupts (we want to make sure interrupts
         * are enabled AFTER all drivers are initialized) */
        intr_enable();

        /* Run initproc */
        sched_make_runnable(initthr);
        /* Now wait for it */
        child = do_waitpid(-1, 0, &status);
        KASSERT(PID_INIT == child);

#ifdef __MTP__
        kthread_reapd_shutdown();
#endif


#ifdef __SHADOWD__
        /* wait for shadowd to shutdown */
        shadowd_shutdown();
#endif

#ifdef __VFS__
        /* Shutdown the vfs: */
        dbg_print("weenix: vfs shutdown...\n");
        vput(curproc->p_cwd);
        if (vfs_shutdown())
                panic("vfs shutdown FAILED!!\n");

#endif

        /* Shutdown the pframe system */
#ifdef __S5FS__
        pframe_shutdown();
#endif

        dbg_print("\nweenix: halted cleanly!\n");
        GDB_CALL_HOOK(shutdown);
        hard_shutdown();
        return NULL;
}

/**
 * This function, called by the idle process (within 'idleproc_run'), creates the
 * process commonly refered to as the "init" process, which should have PID 1.
 *
 * The init process should contain a thread which begins execution in
 * initproc_run().
 *
 * @return a pointer to a newly created thread which will execute
 * initproc_run when it begins executing
 */
static kthread_t *
initproc_create(void)
{
        //NOT_YET_IMPLEMENTED("PROCS: initproc_create");
        proc_t* initP;
        kthread_t* initThread;

        initP = proc_create("INIT");
        KASSERT(NULL != initP);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");
        KASSERT(PID_INIT == initP->p_pid);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");

        initThread = kthread_create(initP, initproc_run, NULL, NULL);
        KASSERT(NULL != initThread);
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");

        KASSERT(initP && initThread && "Cannot create thread or process");
        dbg(DBG_PRINT, "(GRADING1A 1.b)\n");

        return initThread;
}



#ifdef __DRIVERS__

static void* do_sunghan(kshell_t *kshell, int argc, char **argv)
{
    KASSERT(kshell != NULL);
    dbg(DBG_PRINT, "(GRADING1D 1): do_sunghan is invoked, argc = %d, argv = 0x%08x\n",
            argc, (unsigned int)argv);
    proc_t* p = proc_create("sunghan");
    kthread_t* k = kthread_create(p, sunghan_test, 0, NULL);

    int status;
    sched_make_runnable(k);
    do_waitpid(p->p_pid, 0, &status);

    dbg(DBG_PRINT, "(GRADING1D 1): do_sunghan is completed with status %d", status);
    return 0;
}  


static void* do_sunghan_deadlock(kshell_t *kshell, int argc, char **argv)
{
    KASSERT(kshell != NULL);
    dbg(DBG_PRINT, "(GRADING1D 2): do_sunghan_deadlock is invoked, argc = %d, argv = 0x%08x\n",
            argc, (unsigned int)argv);
    proc_t* p = proc_create("deadlock");
    kthread_t* k = kthread_create(p, sunghan_deadlock_test, 0, NULL);

    int status;
    sched_make_runnable(k);
    do_waitpid(p->p_pid, 0, &status);

    return 0;
}


static void* do_faber(kshell_t *kshell, int argc, char **argv)
{

    KASSERT(kshell != NULL);
    dbg(DBG_PRINT, "(GRADING1C): do_faber is invoked, argc = %d, argv = 0x%08x\n",
            argc, (unsigned int)argv);
    proc_t* p = proc_create("faber");
    kthread_t* k = kthread_create(p, faber_thread_test, 0, NULL);

    int status;
    sched_make_runnable(k);
    do_waitpid(p->p_pid, 0, &status);
    
    dbg(DBG_PRINT, "(GRADING1C): do_faber is completed with status %d", status);
    return 0;
}


static void* do_vfs_test(kshell_t *kshell, int argc, char **argv)
{

    proc_t* p = proc_create("vfstest");
    dbg(DBG_PRINT, "(GRADING2B)\n");
    kthread_t* k = kthread_create(p, vfstest_main, 1, NULL);

    int status;
    dbg(DBG_PRINT, "(GRADING2B)\n");
    sched_make_runnable(k);
    dbg(DBG_PRINT, "(GRADING2B)\n");
    do_waitpid(p->p_pid, 0, &status);
    dbg(DBG_PRINT, "(GRADING2B)\n");
    
    return 0;

}

#endif /* __DRIVERS__ */



/**
 * The init thread's function changes depending on how far along your Weenix is
 * developed. Before VM/FI, you'll probably just want to have this run whatever
 * tests you've written (possibly in a new process). After VM/FI, you'll just
 * exec "/sbin/init".
 *
 * Both arguments are unused.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
static void *
initproc_run(int arg1, void *arg2)
{
        //NOT_YET_IMPLEMENTED("PROCS: initproc_run");

        int status;

#ifdef __DRIVERS__

        kshell_add_command("sunghan", (kshell_cmd_func_t)&do_sunghan, "sunghan test");
        dbg(DBG_PRINT, "(GRADING1B)");

        kshell_add_command("deadlock", (kshell_cmd_func_t)&do_sunghan_deadlock, "sunghan deadlock test");
        dbg(DBG_PRINT, "(GRADING1B)");

        kshell_add_command("faber", (kshell_cmd_func_t)&do_faber, "faber test");
        dbg(DBG_PRINT, "(GRADING1B)");

#ifdef __VFS__

        kshell_add_command("vfstest", (kshell_cmd_func_t)&do_vfs_test, "vfs test");
        dbg(DBG_PRINT, "(GRADING2B)\n");
        kshell_add_command("thrtest", (kshell_cmd_func_t)&faber_fs_thread_test, "Run faber_fs_thread_test().");
        dbg(DBG_PRINT, "(GRADING2C 1)\n");
        kshell_add_command("dirtest", (kshell_cmd_func_t)&faber_directory_test, "Run faber_directory_test().");
        dbg(DBG_PRINT, "(GRADING2C 2)\n");

#endif

        kshell_t *kshell = kshell_create(0);
        if (NULL == kshell){
            dbg(DBG_PRINT, "(GRADING1B)");
            panic("init: Couldn't create kernel shell\n");
        }

        while (kshell_execute_next(kshell)){
            dbg(DBG_PRINT, "(GRADING1B)");
        }

        kshell_destroy(kshell);
        dbg(DBG_PRINT, "(GRADING1B)");

#endif /* __DRIVERS__ */

        while (do_waitpid(-1,0, &status) != -ECHILD);

        return NULL;
}
