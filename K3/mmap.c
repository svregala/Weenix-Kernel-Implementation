#include "globals.h"
#include "errno.h"
#include "types.h"

#include "mm/mm.h"
#include "mm/tlb.h"
#include "mm/mman.h"
#include "mm/page.h"

#include "proc/proc.h"

#include "util/string.h"
#include "util/debug.h"

#include "fs/vnode.h"
#include "fs/vfs.h"
#include "fs/file.h"

#include "vm/vmmap.h"
#include "vm/mmap.h"

/*
 * This function implements the mmap(2) syscall, but only
 * supports the MAP_SHARED, MAP_PRIVATE, MAP_FIXED, and
 * MAP_ANON flags.
 *
 * Add a mapping to the current process's address space.
 * You need to do some error checking; see the ERRORS section
 * of the manpage for the problems you should anticipate.
 * After error checking most of the work of this function is
 * done by vmmap_map(), but remember to clear the TLB.
 */
int
do_mmap(void *addr, size_t len, int prot, int flags,
        int fd, off_t off, void **ret)
{
        //NOT_YET_IMPLEMENTED("VM: do_mmap");
        //return -1;

		vmarea_t* area = NULL;
		int result;

		if(!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)){
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
			return -EINVAL;
		}

		if((flags & MAP_SHARED) && (flags & MAP_PRIVATE)){
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
			return -EINVAL;
		}

		if(len > (USER_MEM_HIGH-USER_MEM_LOW) || len <=0){
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
			return -EINVAL;
		}

		if(!PAGE_ALIGNED(off)){
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
			return -EINVAL;
		}

		uint32_t addr_var;
		unsigned int ad = (unsigned int)addr;

		if(!(flags & MAP_FIXED)){
			addr_var = 0;
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
		}else{
			if(USER_MEM_HIGH < ad + len){
				dbg(DBG_PRINT, "(GRADING3A 2)\n");
				return -EINVAL;
			}
			if(USER_MEM_LOW > ad){
				dbg(DBG_PRINT, "(GRADING3A 2)\n");
				return -EINVAL;
			}
			
			addr_var = ADDR_TO_PN(addr);
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
		}


		if(flags & MAP_ANON){
			result = vmmap_map(curproc->p_vmmap, 0, addr_var, ((len-1)/PAGE_SIZE + 1), prot, flags, off, VMMAP_DIR_HILO, &area);
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
		}else{
			if(fd<0 || fd>NFILES){
				dbg(DBG_PRINT, "(GRADING3A 2)\n");
				return -EBADF;
			}

			file_t* ourFile = fget(fd);

			if(!ourFile){
				dbg(DBG_PRINT, "(GRADING3A 2)\n");
				return -EBADF;
			}

			if((flags & MAP_SHARED)){
				if((prot & PROT_WRITE)){
					if((ourFile->f_mode == FMODE_READ)){
						fput(ourFile);
						dbg(DBG_PRINT, "(GRADING3A 2)\n");
						return -EINVAL;
					}
					dbg(DBG_PRINT, "(GRADING3A 2)\n");
				}
				dbg(DBG_PRINT, "(GRADING3A 2)\n");
			}
			result = vmmap_map(curproc->p_vmmap, ourFile->f_vnode, addr_var, ((len-1)/PAGE_SIZE + 1), prot, flags, off, VMMAP_DIR_HILO, &area);
			fput(ourFile);
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
		}

		if(result < 0){
			dbg(DBG_PRINT, "(GRADING3A 2)\n");
			return result;
		}

		*ret = PN_TO_ADDR(area->vma_start);
		pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(area->vma_start), (uintptr_t)PN_TO_ADDR(area->vma_start) + (uintptr_t)PAGE_ALIGN_UP(len));

		KASSERT(NULL != curproc->p_pagedir);

		tlb_flush_range((uintptr_t)PN_TO_ADDR(area->vma_start), (uint32_t)PAGE_ALIGN_UP(len)/PAGE_SIZE);

		dbg(DBG_PRINT, "(GRADING3A 2.a)\n");

		return result;
}


/*
 * This function implements the munmap(2) syscall.
 *
 * As with do_mmap() it should perform the required error checking,
 * before calling upon vmmap_remove() to do most of the work.
 * Remember to clear the TLB.
 */
int
do_munmap(void *addr, size_t len)
{
        //NOT_YET_IMPLEMENTED("VM: do_munmap");
        //return -1;

		unsigned int addr_var = ADDR_TO_PN(addr);

		if((len<=0) || (len>(USER_MEM_HIGH-USER_MEM_LOW)) || ((unsigned int)addr + len) > USER_MEM_HIGH || ((unsigned int)addr < USER_MEM_LOW)){
			dbg(DBG_PRINT, "(GRADING3A)\n");
			return -EINVAL;
		}

		int result = vmmap_remove(curproc->p_vmmap, addr_var, (len-1)/PAGE_SIZE + 1);
		if(result < 0){
			dbg(DBG_PRINT, "(GRADING3A)\n");
			return result;
		}
		tlb_flush_all();
		dbg(DBG_PRINT, "(GRADING3A)\n");
		return 0;
}