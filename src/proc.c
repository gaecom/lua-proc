#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(TARGET_DARWIN)
#   include <sys/sysctl.h>
#else
#   include <dirent.h>
#   include <fcntl.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "dlist.h"
#include "info.h"

#define PROC_LIBNAME   "proc"
#define PROC_VERSION   "1.0"
#define PROC_ERROR_CANT_GET_PROCLIST "can't get process list"

static void
chomp(char *s)
{
    int n = strlen(s);
    if ((n > 0) && (s[n-1]) == '\n') {
        s[n-1] = '\0';
    }
}

static int
pusherror(lua_State *L, const char *info)
{   
    lua_pushnil(L);
    if (info == NULL) 
        lua_pushstring(L, strerror(errno));
    else 
        lua_pushfstring(L, "%s: %s", info, strerror(errno));
    lua_pushinteger(L, errno);
    return 3;
}

static int
pushresult(lua_State *L, int i, const char *info)
{
    if (i==-1) return pusherror(L, info);
    lua_pushinteger(L, i);
    return 1;
}

#if defined(TARGET_DARWIN)
procI_getproclist(dlist *info)
{    

    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    int i;
    size_t buffer_size;
    size_t num_procs;
    struct kinfo_proc *pproc;
    struct kinfo_proc *proc_list;
    
    LOG_TRACE_ENTER();
    ASSERT(dlist != NULL);
    sysctl(mib, 4, NULL, &buffer_size, NULL, 0);
    proc_list = malloc(buffer_size);
    sysctl(mib, 4, proc_list, &buffer_size, NULL, 0);
    
    num_procs = buffer_size / sizeof(struct kinfo_proc);
    for (i = 0; i < num_procs; i++)
        pproc = proc_list + i;
        ret = procI_info_create(&info, pproc->kp_proc.p_pid,
                pproc->kp_proc.p_comm);
        FAIL_IF(ret != 0); 
        dlist_add_tail(&proc->list, info);
    }

fail:
    FREE(proc_list);
    dlist_foreach_safe();
    LOG_TRACE_EXIT();
    return ret;
}
#else
static int
procI_get_proclist(dlist *head)
{
    int             ret = 0;
    DIR             *procfs = NULL;
    struct dirent   *dp = NULL;
    struct stat     statbuf;
    pid_t           pid = 0;
    int             fd = 0;
    char            path[256] = {0};
    char            name[256] = {0};
    procI_info      *info = NULL;
    dlist           *pos = NULL;
    dlist           *n = NULL;
    
    procfs = opendir("/proc");    
    while (dp = readdir(procfs)) {
        sprintf(path, "/proc/%s", dp->d_name);                
        if (stat(path, &statbuf) == 0) {
            if (statbuf.st_mode & S_IFDIR) {
                errno = 0;
                pid = strtoul(dp->d_name, NULL, 0);
                if (errno == EINVAL)
                    continue;
                sprintf(path, "/proc/%d/comm", pid);                
                fd = open(path, O_RDONLY);
                if (fd == -1)
                    continue;
                if (read(fd, name, sizeof(name)) != -1) {
                    chomp(name);
                    ret = procI_info_create(&info, pid, name);
                    if (ret == -1)
                        goto fail;
                    dlist_add_tail(&(info->list), head);
                }
                close(fd);
                fd = -1;
            } 
        }
        memset(name, 0x00, sizeof(name));
        memset(path, 0x00, sizeof(path));
    }
    goto succeed;

fail:
    if (fd == -1)
        (void) close(fd);
    if (procfs != NULL)
        closedir(procfs);

    dlist_foreach_safe(pos, n, head) {
        info = dlist_get_entry(pos, procI_info, list);
        procI_info_destroy(info);
    }

    ret = -1;

succeed:
    return ret;
}
#endif

static int
proc_kill(lua_State *L)
{
    pid_t   pid = luaL_checkint(L,1); 
    int     sig = luaL_optint(L, 2, SIGTERM);

    return pushresult(L, kill(pid, sig), NULL);
}

static int
proc_killall(lua_State *L)
{
    int         ret = 0;
    int         failed = 0;
    procI_info  *info = NULL;
    const char  *name = luaL_checkstring(L, 1);
    int         sig = luaL_optint(L, 2, SIGTERM);
    dlist       *pos = NULL;
    dlist       *n = NULL;
    DLIST_DECLARE(head);

    ret = procI_get_proclist(&head);
    if (ret != 0)
        return pusherror(L, PROC_ERROR_CANT_GET_PROCLIST);

    dlist_foreach_safe(pos, n, &head) {
        info = dlist_get_entry(pos, procI_info, list);
        if (strcmp(info->name, name) == 0)
            if (kill(info->pid, sig) != 0)
                failed = 1;
        procI_info_destroy(info);
    }

    if (failed)
        return pusherror(L, "proclist");
    lua_pushinteger(L, 0);
    return 1;
}

static int
proc_list(lua_State *L) /* table = proc.list() */
{
    int         ret = 0;
    int         i = 0;
    procI_info  *info = NULL;
    dlist       *pos = NULL;
    dlist       *n = NULL;
    DLIST_DECLARE(head);

    ret = procI_get_proclist(&head);
    if (ret != 0)
        return pusherror(L, PROC_ERROR_CANT_GET_PROCLIST);

    lua_newtable(L);
    dlist_foreach_safe(pos, n, &head) {
        info = dlist_get_entry(pos, procI_info, list);
        lua_pushinteger(L, i+1);
        lua_newtable(L);
        lua_pushliteral(L, "name");
        lua_pushstring(L, info->name);
        lua_settable(L, -3);
        lua_pushliteral(L, "pid");
        lua_pushinteger(L, info->pid);
        lua_settable(L, -3);
        lua_settable(L, -3);
        procI_info_destroy(info);
        i++;
    }
    return 1;
}

static int
proc_pidof(lua_State *L)   /* table = proc.pidof(name) */
{
    int         ret = 0;
    int         count = 0;
    procI_info  *info = NULL;
    dlist       *pos = NULL;
    dlist       *n = NULL;
    const char  *name = luaL_checkstring(L, 1);
    DLIST_DECLARE(head);

    ret = procI_get_proclist(&head);
    if (ret != 0)
        return pusherror(L, PROC_ERROR_CANT_GET_PROCLIST);

    lua_newtable(L);
    dlist_foreach_safe(pos, n, &head) {
        info = dlist_get_entry(pos, procI_info, list);
        if (strcmp(info->name, name) == 0)
        {
            count++;
            lua_pushinteger(L, count);
            lua_pushinteger(L, info->pid);
            lua_settable(L, -3);
        }
        procI_info_destroy(info);
    }
    return 1;
}

static const luaL_Reg R[] =
{
    {"list", proc_list},
    {"pidof", proc_pidof},
    {"killall", proc_killall},
    {"kill", proc_kill},
    {NULL, NULL}
};

LUALIB_API int
luaopen_proc(lua_State *L)
{
    luaL_register(L, PROC_LIBNAME, R);
    lua_pushliteral(L, PROC_VERSION);
    lua_setfield(L, -2, "version");
    return 1;
}
