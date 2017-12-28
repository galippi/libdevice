#include <assert.h>
#include <stdio.h>

#include "libdevice.h"

class LibDeviceDescr
{
public:
  LibDeviceDescr(LibDeviceBase *dev, t_device_fd fd)
  {
    this->fd = fd;
    device = dev;
  }
  t_device_fd fd;
  LibDeviceBase *device;
};

/* create static instance which call be called externally */

std::map <std::string, LibDeviceBase*> LibDevice::deviceList;
std::map <int, LibDeviceDescr*> LibDevice::deviceDescr;
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
  if (deviceDescr.find(fd) != deviceDescr.end())
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
          LibDeviceDescr *dev = new LibDeviceDescr(devItPtr->second, fd);
          deviceDescr[idx] = dev;
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
  auto dev = deviceDescr[fd];
  int res = dev->device->close(dev->fd);
  deviceDescr.erase(fd);
  deviceIdxRelease(fd);
  return res;
}

int LibDevice::ioctl(t_device_fd fd, unsigned long int request, void *data)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  return dev->device->ioctl(dev->fd, request, data);
}

int LibDevice::read(t_device_fd fd, void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  return dev->device->read(dev->fd, buf, n);
}

int LibDevice::write(t_device_fd fd, const void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  return dev->device->write(dev->fd, buf, n);
}

LibDevice::~LibDevice()
{
  auto devDescrPtr = deviceDescr.begin();
  while (devDescrPtr != deviceDescr.end())
  {
    fprintf(stderr, "Warning: device %s is not closed!\n", devDescrPtr->second->device->getName());
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
