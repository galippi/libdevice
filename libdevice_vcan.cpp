#include "libdevice.h"

#include "libdevice_vcan.h"

#include "vcan.h"

class LibDevicevCAN : public LibDeviceBase
{
public:
  LibDevicevCAN(t_network_id netId) : LibDeviceBase(netId)
  {

  }
  ~LibDevicevCAN()
  {

  }
  const char *getName(void)
  {
    return "vcan";
  }
  t_device_fd open(const char *name, int flags)
  {
    return vcan_open(name, flags);
  }
  int close(t_device_fd fd)
  {
    return vcan_close(fd);
  }
  int read(t_device_fd fd, void *buf, unsigned int n)
  {
    return vcan_read(fd, buf, n);
  }
  int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    return vcan_write(fd, buf, n);
  }
  int ioctl(t_device_fd fd, unsigned long int request, void *data)
  {
    return vcan_ioctl(fd, request, (struct ifreq*)data);
  }
};

void registerVCANDevice(const char *networkName)
{
  LibDevicevCAN *vCAN = new LibDevicevCAN(network_open(networkName));
  LibDeviceRegisterDevice(vCAN);
}
