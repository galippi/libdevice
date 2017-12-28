#include <assert.h>
#include <stdio.h>

#include "libdevice.h"

/* create static instance which call be called externally */

std::map <std::string, LibDeviceBase*> LibDevice::deviceList;
std::map <int, LibDeviceBase*> LibDevice::deviceDescr;
std::map <int, int> LibDevice::deviceFd;
int LibDevice::deviceIdxNext = 0;
int LibDevice::simTimer_us = 0;
LibDevice libDevice;
DeviceFd LibDevice::fdHandler;

LibDeviceBase::~LibDeviceBase()
{ /* debug info: check that all bus is closed */
#if 0
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceBase::~LibDeviceBase - not freed up bus %s (device is %s)!\n", bus[i]->name.c_str(), getName());
    }
  }
#endif
}

void LibDevice::registerDevice(LibDeviceBase *device)
{
  deviceList[device->getName()] = device;
}

t_device_fd LibDevice::deviceIdxGetNextFree(void)
{
  return fdHandler.GetNextFreeFd();
}

void LibDevice::deviceIdxRelease(t_device_fd fd)
{
  fdHandler.ReleaseFd(fd);
  return;
}

int LibDevice::deviceIsValid(t_device_fd fd) /* const */
{
  if (deviceFd.find(fd) != deviceFd.end())
    return 1;
  else
    return 0;
}

t_device_fd LibDevice::open(const std::string& name, int flags)
{
  if (name.find("/dev/") == 0)
  { /* valid device prefix */
    std::string devName = name.substr(5);
    size_t devEnd = devName.find('/');
    if ((devEnd != 0) && (devEnd != std::string::npos))
    {
      devName = devName.substr(0, devEnd);
      auto devItPtr = deviceList.find(devName);
      if (devItPtr != deviceList.end())
      {
        std::string busName = name.substr(5+devName.length()+1);
        t_device_fd fd = devItPtr->second->open(busName.c_str(), flags);
        if (fd >= 0)
        {
          int idx = deviceIdxGetNextFree();
          deviceDescr[idx] = devItPtr->second;
          deviceFd[idx] = fd;
          return idx;
        }else
        { /* free up the already reserved index */
          //deviceIdxRelease(idx);
        }
        return fd;
      }
    }
  }
  return -1;
}

int LibDevice::close(t_device_fd fd)
{
  assert(deviceIsValid(fd));
  int res = deviceDescr[fd]->close(deviceFd[fd]);
  deviceDescr.erase(fd);
  deviceFd.erase(fd);
  deviceIdxRelease(fd);
  return res;
}

int LibDevice::ioctl(t_device_fd fd, unsigned long int request, void *data)
{
  assert(deviceIsValid(fd));
  return deviceDescr[fd]->ioctl(deviceFd[fd], request, data);
}

int LibDevice::read(t_device_fd fd, void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  return deviceDescr[fd]->read(deviceFd[fd], buf, n);
}

int LibDevice::write(t_device_fd fd, const void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  return deviceDescr[fd]->write(deviceFd[fd], buf, n);
}

LibDevice::~LibDevice()
{
  auto devDescrPtr = deviceDescr.begin();
  while (devDescrPtr != deviceDescr.end())
  {
    fprintf(stderr, "Warning: device %s is not closed!\n", devDescrPtr->second->getName());
    devDescrPtr++;
  }
}

extern "C" t_device_fd device_open(const char *device, int flags)
{
  return LibDevice::open(device, flags);
}

extern "C" int device_close(t_device_fd fd)
{
  return LibDevice::close(fd);
}

extern "C" int device_ioctl(t_device_fd fd, unsigned long int request, void *data)
{
  return LibDevice::ioctl(fd, request, data);
}

extern "C" int device_read(t_device_fd fd, void *buf, unsigned int n)
{
  return LibDevice::read(fd, buf, n);
}

extern "C" int device_write(t_device_fd fd, const void *buf, unsigned int n)
{
  return LibDevice::write(fd, buf, n);
}

#if 0
extern "C" void device_register(const t_deviceFuncTable deviceFuncTable)
{
  return LibDevice::registerDevice(fd, buf, n);
}
#endif

t_device_fd DeviceFd::GetNextFreeFd(void)
{
  if (freeFdList.size() != 0)
  {
    int idx = freeFdList.size() - 1;
    t_device_fd fd = freeFdList[idx];
    freeFdList.pop_back();
    return fd;
  }else
  {
    if (nextFree < 0x7fffffff)
    {
      t_device_fd fd = nextFree;
      nextFree++;
      return fd;
    }else
    { /* too many device -> error */
      return -1;
    }
  }
}

void DeviceFd::ReleaseFd(t_device_fd fd)
{
  if ((fd >= 0) && (fd < nextFree))
  {
    if (fd == (nextFree - 1))
    {
      nextFree --;
    }else
    {
      freeFdList.push_back(fd);
    }
  }else
  { /* invalid descriptor -> error case */
    assert(0);
  }
}
