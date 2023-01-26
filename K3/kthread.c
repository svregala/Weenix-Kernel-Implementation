#include "config.h"
#include "globals.h"

#include "errno.h"

#include "util/init.h"
#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"

#include "mm/slab.h"
#include "mm/page.h"

kthread_t *curthr; /* global */
static slab_allocator_t *kthread_allocator = NULL;

#ifdef __MTP__
/* Stuff for the reaper daemon, which cleans up dead detached threads */
static proc_t *reapd = NULL;
static kthread_t *reapd_thr = NULL;
static ktqueue_t reapd_waitq;
static list_t kthread_reapd_deadlist; /* Threads to be cleaned */

static void *kthread_reapd_run(int arg1, void *arg2);
#endif

void
kthread_init()
{
        kthread_allocator = slab_allocator_create("kthread", sizeof(kthread_t));
        KASSERT(NULL != kthread_allocator);
}

/**
 * Allocates a new kernel stack.
 *
 * @return a newly allocated stack, or NULL if there is not enough
 * memory available
 */
static char *
alloc_stack(void)
{
        /* extra page for "magic" data */
        char *kstack;
        int npages = 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT);
        kstack = (char *)page_alloc_n(npages);

        return kstack;
}

/**
 * Frees a stack allocated with alloc_stack.
 *
 * @param stack the stack to free
 */
static void
free_stack(char *stack)
{
        page_free_n(stack, 1 + (DEFAULT_STACK_SIZE >> PAGE_SHIFT));
}

void
kthread_destroy(kthread_t *t)
{
        KASSERT(t && t->kt_kstack);
        free_stack(t->kt_kstack);
        if (list_link_is_linked(&t->kt_plink))
                list_remove(&t->kt_plink);

        slab_obj_free(kthread_allocator, t);
}

/*
 * Allocate a new stack with the alloc_stack function. The size of the
 * stack is DEFAULT_STACK_SIZE.
 *
 * Don't forget to initialize the thread context with the
 * context_setup function. The context should have the same pagetable
 * pointer as the process.
 */
kthread_t *
kthread_create(struct proc *p, kthread_func_t func, long arg1, void *arg2)
{
        //NOT_YET_IMPLEMENTED("PROCS: kthread_create");
        //return NULL;

        KASSERT(NULL != p);
        dbg(DBG_PRINT, "(GRADING1A 3.a)\n");

        kthread_t* kthread = (kthread_t*)slab_obj_alloc(kthread_allocator);
        kthread->kt_kstack = alloc_stack();
        kthread->kt_retval = (void *)0;
        kthread->kt_errno = 0;
        kthread->kt_proc = p;
        kthread->kt_cancelled = 0;
        kthread->kt_state = KT_RUN;
        kthread->kt_wchan = NULL;

        list_link_init(&(kthread->kt_qlink));
        list_link_init(&(kthread->kt_plink));
        list_insert_tail(&(p->p_threads), &(kthread->kt_plink));
        context_setup(&kthread->kt_ctx, func, arg1, arg2, kthread->kt_kstack, DEFAULT_STACK_SIZE, p->p_pagedir);

        dbg(DBG_PRINT, "(GRADING1A 3)\n");
        return kthread;
}

/*
 * If the thread to be cancelled is the current thread, this is
 * equivalent to calling kthread_exit. Otherwise, the thread is
 * sleeping (either on a waitqueue or a runqueue) 
 * and we need to set the cancelled and retval fields of the
 * thread. On wakeup, threads should check their cancelled fields and
 * act accordingly. 
 *
 * If the thread's sleep is cancellable, cancelling the thread should
 * wake it up from sleep.
 *
 * If the thread's sleep is not cancellable, we do nothing else here.
 *
 */
void
kthread_cancel(kthread_t *kthr, void *retval)
{
        //NOT_YET_IMPLEMENTED("PROCS: kthread_cancel");

        KASSERT(NULL != kthr);
        dbg(DBG_PRINT, "(GRADING1A 3.b)\n");
        dbg(DBG_PRINT, "(GRADING1C)\n");    // change argument in consecutive call

        kthr->kt_cancelled = 1;
        if(kthr == curthr){
                dbg(DBG_PRINT, "(GRADING1A 3)\n");
                kthread_exit(retval);
        }
        else{
                kthr->kt_retval = retval;
                sched_cancel(kthr);
                dbg(DBG_PRINT, "(GRADING1A 3)\n");
        }
}

/*
 * You need to set the thread's retval field and alert the current
 * process that a thread is exiting via proc_thread_exited. You should
 * refrain from setting the thread's state to KT_EXITED until you are
 * sure you won't make any more blocking calls before you invoke the
 * scheduler again.
 *
 * It may seem unneccessary to push the work of cleaning up the thread
 * over to the process. However, if you implement MTP, a thread
 * exiting does not necessarily mean that the process needs to be
 * cleaned up.
 *
 * The void * type of retval is simply convention and does not necessarily
 * indicate that retval is a pointer
 */
void
kthread_exit(void *retval)
{
        //NOT_YET_IMPLEMENTED("PROCS: kthread_exit");

        KASSERT(!curthr->kt_wchan);
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");
        KASSERT(!curthr->kt_qlink.l_next && !curthr->kt_qlink.l_prev);
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");
        KASSERT(curthr->kt_proc == curproc);
        dbg(DBG_PRINT, "(GRADING1A 3.c)\n");

        curthr->kt_retval = retval;
        curthr->kt_state = KT_EXITED;
        dbg(DBG_PRINT, "(GRADING1A 3)\n");
        proc_thread_exited(retval);
}

/*
 * The new thread will need its own context and stack. Think carefully
 * about which fields should be copied and which fields should be
 * freshly initialized.
 *
 * You do not need to worry about this until VM.
 */
kthread_t *
kthread_clone(kthread_t *thr)
{
        //NOT_YET_IMPLEMENTED("VM: kthread_clone");
        //return NULL;

        kthread_t* clone_dummy = NULL;
        kthread_t* temp = NULL;

        kthread_t* clone_thr = (kthread_t*) slab_obj_alloc(kthread_allocator);
        clone_thr->kt_kstack = alloc_stack();

        pagedir_t* thr_dir = thr->kt_proc->p_pagedir;

        context_setup(&(clone_thr->kt_ctx),NULL,0,NULL,clone_thr->kt_kstack,DEFAULT_STACK_SIZE,thr_dir);

        clone_thr->kt_proc = NULL;
        clone_thr->kt_state = thr->kt_state;
        clone_thr->kt_retval = thr->kt_retval;
        clone_thr->kt_cancelled = thr->kt_cancelled;
        clone_thr->kt_wchan = thr->kt_wchan;
        clone_thr->kt_errno = thr->kt_errno;

        list_link_init(&clone_thr->kt_qlink);
        list_link_init(&clone_thr->kt_plink);

        return clone_thr;

}

/*
 * The following functions will be useful if you choose to implement
 * multiple kernel threads per process. This is strongly discouraged
 * unless your weenix is perfect.
 */
#ifdef __MTP__
int
kthread_detach(kthread_t *kthr)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_detach");
        return 0;
}

int
kthread_join(kthread_t *kthr, void **retval)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_join");
        return 0;
}

/* ------------------------------------------------------------------ */
/* -------------------------- REAPER DAEMON ------------------------- */
/* ------------------------------------------------------------------ */
static __attribute__((unused)) void
kthread_reapd_init()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_init");
}
init_func(kthread_reapd_init);
init_depends(sched_init);

void
kthread_reapd_shutdown()
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_shutdown");
}

static void *
kthread_reapd_run(int arg1, void *arg2)
{
        NOT_YET_IMPLEMENTED("MTP: kthread_reapd_run");
        return (void *) 0;
}
#endif
