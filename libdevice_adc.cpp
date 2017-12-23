#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_adc.h"

class LibDeviceAdc : public LibDeviceBase
{
public:
  LibDeviceAdc(){}
  ~LibDeviceAdc();
  virtual const char *getName(void){return "adc";}
  t_device_fd open(const char *name, int flags)
  {
    auto devItPtr = busListAdc.find(name);
    if (devItPtr != busListAdc.end())
    {
      devItPtr->second->connectionCtr++;
      return devItPtr->second->fd;
    }
    t_device_fd fd = fdHandler.GetNextFreeFd();
    if (fd >= 0)
    {
      DeviceBusDouble *busPtr = new DeviceBusDouble(name, fd);
      busListAdc[name] = busPtr;
      if ((unsigned)fd >= busAdc.size())
      {
        busAdc.push_back(busPtr);
      }else
      {
        busAdc[fd] = busPtr;
      }
    }
    return fd;
  }
  virtual int close(t_device_fd fd)
  {
    if (busAdc[fd] != NULL)
    {
      if (busAdc[fd]->connectionCtr != 1)
      { /* other connection remains -> release this connection, but keep the bus */
        busAdc[fd]->connectionCtr--;
      }else
      { /* last connection -> free up the bus */
        delete busAdc[fd];
        busAdc[fd] = NULL;
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
    if ((n == sizeof(double)) &&
        (busAdc[fd] != NULL))
    {
      double val = busAdc[fd]->val;
      memcpy(buf, &val, sizeof(val));
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    if ((n == sizeof(double)) &&
        (busAdc[fd] != NULL))
    {
      double val;
      memcpy(&val, buf, sizeof(val));
      busAdc[fd]->val = val;
      return sizeof(val);
    }else
    {
      return -1;
    }
  }

protected:
  std::map <std::string, DeviceBusDouble*> busListAdc;
  std::vector <DeviceBusDouble*> busAdc;
  DeviceFd fdHandler;
};

static LibDeviceAdc adc;

void registerAdcDevice(void)
{
  LibDevice::registerDevice(&adc);
}

LibDeviceAdc::~LibDeviceAdc()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < busAdc.size(); i++)
  {
    if (busAdc[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceAdc::~LibDeviceAdc - not freed up bus %s!\n", busAdc[i]->name.c_str());
    }
  }
}

int device_adc_write_double(int fd, double val)
{
  return device_write(fd, &val, sizeof(val));
}

double device_adc_read_double(int fd)
{
  double val;
  device_read(fd, &val, sizeof(val));
  return val;
}
