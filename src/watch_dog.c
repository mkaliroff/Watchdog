
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <time.h>               /* time                                      */
#include <sys/types.h>          /* pid_t                                     */
#include <assert.h>             /* assert, NDEBUG                            */
#include <stddef.h>             /* size_t, NULL                              */
#include <stdlib.h>             /* malloc, free                              */
#include <unistd.h>             /* fork, execvp                              */
#include <sys/wait.h>           /* wait, WEXITSTATUS, WTERMSIG               */
#include <pthread.h>            /* create, join                              */
#include <signal.h>             /* SIGUSR1, SIGUSR2, sigaction, kill, pause  */
#include <stdio.h>              /* sprintf , printf for testing              */
#include <string.h>             /* strcat                                    */
#include <libgen.h>             /* dirname                                   */
#include <limits.h>             /* PATH_MAX                                  */

#include "watch_dog.h"           /* Internal API */
#include "wd_heap_scheduler.h"   /* Internal API */
#include "internal_watch_dog.h"  /* Internal API */
/*****************************************************************************/

#define WD_TRUE (1)
#define WD_FALSE (0)

#define TOLERANCE (4)
#define TIME_T_MAX_VALUE (20)

#define IGNORE(X) ((void) X)

#define INVALID_PID (-1)

/*****************************************************************************/

wd_obj_t *wd_obj = NULL;

/*****************************************************************************/
static wd_status_t SetupSignalHandlerStop(void);
static void WDSignalHandlerAlive(int);
static void WDSignalHandlerStop(int sig);
static wd_status_t InitWDProc(char **);
static wd_status_t WDRunScheduler(void);
static void *WDRun(void *);
int SendSignal(void *);

static int StopMainScheduler(void *);
int Revival(void *);
wd_status_t InitMainProc(char **argv);
static int StopWDScheduler(void *object);

/*****************************************************************************/
wd_status_t WDWatch(time_t interval, char *argv[])
{
    char env_buffer[TIME_T_MAX_VALUE] = {0};
    wd_status_t status = WD_SUCCESS;

    if(2 > interval)
    {
        return (WD_INTERNAL_FAILURE);
    }

    wd_obj = (wd_obj_t *)malloc(sizeof(wd_obj_t));
    if(NULL == wd_obj)
    {
        return (WD_INTERNAL_FAILURE);
    }

    sprintf(env_buffer, "%ld", (long int)interval);
    /* convert interval into env_buffer */

    if(setenv("WD_USER_INTERVAL", env_buffer, 1))
    {
        return (WD_INTERNAL_FAILURE);
    }

    wd_obj->is_main_proc = WD_TRUE;

    status = SetUpWDObject(interval, argv);
    if(WD_SUCCESS != status)
    {
        return (WD_FAILURE);
    }

    status = SetupSignalHandler();
    if(WD_SUCCESS != status)
    {
        return (WD_FAILURE);
    }
    status = InitWDProc(argv);
    if(WD_INTERNAL_FAILURE == status)
    {
        return (WD_INTERNAL_FAILURE);
    }
    status = SetUpWDScheduler();
    if(WD_SUCCESS != status)
    {
        return (WD_INTERNAL_FAILURE);
    }

    status = WDRunScheduler();
    if(WD_SUCCESS != status)
    {
        return (WD_THREAD_FAILURE);
    }

    return (WD_SUCCESS);
}
/*****************************************************************************/
wd_status_t WDUnWatch(void)
{
    wd_obj->wd_time_to_clean_up = WD_TRUE;
    pthread_join(wd_obj->thread, NULL);
    return (WD_SUCCESS);
}
/*****************************************************************************/ 


/*****************************************************************************/ 
wd_status_t SetUpWDObject(time_t interval, char **argv)
{
    wd_obj->argv = argv;
    wd_obj->interval = interval;
    wd_obj->wd_time_to_clean_up = WD_FALSE;
    wd_obj->wd_is_alive = WD_FALSE;

    if(WD_TRUE == wd_obj->is_main_proc)
    {
        wd_obj->signal_pid = INVALID_PID;
    }
    else
    {
        wd_obj->signal_pid = getppid();

    }

    wd_obj->scheduler = HeapSchedulerCreate();
    if(NULL == wd_obj->scheduler)
    {
        return (WD_FAILURE);
    }

    return (WD_SUCCESS);
}
/* Setup Object Struct */
/*****************************************************************************/
wd_status_t SetupSignalHandler(void)
{
    struct sigaction handler = {0};
    handler.sa_flags = 0;
    handler.sa_handler = WDSignalHandlerAlive;
    if(sigaction(SIGUSR1, &handler, NULL))
    {
        return (WD_INTERNAL_FAILURE);
    }

    if(WD_TRUE == wd_obj->is_main_proc)
    {
        return SetupSignalHandlerStop();
    }

    return (WD_SUCCESS);
}
/* setup signal handler */
/*****************************************************************************/
static wd_status_t SetupSignalHandlerStop(void)
{
    struct sigaction handler2 = {0};
    handler2.sa_flags = 0;
    sigemptyset(&handler2.sa_mask);

    handler2.sa_handler = WDSignalHandlerStop;
    if(sigaction(SIGUSR2, &handler2, NULL))
    {
        return (WD_INTERNAL_FAILURE);
    }

    return (WD_SUCCESS);
}
/* setup signals handlers, sigusr1 sigusr2 */
/*****************************************************************************/
static void WDSignalHandlerAlive(int sig)
{
    wd_obj->wd_is_alive = WD_TRUE;
    #ifndef NDEBUG
    printf("Proc(%d) ALIVE signal recived\n",getpid());
    #endif
    
    IGNORE(sig);
}
/* raise wd_proccess_status */
/*****************************************************************************/
static void WDSignalHandlerStop(int sig)
{
    wd_obj->wd_time_to_clean_up = WD_TRUE;
    #ifndef NDEBUG  
    printf("Proc(%d) STOP signal recived\n",getpid());
    #endif

    IGNORE(sig);
}
/* for sigusr2 raise wd_time_to_clean_up flag */
/*****************************************************************************/
static wd_status_t InitWDProc(char **argv)
{
    int status = 0;
    char main_proc_path[PATH_MAX] = {0};
    char watchdog_path[PATH_MAX] = {0};

    strcat(main_proc_path,argv[0]);
    strcat(watchdog_path, dirname(dirname(main_proc_path)));
    strcat(watchdog_path,"/dist/watchdog.out");

    wd_obj->signal_pid = fork();
    if(-1 == wd_obj->signal_pid)
    {
        return (WD_INTERNAL_FAILURE);
    }
    else if(0 == wd_obj->signal_pid)
    {
        execvp(watchdog_path, argv);
        return (WD_INTERNAL_FAILURE);
    }
    else
    {
        while(-1  == waitpid(wd_obj->signal_pid, &status, WUNTRACED));
        if(!WIFSTOPPED(status))
        {
            return (WD_INTERNAL_FAILURE);
        }
    }

    return (WD_SUCCESS);
}
/* init watch_dog.out process */
/*****************************************************************************/
wd_status_t InitMainProc(char **argv)
{
    wd_obj->signal_pid = fork();
    if(-1 == wd_obj->signal_pid)
    {
        return (WD_INTERNAL_FAILURE);
    }
    else if(0 == wd_obj->signal_pid)
    {
        execvp(wd_obj->argv[0], argv);
        return (WD_INTERNAL_FAILURE);
    }

    return (WD_SUCCESS);
}
/* init watch_dog.out process */
/*****************************************************************************/
wd_status_t SetUpWDScheduler(void)
{
    size_t i = 0;
    ilrd_uid_t uid = {0};

    if(wd_obj->interval / TOLERANCE >= 1)
    {
        uid = HeapSchedulerAddTask(wd_obj->scheduler, SendSignal, \
            NULL, TaskCleanUp ,  wd_obj->interval / TOLERANCE, time(0));
        if(UIDIsMatch(UIDGetBad(), uid))
        {
            return (WD_INTERNAL_FAILURE);
        }
    }
    else
    {
        for(i = 0 ; i < TOLERANCE ; i++)
        {
            uid = HeapSchedulerAddTask(wd_obj->scheduler, SendSignal, \
                NULL, TaskCleanUp ,  wd_obj->interval / TOLERANCE, (time_t)0);
            if(UIDIsMatch(UIDGetBad(), uid))
            {
                return (WD_INTERNAL_FAILURE);
            }
        }
    }

    if(WD_TRUE == wd_obj->is_main_proc)
    {
        uid = HeapSchedulerAddTask(wd_obj->scheduler, StopMainScheduler, NULL,\
            TaskCleanUp , 1, (time_t)1);
    }
    else
    {
     if(WD_TRUE == wd_obj->is_main_proc)
    {
        uid = HeapSchedulerAddTask(wd_obj->scheduler, StopWDScheduler, NULL,\
            TaskCleanUp , 1, (time_t)1);
    }       
    }
    if(UIDIsMatch(UIDGetBad(), uid))
    {
        return (WD_INTERNAL_FAILURE);
    }

    uid = HeapSchedulerAddTask(wd_obj->scheduler, Revival, NULL, \
        TaskCleanUp , (size_t)wd_obj->interval, time(NULL) + wd_obj->interval);
    if(UIDIsMatch(UIDGetBad(), uid))
    {
        return (WD_INTERNAL_FAILURE);
    }

    return (WD_SUCCESS);
}
/* Add Tasks to scheduler */
/*****************************************************************************/
static wd_status_t WDRunScheduler(void)
{
    if(pthread_create(&wd_obj->thread, NULL, WDRun, NULL))
    {
        HeapSchedulerDestroy(wd_obj->scheduler);
        free(wd_obj);
        return (WD_THREAD_FAILURE);
    }

    return (WD_SUCCESS);
}
/* create thread and start scheduler */
/*****************************************************************************/
static void *WDRun(void *arg)
{
    #ifndef NDEBUG
    printf("\nThread is activated and running the scheduler!\n");
    #endif
    kill(wd_obj->signal_pid, SIGCONT); /* try to use semaphores */
    HeapSchedulerRun(wd_obj->scheduler);
    HeapSchedulerDestroy(wd_obj->scheduler);
    free(wd_obj);
    IGNORE(arg);
    return (NULL);
}
/*****************************************************************************/
int SendSignal(void *arg)
{
    #ifndef NDEBUG
    printf("Main Proc(%d) SIGUSR1 signal sent to Watchdog\n",getpid());
    #endif
    kill(wd_obj->signal_pid, SIGUSR1);


    IGNORE(arg);
    return (WD_SUCCESS);
}
/* send sigusr1 to other proc */
/*****************************************************************************/
static int StopMainScheduler(void *object)
{
    if(WD_TRUE == wd_obj->wd_time_to_clean_up)
    {
        #ifndef NDEBUG
        printf("PROC %d - Stop task activated\n", getpid());
        #endif
        wd_obj->wd_time_to_clean_up = WD_FALSE;  
        kill(wd_obj->signal_pid, SIGUSR2);
        HeapSchedulerStop(wd_obj->scheduler);
        return (WD_FAILURE);
    }

    IGNORE(object);
    return (WD_SUCCESS);
}
/* send sigusr2 to other proc */
/*****************************************************************************/
static int StopWDScheduler(void *object)
{
    if(WD_TRUE == wd_obj->wd_time_to_clean_up)
    {
        #ifndef NDEBUG  
        printf("PROC %d - Stop task activated\n", getpid());
        #endif
        HeapSchedulerStop(wd_obj->scheduler);
        HeapSchedulerDestroy(wd_obj->scheduler);
        free(wd_obj);
    }
    IGNORE(object);
    return (WD_SUCCESS);
}
/*****************************************************************************/
int Revival(void *object)
{
    if(WD_FALSE == wd_obj->wd_time_to_clean_up && \
                            WD_FALSE == wd_obj->wd_is_alive)
    {
        #ifndef NDEBUG  
        printf("\nWD Revival task activated\n");
        #endif
        if(WD_TRUE == wd_obj->is_main_proc)
        {
            InitWDProc(wd_obj->argv);
        }
        else
        {
            InitMainProc(wd_obj->argv);
        }
        wd_obj->wd_time_to_clean_up = WD_TRUE;
        return WD_FAILURE;
    }

    wd_obj->wd_is_alive = WD_FALSE;    

    IGNORE(object);
    return (WD_SUCCESS);
}
/* check if wd_is_alive is raised, otherwise revive other proc */
/*****************************************************************************/
void TaskCleanUp(void *object)
{
    IGNORE(object);
}
/* empty func */
/*****************************************************************************/

