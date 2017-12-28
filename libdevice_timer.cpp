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

class LibDeviceTimer : public LibDeviceBase
{
public:
  LibDeviceTimer(){}
  ~LibDeviceTimer();
  virtual const char *getName(void){return "timer";}
  t_device_fd open(const char *name, int flags)
  {
    auto devItPtr = busList.find(name);
    if (devItPtr != busList.end())
    {
      devItPtr->second->connectionCtr++;
      return devItPtr->second->fd;
    }
    t_device_fd fd = fdHandler.GetNextFreeFd();
    if (fd >= 0)
    {
      DeviceBusTimer *busPtr = new DeviceBusTimer(name, fd);
      busList[name] = busPtr;
      if ((unsigned)fd >= bus.size())
      {
        bus.push_back(busPtr);
      }else
      {
        bus[fd] = busPtr;
      }
    }
    return fd;
  }
  virtual int close(t_device_fd fd)
  {
    if (bus[fd] != NULL)
    {
      if (bus[fd]->connectionCtr != 1)
      { /* other connection remains -> release this connection, but keep the bus */
        bus[fd]->connectionCtr--;
      }else
      { /* last connection -> free up the bus */
        delete bus[fd];
        bus[fd] = NULL;
        fdHandler.ReleaseFd(fd);
      }
      return 0;
    }else
    {
      return -1;
    }
  }
  virtual int read(t_device_fd fd, void *buf, unsigned int n)
  {
    return -1;
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    return -1;
  }
  int ioctl(t_device_fd fd, unsigned long int request, void *data)
  {
    switch (request)
    {
      case e_DeviceTimerStep:
      {
        t_DeviceTimerStep *ptr = (t_DeviceTimerStep*)data;
        uint32_t newVal = bus[fd]->val + ptr->timer;
        auto itPtr = bus[fd]->interruptCallback.begin();
        while ((itPtr != bus[fd]->interruptCallback.end()) && (itPtr->first <= newVal))
        {
          uint32_t t = itPtr->first;
          bus[fd]->val = t;
          //void *ptr = itPtr->second;
           //t_DeviceTimerCbFunc *funcPtr = (t_DeviceTimerCbFunc*)ptr;
           //funcPtr(NULL);
          t_TimerCallBack cbData = {t};
          (*itPtr->second)(&cbData);
          bus[fd]->interruptCallback.erase(itPtr);
          itPtr++;
        }
        bus[fd]->val = newVal;
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
          if (timer <= bus[fd]->val)
          { /* invalid timer value -> specified time is already ellapsed */
            return -1;
          }
        }else
        {
          timer = bus[fd]->val + ptr->timer;
        }
        bus[fd]->interruptCallback.insert(std::pair<uint32_t, t_DeviceTimerCbFunc>(timer, ptr->cbFunc));
        return 0;
      }
      default:
        return -1;
    }
  }

protected:
  std::map <std::string, DeviceBusTimer*> busList;
  std::vector <DeviceBusTimer*> bus;
  DeviceFd fdHandler;
};

static LibDeviceTimer timer;

t_device_fd systemTimer = -1;
void registerTimerDevice(void)
{
  LibDevice::registerDevice(&timer);
  systemTimer = device_open("/dev/timer/system", 0);
}

LibDeviceTimer::~LibDeviceTimer()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceTimer::~LibDeviceTimer - not freed up bus %s!\n", bus[i]->name.c_str());
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
