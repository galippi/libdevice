#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_can.h"

class DeviceBusCAN : public DeviceBusBase
{
public:
  DeviceBusCAN(const char *name, t_device_fd fd) : DeviceBusBase(name, fd)
  {
  }
};

class LibDeviceCAN : public LibDeviceBaseHandler
{
public:
  LibDeviceCAN(){}
  ~LibDeviceCAN();
  virtual const char *getName(void){return "can";}
  DeviceBusBase *createBus(const char *name, t_device_fd fd)
  {
    return new DeviceBusCAN(name, fd);
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
    return -1;
  }
};

static LibDeviceCAN CAN;

void registerCANDevice(void)
{
  LibDeviceRegisterDevice(&CAN);
}

LibDeviceCAN::~LibDeviceCAN()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceCAN::~LibDeviceCAN - not freed up bus %s!\n", bus[i]->getName().c_str());
    }
  }
}

