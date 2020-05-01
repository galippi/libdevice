#include <map>

#include <memory.h>
#include <stdio.h>

#include "libdevice.h"

#include "libdevice_can.h"

class DeviceBusCANConnection
{
public:
  DeviceBusCANConnection(t_device_fd fd, unsigned size = 20)
  {
    inIdx = 0;
    outIdx = 0;
    this->size = size;
    Msgs = (t_LibDeviceCAN*)malloc(this->size * sizeof(t_LibDeviceCAN));
    writeCallbackData = NULL;
    this->fd = fd;
  }
  ~DeviceBusCANConnection()
  {
    free(Msgs);
  }
  const t_LibDeviceCAN *get()
  {
    if (num() != 0)
    {
      unsigned idx = outIdx++;
      if (outIdx == (2 * size))
        outIdx = 0;
      if (idx >= size)
        idx -= size;
      return &Msgs[idx];
    }
    return NULL;
  }
  int put(const t_LibDeviceCAN *msg)
  {
    if (num() != size)
    {
      unsigned idx = inIdx++;
      if (inIdx == (2 * size))
        inIdx = 0;
      if (idx >= size)
        idx -= size;
      Msgs[idx] = *msg;
      return 0;
    }else
    {
      overflow();
      return 1;
    }
  }
  unsigned num()
  {
    unsigned diff = inIdx - outIdx;
    if (diff > size)
      diff = (2 * size) + diff;
    return diff;
  }
  void overflow()
  { /* overflow handler */
  }
  t_device_fd fd;
  t_LibDeviceCAN *Msgs;
  unsigned inIdx, outIdx;
  unsigned size;
  t_DeviceCanWriteCbData *writeCallbackData;
};

class DeviceBusCAN : public DeviceBusBase
{
public:
  DeviceBusCAN(const char *name, t_device_fd fd) : DeviceBusBase(name, fd)
  {
  }
  std::map<t_device_fd, DeviceBusCANConnection*> connectionList;
};

class LibDeviceCAN : public LibDeviceBaseHandler
{
public:
  LibDeviceCAN(t_network_id netId) : LibDeviceBaseHandler(netId){}
  ~LibDeviceCAN();
  virtual const char *getName(void){return "can";}
  DeviceBusBase *createBus(const char *name, t_device_fd fd)
  {
    return NULL;
  }
  t_device_fd open(const char *name, int flags)
  {
    t_device_fd fd = GetNextFreeFd();
    if (fd < 0)
    { /* unable to reserve more descriptor */
      return -1;
    }
    auto devItPtr = busList.find(name);
    DeviceBusCAN *busPtr;
    if (devItPtr != busList.end())
    {
      busPtr = (DeviceBusCAN*)devItPtr->second;
    }else
    {
      busPtr = new DeviceBusCAN(name, fd);
      busList[name] = busPtr;
    }
    busPtr->connectionList.insert(std::pair<t_device_fd,DeviceBusCANConnection*>(fd, new DeviceBusCANConnection(fd)));
    bus[fd] = busPtr;
    return fd;
  }
  int close(t_device_fd fd)
  {
    DeviceBusCAN *busPtr = (DeviceBusCAN*)bus[fd];
    assert(busPtr != NULL);
    bus[fd] = NULL;;
    auto it = busPtr->connectionList.find(fd);
    if (it == busPtr->connectionList.end())
    {
      return -1;
    }
    busPtr->connectionList.erase(it);
    if (busPtr->connectionList.size() == 0)
    {
      busList.erase(busPtr->getName());
      delete busPtr;
    }
    return 0;
  }
  virtual int read(t_device_fd fd, void *buf, unsigned int n)
  {
    if (n != sizeof(t_LibDeviceCAN))
      return -1;
    DeviceBusCAN *busPtr = (DeviceBusCAN*)bus[fd];
    if (busPtr == NULL)
      return -1;
    auto conn = busPtr->connectionList;
    auto it = conn.find(fd);
    if(it != conn.end())
    {
      const t_LibDeviceCAN *msg = it->second->get();
      if (msg != NULL)
      {
        memcpy(buf, msg, n);
        return n;
      }else
        return 0;
    }
    return -1;
  }
  virtual int write(t_device_fd fd, const void *buf, unsigned int n)
  {
    if (n != sizeof(t_LibDeviceCAN))
      return -1;
    DeviceBusCAN *busPtr = (DeviceBusCAN*)bus[fd];
    if (busPtr == NULL)
      return -1;
    auto conn = busPtr->connectionList;
    auto it = conn.begin();
    const t_LibDeviceCAN *msg = (const t_LibDeviceCAN *)buf;
    int res = n;
    while(it != conn.end())
    {
      if (it->second->put(msg) != 0)
        res = -1;
      else
        if (it->second->writeCallbackData != NULL)
          it->second->writeCallbackData->deviceCanWriteCbFunc(it->second->fd, it->second->writeCallbackData->data);
      it++;
    }
    return res;
  }
  int ioctl(t_device_fd fd, unsigned long int request, void *data)
  {
    if (request == e_DeviceCanSetWriteCb)
    {
        DeviceBusCAN *busPtr = (DeviceBusCAN*)bus[fd];
        assert(busPtr != NULL);
        auto it = busPtr->connectionList.find(fd);
        assert(it != busPtr->connectionList.end());
        it->second->writeCallbackData = (t_DeviceCanWriteCbData*)data;
        return 0;
    }else
    {
      assert(0); /* not implemented request */
      return -1;
    }
  }
};

void registerCANDevice(const char *networkName)
{
  LibDeviceCAN *CAN = new LibDeviceCAN(network_open(networkName));
  LibDeviceRegisterDevice(CAN);
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

