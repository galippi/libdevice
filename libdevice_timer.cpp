#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_timer.h"

class DeviceBusTimer : public DeviceBusU32
{
public:
  DeviceBusTimer(const char *name, int fd) : DeviceBusU32(name, fd)
  {
  }
  std::multimap <uint32_t, t_DeviceTimerCbFunc> interruptCallback;
};

class LibDeviceTimer : public LibDeviceBaseHandler
{
public:
  LibDeviceTimer(t_network_id netId) : LibDeviceBaseHandler(netId){}
  ~LibDeviceTimer();
  virtual const char *getName(void){return "timer";}
  DeviceBusBase *createBus(const char *name, t_device_fd fd)
  {
    return new DeviceBusTimer(name, fd);
  }
  virtual int read(t_device_fd fd, void *buf, unsigned int n)
  {
    DeviceBusTimer *dev = (DeviceBusTimer *)bus[fd];
    if (n == sizeof(dev->val))
    {
      memcpy(buf, &(dev->val), n);
      return n;
    }
    return -1;
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    return -1;
  }
  int ioctl(t_device_fd fd, unsigned long int request, void *data)
  {
    DeviceBusTimer *dev = (DeviceBusTimer *)bus[fd];
    switch (request)
    {
      case e_DeviceTimerStep:
      {
        t_DeviceTimerStep *ptr = (t_DeviceTimerStep*)data;
        uint32_t newVal = dev->val + ptr->timer;
        auto itPtr = dev->interruptCallback.begin();
        while ((itPtr != dev->interruptCallback.end()) && (itPtr->first <= newVal))
        {
          uint32_t t = itPtr->first;
          dev->val = t;
          //void *ptr = itPtr->second;
           //t_DeviceTimerCbFunc *funcPtr = (t_DeviceTimerCbFunc*)ptr;
           //funcPtr(NULL);
          t_TimerCallBack cbData = {t};
          (*itPtr->second)(&cbData);
          dev->interruptCallback.erase(itPtr);
          itPtr++;
        }
        dev->val = newVal;
        return 0;
      }
      case e_DeviceTimerSetTimer:
      case e_DeviceTimerSetTimerDelta:
      {
        t_DeviceTimerSetTimer *ptr = (t_DeviceTimerSetTimer*)data;
        uint32_t timer;
        if (request == e_DeviceTimerSetTimer)
        {
          timer = ptr->timer;
          if (timer <= dev->val)
          { /* invalid timer value -> specified time is already ellapsed */
            return -1;
          }
        }else
        {
          timer = dev->val + ptr->timer;
        }
        dev->interruptCallback.insert(std::pair<uint32_t, t_DeviceTimerCbFunc>(timer, ptr->cbFunc));
        return 0;
      }
      default:
        return -1;
    }
  }
};

t_device_fd systemTimer = -1;
void registerTimerDevice(const char *networkName)
{
  t_network_id netId = network_open(networkName);
  LibDeviceTimer *timer = new LibDeviceTimer(netId);
  LibDeviceRegisterDevice(timer);
  if (systemTimer < 0)
  {
    t_network_id netIdWorld = network_open("World");
    systemTimer = device_open(netIdWorld, "/dev/timer/system", 0);
  }
}

LibDeviceTimer::~LibDeviceTimer()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceTimer::~LibDeviceTimer - not freed up bus %s!\n", bus[i]->getName().c_str());
    }
  }
}

int  device_timer_step(int fd, uint32_t val)
{
  t_DeviceTimerStep timerStep = {val};
  return device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep);
}

int  device_timer_setCb(int fd, e_DeviceTimerIoctl mode, uint32_t val, t_DeviceTimerCbFunc cb)
{
  t_DeviceTimerSetTimer timer = {val, cb};
  return device_ioctl(systemTimer, mode, (void*)&timer);
}
