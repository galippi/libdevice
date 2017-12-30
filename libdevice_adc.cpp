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
    auto devItPtr = busList.find(name);
    if (devItPtr != busList.end())
    {
      return devItPtr->second->attach();
    }
    t_device_fd fd = fdHandler.GetNextFreeFd();
    if (fd >= 0)
    {
      DeviceBusDouble *busPtr = new DeviceBusDouble(name, fd);
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
    auto busPtr = bus[fd];
    if (busPtr != NULL)
    {
      if (busPtr->detach() != 0)
      { /* other connection remains -> keep the bus  (nothing to do) */
      }else
      { /* last connection -> free up the bus */
        busList.erase(busPtr->getName());
        delete busPtr;
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
    if ((n == sizeof(double)) &&
        (bus[fd] != NULL))
    {
      double val = bus[fd]->val;
      memcpy(buf, &val, n);
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    if ((n == sizeof(double)) &&
        (bus[fd] != NULL))
    {
      double val;
      memcpy(&val, buf, sizeof(val));
      bus[fd]->val = val;
      return sizeof(val);
    }else
    {
      return -1;
    }
  }

protected:
  std::map <std::string, DeviceBusDouble*> busList;
  std::vector <DeviceBusDouble*> bus;
  DeviceFd fdHandler;
};

static LibDeviceAdc adc;

void registerAdcDevice(void)
{
  LibDeviceRegisterDevice(&adc);
}

LibDeviceAdc::~LibDeviceAdc()
{ /* debug info: check that all bus is closed */
  for (size_t i = 0; i < bus.size(); i++)
  {
    if (bus[i] != NULL)
    { /* bus is not freed up */
      fprintf(stderr, "Warning: LibDeviceAdc::~LibDeviceAdc - not freed up bus %s!\n", bus[i]->getName().c_str());
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
