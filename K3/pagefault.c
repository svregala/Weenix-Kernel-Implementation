#include "types.h"
#include "globals.h"
#include "kernel.h"
#include "errno.h"

#include "util/debug.h"

#include "proc/proc.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/pagetable.h"

#include "vm/pagefault.h"
#include "vm/vmmap.h"

#include "mm/tlb.h"

/*
 * This gets called by _pt_fault_handler in mm/pagetable.c The
 * calling function has already done a lot of error checking for
 * us. In particular it has checked that we are not page faulting
 * while in kernel mode. Make sure you understand why an
 * unexpected page fault in kernel mode is bad in Weenix. You
 * should probably read the _pt_fault_handler function to get a
 * sense of what it is doing.
 *
 * Before you can do anything you need to find the vmarea that
 * contains the address that was faulted on. Make sure to check
 * the permissions on the area to see if the process has
 * permission to do [cause]. If either of these checks does not
 * pass kill the offending process, setting its exit status to
 * EFAULT (normally we would send the SIGSEGV signal, however
 * Weenix does not support signals).
 *
 * Now it is time to find the correct page. Make sure that if the
 * user writes to the page it will be handled correctly. This
 * includes your shadow objects' copy-on-write magic working
 * correctly.
 *
 * Finally call pt_map to have the new mapping placed into the
 * appropriate page table.
 *
 * @param vaddr the address that was accessed to cause the fault
 *
 * @param cause this is the type of operation on the memory
 *              address which caused the fault, possible values
 *              can be found in pagefault.h
 */
void
handle_pagefault(uintptr_t vaddr, uint32_t cause)
{
        //NOT_YET_IMPLEMENTED("VM: handle_pagefault");

	uint32_t pn = ADDR_TO_PN(vaddr);
    vmarea_t *vmarea = vmmap_lookup(curproc->p_vmmap, pn);
    dbg(DBG_PRINT, "(GRADING3A 5)\n");
    if(vmarea != NULL && !(vmarea->vma_prot & PROT_WRITE) && (cause & FAULT_WRITE)){
        dbg(DBG_PRINT, "(GRADING3A 5)\n");
        do_exit(EFAULT);
    }
    else if(vmarea != NULL && !(vmarea->vma_prot & PROT_READ) && !(cause & FAULT_EXEC) && !(cause & FAULT_WRITE)){
        dbg(DBG_PRINT, "(GRADING3A 5)\n");
        do_exit(EFAULT);
    }
    else if(vmarea == NULL){
        dbg(DBG_PRINT, "(GRADING3A 5)\n");
        do_exit(EFAULT);
    }
    else{
        int fw=0;
        uint32_t pd = PD_PRESENT | PD_USER;
        uint32_t pt = PT_PRESENT | PT_USER;
        pframe_t *pf;
        if(cause & FAULT_WRITE){
            pd = pd | PD_WRITE;
            pt = pt | PT_WRITE;
            fw=1;
            dbg(DBG_PRINT, "(GRADING3A 5)\n");
        }
        int lookUpResult = pframe_lookup(vmarea->vma_obj, pn - vmarea->vma_start + vmarea->vma_off, fw, &pf);
        if(lookUpResult < 0){
            dbg(DBG_PRINT, "(GRADING3A 5)\n");
            do_exit(EFAULT);
        }
        else{
            KASSERT(pf);
            dbg(DBG_PRINT, "(GRADING3A 5.a)\n");
            KASSERT(pf->pf_addr);
            dbg(DBG_PRINT, "(GRADING3A 5.a)\n");

            if(fw){
                pframe_pin(pf);
                pframe_dirty(pf);
                pframe_unpin(pf);
                dbg(DBG_PRINT, "(GRADING3A 5)\n");
            }
            uintptr_t paddr = (uintptr_t) pt_virt_to_phys((uintptr_t) pf->pf_addr);
            int mapResult = pt_map(curproc->p_pagedir, (uintptr_t) PAGE_ALIGN_DOWN(vaddr), paddr, pd, pt);
            tlb_flush((uintptr_t) PAGE_ALIGN_DOWN(vaddr));
            dbg(DBG_PRINT, "(GRADING3A 5)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A 5)\n");

    }
    dbg(DBG_PRINT, "(GRADING3A 5)\n");

}
