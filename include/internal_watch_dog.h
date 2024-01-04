#ifndef __INTERNAL_WATCH_DOG_H__
#define __INTERNAL_WATCH_DOG_H__

typedef struct wd_obj
{
    char **argv;
    time_t interval;
    pthread_t thread;
    atomic_int wd_is_alive;
    atomic_int wd_time_to_clean_up;
    heap_scheduler_t *scheduler;
    pid_t signal_pid;
    int is_main_proc;
} wd_obj_t;


wd_status_t SetupSignalHandler(void);
int SendSignal(void *arg);
void TaskCleanUp(void *);
int Revival(void *object);
wd_status_t SetUpWDScheduler(void);
wd_status_t InitMainProc(char **argv);
wd_status_t SetUpWDObject(time_t, char **);


#endif /* __INTERNAL_WATCH_DOG_H__ */
