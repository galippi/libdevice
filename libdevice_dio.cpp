#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_dio.h"

class LibDeviceDio : public LibDeviceBaseHandler
{
public:
  LibDeviceDio(){}
  ~LibDeviceDio();
  virtual const char *getName(void){return "dio";}
  DeviceBusBase *createBus(const char *name, t_device_fd fd)
  {
    return new DeviceBusChar(name, fd);
  }
  virtual int read(t_device_fd fd, void *buf, unsigned int n)
  {
    DeviceBusChar *dev = (DeviceBusChar *)bus[fd];
    if ((n == sizeof(char)) &&
        (dev != NULL))
    {
      char val = dev->val;
      memcpy(buf, &val, sizeof(val));
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    DeviceBusChar *dev = (DeviceBusChar *)bus[fd];
    if ((n == sizeof(char)) &&
        (dev != NULL))
    {
      char val;
      memcpy(&val, buf, sizeof(val));
      dev->val = val;
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
};

static LibDeviceDio dio;

void registerDioDevice(void)
{
  LibDeviceRegisterDevice(&dio);
}

LibDeviceDio::~LibDeviceDio()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceDio::~LibDeviceDio - not freed up bus %s!\n", bus[i]->getName().c_str());
    }
  }
}

int device_dio_write_char(int fd, char val)
{
  return device_write(fd, &val, sizeof(val));
}

char device_dio_read_char(int fd)
{
  char val;
  device_read(fd, &val, sizeof(val));
  return val;
}
