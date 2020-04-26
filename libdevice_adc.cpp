#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_adc.h"

class LibDeviceAdc : public LibDeviceBaseHandler
{
public:
  LibDeviceAdc(LibDeviceNetwork *network) : LibDeviceBaseHandler(network){}
  LibDeviceAdc(t_network_id netId) : LibDeviceBaseHandler(netId){}
  ~LibDeviceAdc();
  virtual const char *getName(void){return "adc";}
  DeviceBusBase *createBus(const char *name, t_device_fd fd)
  {
    return new DeviceBusDouble(name, fd);
  }

  virtual int read(t_device_fd fd, void *buf, unsigned int n)
  {
    DeviceBusDouble *dev = (DeviceBusDouble *)bus[fd];
    if ((n == sizeof(double)) &&
        (dev != NULL))
    {
      double val = dev->val;
      memcpy(buf, &val, n);
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    DeviceBusDouble *dev = (DeviceBusDouble *)bus[fd];
    if ((n == sizeof(double)) &&
        (dev != NULL))
    {
      double val;
      memcpy(&val, buf, sizeof(val));
      dev->val = val;
      return sizeof(val);
    }else
    {
      return -1;
    }
  }
};

void registerAdcDevice(const char *networkName)
{
	LibDeviceAdc *adc = new LibDeviceAdc(network_open(networkName));
	LibDeviceRegisterDevice(adc);
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
