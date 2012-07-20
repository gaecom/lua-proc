#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "dlist.h"
#include "info.h"

int
procI_info_create(procI_info  **info, pid_t pid, char *name)
{
    int ret = 0;

    if ((*info = calloc(1, sizeof(procI_info))) == NULL)
        goto fail;
    (*info)->name = strdup(name);
    if ((*info)->name == NULL)
        goto fail;
    (*info)->pid = pid;
    DLIST_INIT(&((*info)->list));
    goto succeed;

fail:
    procI_info_destroy(*info);
    *info = NULL;
succeed:
    return ret;
}

void
procI_info_destroy(procI_info  *info)
{
    if (info != NULL) {
        free(info->name);
        DLIST_INIT(&info->list);
        free(info);
    }
}
