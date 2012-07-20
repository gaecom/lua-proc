#ifndef PROC_INFO_H
#define PROC_INFO_H

#include <sys/types.h>

#include "dlist.h"

typedef struct procI_info
{
    pid_t   pid;
    char    *name;
    dlist   list;
} procI_info;

void
procI_info_destroy(procI_info  *info);

int
procI_info_create(procI_info  **info, pid_t pid, char *name);


#endif /* ! PROC_INFO_H */
