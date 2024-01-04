#define _POSIX_SOURCE
#include <stddef.h>             /* size_t, NULL                      */
#include <stdlib.h>             /* malloc, free                       */
#include <signal.h>             /* kill */
#include <pthread.h>            /*  pthread_t */
#include <unistd.h>             /* getpid                              */

#include "watch_dog.h"           /* Internal API */
#include "wd_heap_scheduler.h"   /* Internal API */
#include "internal_watch_dog.h"  /* Internal API */
/*****************************************************************************/
#define PATH_MAX_SIZE (255)

#define WD_TRUE (1)
#define WD_FALSE (0)
/*****************************************************************************/

wd_obj_t *wd_obj = NULL;

/*****************************************************************************/
int main(int argc, char *argv[])
{
    time_t interval = (time_t)0;
    wd_status_t status = WD_SUCCESS;
    char *user_interval = getenv("WD_USER_INTERVAL");
    if(NULL == user_interval)
    {
        return (WD_INTERNAL_FAILURE);
    }

    wd_obj = (wd_obj_t *)malloc(sizeof(wd_obj_t));
    if(NULL == wd_obj)
    {

        return (WD_INTERNAL_FAILURE);
    }

    interval = (time_t)atol(user_interval);

    wd_obj->is_main_proc = WD_FALSE;

    status = SetUpWDObject(interval, argv);
    if(WD_SUCCESS != status)
    {

        free(wd_obj);
        return (WD_INTERNAL_FAILURE);
    }

    status = SetupSignalHandler();
    if(WD_SUCCESS != status)
    {
        HeapSchedulerDestroy(wd_obj->scheduler);
        free(wd_obj);
        return (WD_INTERNAL_FAILURE);
    }

    status = SetUpWDScheduler();
    if(WD_SUCCESS != status)
    {
        HeapSchedulerDestroy(wd_obj->scheduler);
        free(wd_obj);
        return (WD_INTERNAL_FAILURE);
    }

    kill(getpid(), SIGSTOP);  /* check this out */

    HeapSchedulerRun(wd_obj->scheduler);
    return (WD_SUCCESS);
}
/*****************************************************************************/
 
