#ifndef _LIBDEVICE_TIMER_H_
#define _LIBDEVICE_TIMER_H_

#include "libdevice.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int device_timer_open(const char *name, int flags);
extern int device_timer_close(int fd);
extern int device_timer_ioctl(int fd, unsigned long int request, void *data);
extern int device_timer_read(int fd, void *buf, unsigned int n);
extern int device_timer_write(int fd, const void *buf, unsigned int n);

void registerTimerDevice(void);

typedef struct s_TimerCallBack t_TimerCallBack;

typedef void (*t_DeviceTimerCbFunc)(t_TimerCallBack *);

typedef struct s_TimerCallBack
{
  //t_device_fd fd;
  uint32_t timer;
}t_DeviceTimerStep;

typedef struct
{
  //t_device_fd fd;
  uint32_t timer;
  t_DeviceTimerCbFunc cbFunc;
}t_DeviceTimerSetTimer;

typedef enum
{
  e_DeviceTimerStep,
  e_DeviceTimerSetTimer,
  e_DeviceTimerSetTimerDelta,
}e_DeviceTimerIoctl;

extern t_device_fd systemTimer;

int device_timer_step(int fd, uint32_t val);
static inline int device_systimer_step(uint32_t val)
{
  return device_timer_step(systemTimer, val);
}

int device_timer_setCb(int fd, e_DeviceTimerIoctl mode, uint32_t val, t_DeviceTimerCbFunc cb);
static inline int  device_systimer_setCb(e_DeviceTimerIoctl mode, uint32_t val, t_DeviceTimerCbFunc cb)
{
  return device_timer_setCb(systemTimer, mode, val, cb);
}

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_TIMER_H_ */
