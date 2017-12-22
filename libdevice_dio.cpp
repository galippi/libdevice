#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_dio.h"

class LibDeviceDio : public LibDeviceBase
{
public:
  LibDeviceDio(){}
  ~LibDeviceDio();
  virtual const char *getName(void){return "dio";}
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
      DeviceBus *busPtr = new DeviceBus(name, fd);
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
    if ((n == sizeof(char)) &&
        (bus[fd] != NULL))
    {
      char val = bus[fd]->val;
      memcpy(buf, &val, sizeof(val));
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    if ((n == sizeof(char)) &&
        (bus[fd] != NULL))
    {
      char val;
      memcpy(&val, buf, sizeof(val));
      bus[fd]->val = val;
      return sizeof(val);
    }else
    {
      return -1;
    }
  }

protected:
  std::map <std::string, DeviceBus*> busList;
  std::vector <DeviceBus*> bus;
  DeviceFd fdHandler;
};

static LibDeviceDio dio;

void registerDioDevice(void)
{
  LibDevice::registerDevice(&dio);
}

LibDeviceDio::~LibDeviceDio()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceDio::~LibDeviceDio - not freed up bus %s!\n", bus[i]->name.c_str());
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
