#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/* find empty index in p->p_files[] */
int
get_empty_fd(proc_t *p)
{
        int fd;

        for (fd = 0; fd < NFILES; fd++) {
                if (!p->p_files[fd])
                        return fd;
        }

        dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
            "for pid %d\n", curproc->p_pid);
        return -EMFILE;
}

/*
 * There a number of steps to opening a file:
 *      1. Get the next empty file descriptor.
 *      2. Call fget to get a fresh file_t.
 *      3. Save the file_t in curproc's file descriptor table.
 *      4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 *         oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
 *         O_APPEND.
 *      5. Use open_namev() to get the vnode for the file_t.
 *      6. Fill in the fields of the file_t.
 *      7. Return new fd.
 *
 * If anything goes wrong at any point (specifically if the call to open_namev
 * fails), be sure to remove the fd from curproc, fput the file_t and return an
 * error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        oflags is not valid.
 *      o EMFILE
 *        The process already has the maximum number of files open.
 *      o ENOMEM
 *        Insufficient kernel memory was available.
 *      o ENAMETOOLONG
 *        A component of filename was too long.
 *      o ENOENT
 *        O_CREAT is not set and the named file does not exist.  Or, a
 *        directory component in pathname does not exist.
 *      o EISDIR
 *        pathname refers to a directory and the access requested involved
 *        writing (that is, O_WRONLY or O_RDWR is set).
 *      o ENXIO
 *        pathname refers to a device special file and no corresponding device
 *        exists.
 */

int
do_open(const char *filename, int oflags)
{
        //NOT_YET_IMPLEMENTED("VFS: do_open");
        //return -1;
        int readAndWrite = (oflags & O_RDWR) && !(oflags & O_WRONLY) && !(oflags & O_RDONLY);
        int writeOnly = (oflags & O_WRONLY) && !(oflags & O_RDWR);
        int readOnly = ((oflags == O_RDONLY) || (oflags == (O_RDONLY | O_CREAT))) && !(oflags & O_RDWR);
        int append = (oflags & O_APPEND);
        dbg(DBG_PRINT, "(GRADING2B)\n");

        if((readOnly && writeOnly) || (readAndWrite && (readOnly || writeOnly))){
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return -EINVAL;
        }

        int fd = get_empty_fd(curproc);

        if (fd == -EMFILE){
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return -EMFILE;
        }

        file_t *file = fget(-1);

        if(file != NULL){
            curproc->p_files[fd] = file;
            file->f_mode = 0;
            dbg(DBG_PRINT, "(GRADING2B)\n");

            if (append){
                file->f_mode = FMODE_APPEND;
                dbg(DBG_PRINT, "(GRADING2B)\n");
            }

            if(writeOnly){
                file->f_mode = file->f_mode | FMODE_WRITE;
                dbg(DBG_PRINT, "(GRADING2B)\n");
            } 
            
            else if(readAndWrite){
                file->f_mode = file->f_mode | FMODE_READ | FMODE_WRITE;
                dbg(DBG_PRINT, "(GRADING2B)\n");
            }

            else if(readOnly){
                file->f_mode = file->f_mode | FMODE_READ;
                dbg(DBG_PRINT, "(GRADING2B)\n");
            }

            else {
                fput(file);
                curproc->p_files[fd] = NULL;
                return -EINVAL;
            }

            vnode_t *res_vnode = NULL;
            int open_result = open_namev(filename, oflags, &res_vnode, NULL);

            if (open_result < 0){
                curproc->p_files[fd] = NULL;
                fput(file);
                dbg(DBG_PRINT, "(GRADING2B)\n");
                return open_result;
            }

            file->f_pos = 0;
            file->f_refcount = 1;
            file->f_vnode = res_vnode;
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return fd;
        }else{
            dbg(DBG_PRINT, "(GRADING2B)\n");
            return -ENOMEM;
        }
}
