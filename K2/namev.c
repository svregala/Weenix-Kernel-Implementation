/******************************************************************************/
/* Important Spring 2022 CSCI 402 usage information:                          */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * Most of the work should be done by the vnode's implementation
 * specific lookup() function.
 *
 * If dir has no lookup(), return -ENOTDIR.
 *
 * Note: returns with the vnode refcount on *result incremented.
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{

        KASSERT(NULL != dir);
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
        KASSERT(NULL != name);
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
        KASSERT(NULL != result);
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
        if (dir->vn_ops->lookup == NULL){
            dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
            return -ENOTDIR;
        }
        dbg(DBG_PRINT, "(GRADING2A 2.a)\n");
        return dir->vn_ops->lookup(dir, name, len, result);
}


/* When successful this function returns data in the following "out"-arguments:
 *  o res_vnode: the vnode of the parent directory of "name"
 *  o name: the `basename' (the element of the pathname)
 *  o namelen: the length of the basename
 *
 * For example: dir_namev("/s5fs/bin/ls", &namelen, &name, NULL,
 * &res_vnode) would put 2 in namelen, "ls" in name, and a pointer to the
 * vnode corresponding to "/s5fs/bin" in res_vnode.
 *
 * The "base" argument defines where we start resolving the path from:
 * A base value of NULL means to use the process's current working directory,
 * curproc->p_cwd.  If pathname[0] == '/', ignore base and start with
 * vfs_root_vn.  dir_namev() should call lookup() to take care of resolving each
 * piece of the pathname.
 *
 * Note: A successful call to this causes vnode refcount on *res_vnode to
 * be incremented.
 */
int
dir_namev(const char *pathname, size_t *namelen, const char **name,
          vnode_t *base, vnode_t **res_vnode)
{
        //NOT_YET_IMPLEMENTED("VFS: dir_namev");
        //return 0;

        KASSERT(NULL != pathname);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        KASSERT(NULL != namelen);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        KASSERT(NULL != name);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");
        KASSERT(NULL != res_vnode);
        dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

        // handle the case for when pathname is an empty string
        if (*pathname == '\0') {
            return -EINVAL;
        }

        vnode_t *prev_vnode = NULL;
        vnode_t *base_vnode = NULL;
        int i = 0;
        int offset = 0;
        int nameLength = 0;
        int returncode = 0;

        if(pathname[0] == '/'){
            base_vnode = vfs_root_vn;
        }else if(base != NULL){
            base_vnode = base;
        }else{
            base_vnode = curproc->p_cwd;
        }

        vref(base_vnode);

        while(pathname[i] == '/'){    
            i++;
        }



        do{
            nameLength = 0;
            offset = i;

            if( pathname[i]=='\0' && prev_vnode==NULL){
                *namelen = 1;
                *name = ".";
                *res_vnode = base_vnode;
                return 0;
            }

            if(prev_vnode!=NULL){
                vput(prev_vnode);
            }

            prev_vnode = base_vnode;

            while(pathname[i] != '/' && pathname[i] != '\0'){
                i++;
                nameLength++;
            }

            if(nameLength > NAME_LEN){
                vput(prev_vnode);
                return -ENAMETOOLONG;
            }


            returncode = lookup(prev_vnode,pathname+offset,nameLength,&base_vnode);
            KASSERT(NULL != base_vnode);
            dbg(DBG_PRINT, "(GRADING2A 2.b)\n");

            dbg(DBG_PRINT, "return code in dir_namev is %d\n", returncode);


            if((returncode == -ENOTDIR) || (returncode<0 && returncode!=-ENOENT)){
                vput(prev_vnode);
                return returncode;
            }

            while(pathname[i] == '/'){
                i++;
            }


        }while(pathname[i] != '\0' && returncode >= 0);


        if (pathname[i-1]== '/') {
            if (!S_ISDIR((base_vnode)->vn_mode))
            {
                vput(prev_vnode);
                vput(base_vnode);
                dbg(DBG_ERROR,"\n vnode is not a directory\n");
                return -ENOTDIR;
            }
        }


        if(pathname[i]!='\0'){
            vput(prev_vnode);
            return -ENOENT;
        }else if(returncode == 0){
            vput(base_vnode);
        }
        
        *name = (pathname + offset);
        *namelen = nameLength;
        *res_vnode = prev_vnode;

        return 0;
}

/* This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode (if it
 * exists).  flag is right out of the parameters to open(2); see
 * <weenix/fcntl.h>.  If the O_CREAT flag is specified and the file does
 * not exist, call create() in the parent directory vnode. However, if the
 * parent directory itself does not exist, this function should fail - in all
 * cases, no files or directories other than the one at the very end of the path
 * should be created.
 *
 * Note: Increments vnode refcount on *res_vnode.
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
        size_t namelen = 0;
        const char *name = NULL;
        vnode_t *res_dir_vnode = NULL;
        int dirReturnCode = dir_namev(pathname, &namelen, &name, base, &res_dir_vnode);

        if (dirReturnCode < 0){
            dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
            return dirReturnCode;
        }

        int lookupReturnCode = lookup(res_dir_vnode, name, namelen, res_vnode);
        
        if(lookupReturnCode < 0){
            if ((flag & O_CREAT)) {
                KASSERT(NULL != res_dir_vnode->vn_ops->create);
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                dbg(DBG_PRINT, "(GRADING2B)\n");
            }
            if((flag & O_CREAT) && lookupReturnCode == -ENOENT){
                int createReturnCode = res_dir_vnode->vn_ops->create(res_dir_vnode, name, namelen, res_vnode);
                vput(res_dir_vnode);
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                return createReturnCode;
            }
            else{
                vput(res_dir_vnode);
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                return lookupReturnCode;
            }
        }
        else{
            if(((flag & O_WRONLY) || (flag & O_RDWR)) && (*res_vnode)->vn_ops->mkdir != NULL){
                vput(*res_vnode);
                *res_vnode = NULL;
                vput(res_dir_vnode);
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                return -EISDIR;
            }
            else{
                vput(res_dir_vnode);
                
                dbg(DBG_PRINT, "(GRADING2A 2.c)\n");
                return 0;
            }
        }
}

#ifdef __GETCWD__
/* Finds the name of 'entry' in the directory 'dir'. The name is writen
 * to the given buffer. On success 0 is returned. If 'dir' does not
 * contain 'entry' then -ENOENT is returned. If the given buffer cannot
 * hold the result then it is filled with as many characters as possible
 * and a null terminator, -ERANGE is returned.
 *
 * Files can be uniquely identified within a file system by their
 * inode numbers. */
int
lookup_name(vnode_t *dir, vnode_t *entry, char *buf, size_t size)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_name");
        return -ENOENT;
}


/* Used to find the absolute path of the directory 'dir'. Since
 * directories cannot have more than one link there is always
 * a unique solution. The path is writen to the given buffer.
 * On success 0 is returned. On error this function returns a
 * negative error code. See the man page for getcwd(3) for
 * possible errors. Even if an error code is returned the buffer
 * will be filled with a valid string which has some partial
 * information about the wanted path. */
ssize_t
lookup_dirpath(vnode_t *dir, char *buf, size_t osize)
{
        NOT_YET_IMPLEMENTED("GETCWD: lookup_dirpath");

        return -ENOENT;
}
#endif /* __GETCWD__ */
