
#include <stdio.h>        /* printf, puts */
#include <assert.h>       /* assert       */
#include <time.h>               /* time                           */
#include <sys/types.h>          /* pid_t                           */
#include <assert.h>             /* assert                           */
#include <stddef.h>             /* size_t, NULL                      */
#include <stdlib.h>             /* malloc, free                       */
#include <unistd.h>             /* fork, execvp, sleep                  */
#include <sys/wait.h>           /* wait, WEXITSTATUS, WTERMSIG           */
#include <pthread.h>            /* create, join, mutex_init, mutex_destroy */
#include <signal.h>             /* SIGUSR1, SIGUSR2, sigaction, kill, pause */

#include "watch_dog.h"    /* Internal API */

#define IGNORE(X) ((void) X)

/*****************************************************************************/
void WDTest(char *args[]);
#define SUCCESS (0)
/*****************************************************************************/
int main(int argc, char *argv[])
{
    IGNORE(argc);
    puts("watchdog_test.out program");
    WDTest(argv);
    puts("\nWDTest() : done.");
    return (SUCCESS);
}
/*****************************************************************************/
void WDTest(char *args[])
{
    time_t interval = 5;
    time_t duration = 20;
    time_t start_time = time(NULL);
    assert(WD_SUCCESS ==  WDWatch(interval, args));
    while (time(NULL) - start_time < duration);

    assert(WD_SUCCESS == WDUnWatch());
}
/*****************************************************************************/
