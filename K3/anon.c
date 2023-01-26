#include "globals.h"
#include "errno.h"

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

int anon_count = 0; /* for debugging/verification purposes */

static slab_allocator_t *anon_allocator;

static void anon_ref(mmobj_t *o);
static void anon_put(mmobj_t *o);
static int  anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  anon_fillpage(mmobj_t *o, pframe_t *pf);
static int  anon_dirtypage(mmobj_t *o, pframe_t *pf);
static int  anon_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t anon_mmobj_ops = {
        .ref = anon_ref,
        .put = anon_put,
        .lookuppage = anon_lookuppage,
        .fillpage  = anon_fillpage,
        .dirtypage = anon_dirtypage,
        .cleanpage = anon_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * anonymous page sub system. Currently it only initializes the
 * anon_allocator object.
 */
void
anon_init()
{
        //NOT_YET_IMPLEMENTED("VM: anon_init");
        
        anon_allocator = slab_allocator_create("anon_slab", sizeof(mmobj_t));   // given allocator
        dbg(DBG_PRINT, "(GRADING3A 4.a)\n");
        KASSERT(anon_allocator);
        dbg(DBG_PRINT, "(GRADING3A 4)\n");

}

/*
 * You'll want to use the anon_allocator to allocate the mmobj to
 * return, then initialize it. Take a look in mm/mmobj.h for
 * definitions which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
anon_create()
{
        //NOT_YET_IMPLEMENTED("VM: anon_create");
        //return NULL;

        mmobj_t *anon_obj = (mmobj_t*)slab_obj_alloc(anon_allocator);
        mmobj_init(anon_obj,&anon_mmobj_ops);  // given anon operations
        anon_obj->mmo_refcount=1;
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return anon_obj;

}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
anon_ref(mmobj_t *o)
{
        //NOT_YET_IMPLEMENTED("VM: anon_ref");
        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 4.b)\n");
        o->mmo_refcount++;
        dbg(DBG_PRINT, "(GRADING3A 4)\n");

}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is an anonymous object, it will
 * never be used again. You should unpin and uncache all of the
 * object's pages and then free the object itself.
 */
static void
anon_put(mmobj_t *o)
{
        //NOT_YET_IMPLEMENTED("VM: anon_put");

        KASSERT(o && (0 < o->mmo_refcount) && (&anon_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 4.c)\n");

        if(o->mmo_refcount != o->mmo_nrespages+1){
                o->mmo_refcount--;
                dbg(DBG_PRINT, "(GRADING3A 4)\n");
        }else{
                pframe_t *pframe_iter;
                dbg(DBG_PRINT, "(GRADING3A 4)\n");

                list_iterate_begin(&o->mmo_respages,pframe_iter,pframe_t,pf_olink) {
                        if(!pframe_is_dirty(pframe_iter)){
                                pframe_free(pframe_iter);
                                dbg(DBG_PRINT, "(GRADING3A 4)\n");
                        }else{
                                pframe_clean(pframe_iter);
                                pframe_free(pframe_iter);
                                dbg(DBG_PRINT, "(GRADING3A 4)\n");
                        }
                        dbg(DBG_PRINT, "(GRADING3A 4)\n");
                }
                list_iterate_end();

                slab_obj_free(anon_allocator,o);
                o->mmo_refcount--;
                dbg(DBG_PRINT, "(GRADING3A 4)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A 4)\n");

}

/* Get the corresponding page from the mmobj. No special handling is
 * required. */
static int
anon_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        //NOT_YET_IMPLEMENTED("VM: anon_lookuppage");
        //return -1;
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return pframe_get(o,pagenum,pf);

}

/* The following three functions should not be difficult. */

static int
anon_fillpage(mmobj_t *o, pframe_t *pf)
{
        //NOT_YET_IMPLEMENTED("VM: anon_fillpage");
        //return 0;

        KASSERT(pframe_is_busy(pf));
        dbg(DBG_PRINT, "(GRADING3A 4.d)\n");
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_PRINT, "(GRADING3A 4.d)\n");


        pframe_pin(pf);
        memset(pf->pf_addr,0,PAGE_SIZE);
        pframe_unpin(pf);
        dbg(DBG_PRINT, "(GRADING3A 4)\n");
        return 0;

}

static int
anon_dirtypage(mmobj_t *o, pframe_t *pf)
{
        //NOT_YET_IMPLEMENTED("VM: anon_dirtypage");
        //return -1;

        pframe_set_dirty(pf);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 1;

}

static int
anon_cleanpage(mmobj_t *o, pframe_t *pf)
{
        //NOT_YET_IMPLEMENTED("VM: anon_cleanpage");
        //return -1;

        pframe_clear_dirty(pf);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 1;

}