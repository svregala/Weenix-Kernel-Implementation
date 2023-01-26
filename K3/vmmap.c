#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"

#include "mm/tlb.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_create");
        //return NULL;

        vmmap_t* map;
        map = (vmmap_t*)slab_obj_alloc(vmmap_allocator);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        if(map!=NULL){
            list_init(&(map->vmm_list));
            map->vmm_proc = NULL;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return map;
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_destroy");
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");
        vmarea_t* iter = NULL;
        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink) {

            list_remove(&(iter->vma_olink));
            list_remove(&(iter->vma_plink));
            iter->vma_obj->mmo_ops->put(iter->vma_obj);
            iter->vma_obj=NULL;
            vmarea_free(iter);
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        } list_iterate_end();

        slab_obj_free(vmmap_allocator, map);
        dbg(DBG_PRINT, "(GRADING3A 3)\n");
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_insert");
        KASSERT(NULL != map && NULL != newvma);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        KASSERT(NULL == newvma->vma_vmmap);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        KASSERT(newvma->vma_start < newvma->vma_end); 
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");

        vmarea_t* iter = NULL;
        vmarea_t* holder = NULL;
        newvma->vma_vmmap = map;

        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink) {

            if(newvma->vma_start <= iter->vma_start){
                holder = iter;
                list_insert_tail(&holder->vma_plink, &newvma->vma_plink);
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
                return;
            }
            dbg(DBG_PRINT, "(GRADING3A 3)\n");

        } list_iterate_end();

        list_insert_tail(&map->vmm_list, &newvma->vma_plink);
        dbg(DBG_PRINT, "(GRADING3A 3)\n");
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_find_range");
        //return -1;

        vmarea_t* iter = NULL;
        vmarea_t* holder = NULL;

        int retVal = -1;

        if(dir == VMMAP_DIR_HILO){
            
            list_iterate_reverse(&map->vmm_list, iter, vmarea_t, vma_plink) {

                if(holder == NULL && npages <= ADDR_TO_PN(USER_MEM_HIGH) - iter->vma_end){
                    retVal = ADDR_TO_PN(USER_MEM_HIGH) - npages;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    break;

                }else if(holder != NULL && npages <= (holder->vma_start-iter->vma_end)){
                    retVal = holder->vma_start-npages;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    break;
                }else{
                    holder = iter;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                dbg(DBG_PRINT, "(GRADING3A)\n");

            } list_iterate_end();

            if(retVal<0 && npages <= (holder->vma_start - ADDR_TO_PN(USER_MEM_LOW))){
                    retVal = holder->vma_start-npages;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
            }

            dbg(DBG_PRINT, "(GRADING3A)\n");

        }else if(dir == VMMAP_DIR_LOHI){

            list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink){

                if(holder==NULL && npages <= (iter->vma_start - ADDR_TO_PN(USER_MEM_LOW))){
                    retVal = ADDR_TO_PN(USER_MEM_LOW);
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    break;
                }else if(holder != NULL && npages <= (iter->vma_start - holder->vma_end)){
                    retVal = holder->vma_end;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    break;
                }else{
                    holder = iter;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                dbg(DBG_PRINT, "(GRADING3A)\n");
            } list_iterate_end();

            if(retVal<0){
                if(holder==NULL && npages <= (ADDR_TO_PN(USER_MEM_HIGH) - ADDR_TO_PN(USER_MEM_LOW))){
                    retVal = ADDR_TO_PN(USER_MEM_LOW);
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }else if(npages <= (ADDR_TO_PN(USER_MEM_HIGH)-holder->vma_end)){
                    retVal = holder->vma_end;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
            }

            dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        dbg(DBG_PRINT, "(GRADING3A)\n");

        return retVal;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_lookup");
        //return NULL;
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");

        vmarea_t* iter = NULL;

        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink) {
            if(vfn >= iter->vma_start && vfn < iter->vma_end){
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
                return iter;
            }
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        } list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3A 3)\n");
        return NULL;
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_clone");
        //return NULL;
        vmmap_t* ret_map= vmmap_create();
        vmarea_t* iter = NULL;

        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink) {

            dbg(DBG_PRINT, "(GRADING3A)\n");

            vmarea_t* area = vmarea_alloc();

            area->vma_off = iter->vma_off;
            area->vma_start = iter->vma_start;
            area->vma_end = iter->vma_end;

            area->vma_flags = iter->vma_flags;
            area->vma_prot = iter->vma_prot;
            area->vma_vmmap = ret_map;

            list_link_init(&area->vma_olink);
            list_link_init(&area->vma_plink);

            list_insert_tail(&ret_map->vmm_list, &area->vma_plink);

            dbg(DBG_PRINT, "(GRADING3A)\n");

        } list_iterate_end();

        dbg(DBG_PRINT, "(GRADING3A)\n");

        return ret_map;
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_map");
        //return -1;

        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        KASSERT(0 < npages);
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        KASSERT(PAGE_ALIGNED(off));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");

        vmarea_t* area = vmarea_alloc();

        if(lopage != 0){
            int res = vmmap_is_range_empty(map, lopage, npages);
            if(res == 0){
                vmmap_remove(map, lopage, npages);
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
            }
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        } 
        else if(lopage==0){
            int result = vmmap_find_range(map, npages, dir);

            if(result != -1){
                lopage = result;
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
            }else{
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
                return result;
            }
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }

        area->vma_off = ADDR_TO_PN(off);
        area->vma_start = lopage;
        area->vma_end = lopage + npages;

        area->vma_obj = NULL;
        area->vma_prot = prot;
        area->vma_flags = flags;

        list_link_init(&area->vma_olink);
        list_link_init(&area->vma_plink);

        mmobj_t* obj = NULL;

        if(file){
            int res = file->vn_ops->mmap(file, area, &obj);
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }else{
            obj = anon_create();
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }

        if(!(flags & MAP_PRIVATE)){
            area->vma_obj = obj;
            list_insert_head(&(obj->mmo_un.mmo_vmas), &(area->vma_olink));
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }
        else if(flags & MAP_PRIVATE){
            mmobj_t* shad_obj = shadow_create();
            shad_obj->mmo_shadowed = obj;

            area->vma_obj = shad_obj;

            mmobj_t* new_bottom = obj;

            shad_obj->mmo_un.mmo_bottom_obj = new_bottom;

            list_insert_head( &(new_bottom->mmo_un.mmo_vmas), &(area->vma_olink));
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }

        vmmap_insert(map, area);

        if(new != NULL){
            *new = area;
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
            return 0;
        }

        dbg(DBG_PRINT, "(GRADING3A 3)\n");

        return 0;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_remove");
        //return -1;

        vmarea_t* iter = NULL;
        unsigned int sum_pages = lopage + npages;

        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink) {
            dbg(DBG_PRINT, "(GRADING3A)\n");
            if((lopage > iter->vma_start)){
                dbg(DBG_PRINT, "(GRADING3A)\n");
                if(sum_pages < iter->vma_end){
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                    vmarea_t* area = vmarea_alloc();

                    area->vma_off = iter->vma_off + sum_pages - iter->vma_start;
                    area->vma_start = sum_pages;
                    area->vma_end = iter->vma_end;

                    area->vma_prot = iter->vma_prot;
                    area->vma_flags = iter->vma_flags;
                    area->vma_vmmap = map;
                    area->vma_obj = iter->vma_obj;

                    iter->vma_end = lopage;

                    vmarea_t* new_area = list_item( (iter->vma_plink).l_next, vmarea_t, vma_plink );
                    list_insert_before( &(new_area->vma_plink), &(area->vma_plink) );

                    mmobj_t* object = iter->vma_obj;
                    mmobj_t* first_shad = shadow_create();
                    mmobj_t* second_shad = shadow_create();

                    iter->vma_obj = first_shad;
                    area->vma_obj = second_shad;
                    area->vma_obj->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(iter->vma_obj);
                    iter->vma_obj->mmo_un.mmo_bottom_obj = area->vma_obj->mmo_un.mmo_bottom_obj;
                    iter->vma_obj->mmo_shadowed = object;
                    area->vma_obj->mmo_shadowed = iter->vma_obj->mmo_shadowed;
                    object->mmo_ops->ref(object);

                    list_insert_tail(mmobj_bottom_vmas(object),&(area->vma_olink));

                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }

                else if((lopage < iter->vma_end) && (sum_pages >= iter->vma_end)){
                    iter->vma_end = lopage;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                dbg(DBG_PRINT, "(GRADING3A)\n");
            } 
            else {
                if((sum_pages >= iter->vma_end)){
                    list_remove(&(iter->vma_olink));
                    list_remove(&(iter->vma_plink));
                    iter->vma_obj->mmo_ops->put(iter->vma_obj);
                    iter->vma_obj = NULL;
                    vmarea_free(iter);
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                else if((sum_pages < iter->vma_end) && (sum_pages > iter->vma_start)){
                    iter->vma_off = iter->vma_off + sum_pages - iter->vma_start;
                    iter->vma_start = sum_pages;
                    dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                dbg(DBG_PRINT, "(GRADING3A)\n");
            }
            dbg(DBG_PRINT, "(GRADING3A)\n");
        } list_iterate_end();

        tlb_flush_all();
        pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(lopage), (uintptr_t)PN_TO_ADDR(sum_pages));
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_is_range_empty");
        //return 0;

        vmarea_t* iter = NULL;
        unsigned int sum_pages = startvfn+npages;
        unsigned int endvfn = startvfn+npages;

        KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
        dbg(DBG_PRINT, "(GRADING3A 3.e)\n");

        list_iterate_begin(&map->vmm_list, iter, vmarea_t, vma_plink){
            if(iter->vma_end > startvfn){
                if(sum_pages<=iter->vma_start){
                    dbg(DBG_PRINT, "(GRADING3A 3)\n");
                    continue;
                }else{
                    dbg(DBG_PRINT, "(GRADING3A 3)\n");
                    return 0;
                }
                dbg(DBG_PRINT, "(GRADING3A 3)\n");
            }
            dbg(DBG_PRINT, "(GRADING3A 3)\n");
        }list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3A 3)\n");
        return 1;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_read");
        //return 0;
        dbg(DBG_PRINT, "(GRADING3A)\n");

        if(ADDR_TO_PN(vaddr)==ADDR_TO_PN((int)vaddr+count)){
            vmarea_t* area = vmmap_lookup(map, ADDR_TO_PN(vaddr));
            pframe_t* frame = NULL;

            int result = pframe_lookup(area->vma_obj, (ADDR_TO_PN(vaddr)-area->vma_start+area->vma_off), 1, &frame);
            memcpy(buf, ((char*)frame->pf_addr)+PAGE_OFFSET(vaddr), count);

            dbg(DBG_PRINT, "(GRADING3A)\n");

            return 0;
        }

        unsigned int curr_page = ADDR_TO_PN(vaddr);
        int vResult=0;

        if(curr_page <= ADDR_TO_PN((int)vaddr+count)){

            vmarea_t* vArea = vmmap_lookup(map, curr_page);
            pframe_t* fill_frame = NULL;

            vResult = pframe_lookup(vArea->vma_obj, curr_page-vArea->vma_start+vArea->vma_off, 1, &fill_frame);
            memcpy(buf, ((char*)fill_frame->pf_addr)+PAGE_OFFSET(vaddr), PAGE_SIZE-PAGE_OFFSET(vaddr));
            buf = ((char*)buf+PAGE_SIZE-PAGE_OFFSET(vaddr));

            curr_page=curr_page+1;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        while(curr_page <= ADDR_TO_PN((int)vaddr+count)){

            vmarea_t* vArea = vmmap_lookup(map, curr_page);
            pframe_t* fill_frame=NULL;

            vResult = pframe_lookup(vArea->vma_obj, curr_page-vArea->vma_start+vArea->vma_off, 1, &fill_frame);
            memcpy(buf, fill_frame->pf_addr, PAGE_OFFSET((int)vaddr+count));
            buf = ((char*)buf+PAGE_OFFSET((int)vaddr+count));

            curr_page=curr_page+1;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        if(vResult==0){
            dbg(DBG_PRINT, "(GRADING3A)\n");
            return vResult;
        }

        dbg(DBG_PRINT, "(GRADING3A)\n");

        return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        //NOT_YET_IMPLEMENTED("VM: vmmap_write");
        //return 0;
        dbg(DBG_PRINT, "(GRADING3A)\n");

        if(ADDR_TO_PN(vaddr)==ADDR_TO_PN((int)vaddr-1+count)){

            vmarea_t* area = vmmap_lookup(map, ADDR_TO_PN(vaddr));
            pframe_t* frame = NULL;

            int result = pframe_lookup(area->vma_obj, ADDR_TO_PN(vaddr)-area->vma_start+area->vma_off, 1, &frame);
            memcpy(((char*)frame->pf_addr)+PAGE_OFFSET(vaddr), buf, count);
            pframe_dirty(frame);

            dbg(DBG_PRINT, "(GRADING3A)\n");

            return 0;
        }

        unsigned int curr_page = ADDR_TO_PN(vaddr);
        int vResult=0;

        if(curr_page<=ADDR_TO_PN((int)vaddr-1+count)){
            vmarea_t* vArea = vmmap_lookup(map, curr_page);
            pframe_t* fill_frame = NULL;

            vResult = pframe_lookup(vArea->vma_obj, curr_page-vArea->vma_start+vArea->vma_off, 1, &fill_frame);
            memcpy(((char*)fill_frame->pf_addr)+PAGE_OFFSET(vaddr), buf, PAGE_SIZE-PAGE_OFFSET(vaddr));
            buf = ((char*)buf+PAGE_SIZE-PAGE_OFFSET(vaddr));

            pframe_dirty(fill_frame);
            curr_page=curr_page+1;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }


        while(curr_page <= ADDR_TO_PN((int)vaddr-1+count)){

            vmarea_t* vArea = vmmap_lookup(map, curr_page);
            pframe_t* fill_frame=NULL;

            vResult = pframe_lookup(vArea->vma_obj, curr_page-vArea->vma_start+vArea->vma_off, 1, &fill_frame);
            memcpy(fill_frame->pf_addr, buf, PAGE_OFFSET((int)vaddr-1+count));
            buf = ((char*)buf+PAGE_OFFSET((int)vaddr-1+count));

            pframe_dirty(fill_frame);
            curr_page=curr_page+1;
            dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        if(vResult==0){
            dbg(DBG_PRINT, "(GRADING3A)\n");
            return vResult;
        }

        dbg(DBG_PRINT, "(GRADING3A)\n");

        return 0;
}