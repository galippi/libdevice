#include <assert.h>
#include <stdio.h>
#include <memory.h>

#include "libdevice.h"

LibDeviceNetwork::LibDeviceNetwork(const char *name, t_network_id id)
{
  this->name = name;
  this->id = id;
}

static LibDeviceNetworkList networkList;

LibDeviceNetwork *LibDeviceNetworkList::get(const char *networkName)
{
  auto netItPtr = networkMap.find(networkName);
  if (netItPtr != networkMap.end())
  {
    return netItPtr->second;
  }else
  {
    t_network_id id = networkList.size();
    LibDeviceNetwork *net = new LibDeviceNetwork(networkName, id);
    networkMap[networkName] = net;
    networkList.push_back(net);
    return net;
  }
}

LibDeviceNetwork *LibDeviceNetworkList::get(t_network_id netId)
{
  return networkList[netId];
}

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
  std::multimap <t_device_fd, t_BusWriteCallBack*> writeCallback;
};

class LibDevice
{
  public:
    LibDevice()
    {
    }
    ~LibDevice();
};

/* create static instance which call be called externally */

static std::map <std::string, LibDeviceBase*> deviceList;
static std::vector <LibDeviceDescr*> deviceDescr;
static DeviceFd fdHandler;
static LibDevice libDevice;

LibDeviceBase::LibDeviceBase(t_network_id netId)
{
  this->network = networkList.get(netId);
}

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

void LibDeviceRegisterDevice(LibDeviceBase *device)
{
  deviceList[device->getName()] = device;
}

static inline t_device_fd deviceIdxGetNextFree(void)
{
  return fdHandler.GetNextFreeFd();
}

static inline void deviceIdxRelease(t_device_fd fd)
{
  fdHandler.ReleaseFd(fd);
  return;
}

static inline int deviceIsValid(t_device_fd fd) /* const */
{
  if ((fd >= 0) && (fd < (int)deviceDescr.size()) && (deviceDescr[fd]->device != NULL))
    return 1;
  else
    return 0;
}

LibDevice::~LibDevice()
{
  for (unsigned idx = 0; idx < deviceDescr.size(); idx++)
  {
    if (deviceDescr[idx]->device != NULL)
    {
      fprintf(stderr, "Warning: device %s is not closed!\n", deviceDescr[idx]->device->getName());
    }
  }
}

int LibDeviceRegisterWriteCallback(t_device_fd fd, t_BusWriteCbFunc *cbFuncPtr, void *dataPtr)
{
  t_BusWriteCallBack *cbPtr = (t_BusWriteCallBack *)malloc(sizeof(t_BusWriteCallBack));
  cbPtr->cbFunc = cbFuncPtr;
  cbPtr->dataPtr = dataPtr;
  return device_ioctl(fd, e_DeviceBaseSetWriteCallback, cbPtr);
}

extern "C" t_network_id network_open(const char *networkName)
{
  return networkList.get(networkName)->getId();
}

extern "C" t_device_fd device_open(t_network_id netId, const char *device, int flags)
{
  std::string name = device;
  name = name + "/" + networkList.get(netId)->getName();
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
          if (idx < (int)deviceDescr.size())
          {
            deviceDescr[idx] = dev;
          }else
          {
            deviceDescr.push_back(dev);
          }
          return idx;
        }else
        { /* bus-open error -> nothing to do */
        }
        return fd;
      }
    }
  }
  return -1;
}

extern "C" int device_close(t_device_fd fd)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  int res = dev->device->close(dev->fd);
  deviceDescr[fd]->device = NULL;
  deviceIdxRelease(fd);
  return res;
}

extern "C" int device_ioctl(t_device_fd fd, unsigned long int request, void *data)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  switch (request)
  {
    case e_DeviceBaseSetWriteCallback:
    {
      t_BusWriteCallBack *cbPtr = (t_BusWriteCallBack*)data;
      cbPtr->fdBus = fd;
      cbPtr->fdDevice = dev->fd;
      dev->writeCallback.insert(std::pair<t_device_fd, t_BusWriteCallBack*>(fd, cbPtr));
      return 0;
    }
    default:
      return dev->device->ioctl(dev->fd, request, data);
  }
}

extern "C" int device_read(t_device_fd fd, void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  return dev->device->read(dev->fd, buf, n);
}

extern "C" int device_write(t_device_fd fd, const void *buf, unsigned int n)
{
  assert(deviceIsValid(fd));
  auto dev = deviceDescr[fd];
  int ret = dev->device->write(dev->fd, buf, n);
  auto itPtr = dev->writeCallback.find(fd);
  while(itPtr != dev->writeCallback.end())
  {
    t_BusWriteCallBack *cbPtr = itPtr->second;
    (*cbPtr->cbFunc)(cbPtr);
    itPtr++;
  }
  return ret;
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
    freeFdList.push_back(fd);
  }else
  { /* invalid descriptor -> error case */
    assert(0);
  }
}

LibDeviceBaseHandler::LibDeviceBaseHandler(t_network_id netId) :
  LibDeviceBase(networkList.get(netId))
{
}

t_device_fd LibDeviceBaseHandler::open(const char *name, int flags)
{
  auto devItPtr = busList.find(name);
  if (devItPtr != busList.end())
  {
    return devItPtr->second->attach();
  }
  t_device_fd fd = GetNextFreeFd();
  if (fd >= 0)
  {
    DeviceBusBase *busPtr = createBus(name, fd);
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

int LibDeviceBaseHandler::close(t_device_fd fd)
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
      ReleaseFd(fd);
    }
    return 0;
  }else
  {
    return -1;
  }
}

t_device_fd LibDeviceBaseHandler::GetNextFreeFd(void)
{
  if (freeFdList.size() != 0)
  {
    int idx = freeFdList.size() - 1;
    t_device_fd fd = freeFdList[idx];
    freeFdList.pop_back();
    return fd;
  }else
  {
    t_device_fd fd = bus.size();
    bus.push_back(NULL);
    return fd;
  }
}

void LibDeviceBaseHandler::ReleaseFd(t_device_fd fd)
{
  if ((fd >= 0) && (fd < (int)bus.size()))
  {
    freeFdList.push_back(fd);
  }else
  { /* invalid descriptor -> error case */
    assert(0);
  }
}
