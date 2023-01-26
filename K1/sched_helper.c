#include "globals.h"
#include "errno.h"

#include "main/interrupt.h"

#include "proc/sched.h"
#include "proc/kthread.h"

#include "util/init.h"
#include "util/debug.h"

void ktqueue_enqueue(ktqueue_t *q, kthread_t *thr);
kthread_t * ktqueue_dequeue(ktqueue_t *q);

/*
 * Updates the thread's state and enqueues it on the given
 * queue. Returns when the thread has been woken up with wakeup_on or
 * broadcast_on.
 *
 * Use the private queue manipulation functions above.
 */
void
sched_sleep_on(ktqueue_t *q)
{
        //NOT_YET_IMPLEMENTED("PROCS: sched_sleep_on");
        curthr->kt_state = KT_SLEEP;
        ktqueue_enqueue(q,curthr);
        dbg(DBG_PRINT, "(GRADING1A 4)\n");
        sched_switch();

}

kthread_t *
sched_wakeup_on(ktqueue_t *q)
{
        //NOT_YET_IMPLEMENTED("PROCS: sched_wakeup_on");

        if (!sched_queue_empty(q)) {
            kthread_t * kthread = ktqueue_dequeue(q);
            KASSERT((kthread->kt_state == KT_SLEEP) || (kthread->kt_state == KT_SLEEP_CANCELLABLE));
            dbg(DBG_PRINT, "(GRADING1A 4.a)\n");
            sched_make_runnable(kthread);
            dbg(DBG_PRINT, "(GRADING1A 4)\n");
            return kthread;    
        }

        dbg(DBG_PRINT, "(GRADING1A 4)\n");
        return NULL;
}

void
sched_broadcast_on(ktqueue_t *q)
{
        //NOT_YET_IMPLEMENTED("PROCS: sched_broadcast_on");
        dbg(DBG_PRINT, "(GRADING1A 4)\n");
        while(!sched_queue_empty(q)) {
			kthread_t * kthread = ktqueue_dequeue(q);
			sched_make_runnable(kthread);
            dbg(DBG_PRINT, "(GRADING1A 4)\n");
        }
        dbg(DBG_PRINT, "(GRADING1A 4)\n");
}

