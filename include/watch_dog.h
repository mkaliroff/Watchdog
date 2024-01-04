
#ifndef __WATCH_DOG_H__
#define __WATCH_DOG_H__

#include <time.h> /* time_t */

typedef enum wd_status
{
    WD_SUCCESS = 0, WD_FAILURE, WD_THREAD_FAILURE, WD_INTERNAL_FAILURE

} wd_status_t;

wd_status_t WDWatch(time_t interval, char *argv[]);

wd_status_t WDUnWatch(void);

#endif /* __WATCH_DOG_H__ */
