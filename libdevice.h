#ifndef _LIBDEVICE_H_
#define _LIBDEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int t_network_id;
typedef int t_device_fd;

extern t_network_id network_open(const char *networkName);
extern t_device_fd device_open(t_network_id networkId, const char *name, int flags);
extern t_device_fd device_open_net(const char *networkName, const char *name, int flags);
extern int device_close(t_device_fd fd);
extern int device_ioctl(t_device_fd fd, unsigned long int request, void *data);
extern int device_read(t_device_fd fd, void *buf, unsigned int n);
extern int device_write(t_device_fd fd, const void *buf, unsigned int n);

#ifdef __cplusplus
}

#include <assert.h>

class LibDeviceNetwork;

class LibDeviceBase
{
  public:
    LibDeviceBase(LibDeviceNetwork *network){this->network = network;};
    LibDeviceBase(t_network_id netId);
    virtual ~LibDeviceBase();
    virtual const char *getName(void) = 0;
    virtual t_device_fd open(const char *name, int flags) = 0;
    virtual int close(t_device_fd fd) = 0;
    virtual int read(t_device_fd fd, void *buf, unsigned int n) = 0;
    virtual int write(t_device_fd fd, const void *buf, unsigned int n) = 0;
    virtual int ioctl(t_device_fd fd, unsigned long int request, void *data)
    { // not used function
      //assert(0);
      return -1;
    }
  private:
    LibDeviceNetwork *network;
};

class DeviceBusBase;

#include <string>
#include <map>
#include <vector>

class LibDeviceBase;

class LibDeviceNetwork
{
  public:
    LibDeviceNetwork(const char *name, t_network_id id);
    ~LibDeviceNetwork();
    const char *getName(){return name;};
    t_network_id getId() {return id;};
  private:
    const char *name;
    std::vector <LibDeviceBase*> devices;
    int id;
};

class LibDeviceNetworkList
{
  public:
    LibDeviceNetworkList(){};
    LibDeviceNetwork *get(const char *networkName);
    LibDeviceNetwork *get(t_network_id netId);
  private:
    std::map <std::string, LibDeviceNetwork*> networkMap;
    std::vector <LibDeviceNetwork*> networkList;
};

class LibDeviceBaseHandler : public LibDeviceBase
{
  public:
    LibDeviceBaseHandler(LibDeviceNetwork *network) : LibDeviceBase(network)
    {
    }
    LibDeviceBaseHandler(t_network_id netId);
    virtual DeviceBusBase *createBus(const char *name, t_device_fd fd) = 0;
    t_device_fd open(const char *name, int flags);
    int close(t_device_fd fd);
    std::map <std::string, DeviceBusBase*> busList;
    std::vector <DeviceBusBase*> bus;
    t_device_fd GetNextFreeFd(void);
    void ReleaseFd(t_device_fd);
protected:
    std::vector <t_device_fd> freeFdList;
};

extern void LibDeviceRegisterDevice(LibDeviceBase *device);

class DeviceFd
{
public:
  DeviceFd()
  {
    nextFree = 0;
  }
  t_device_fd GetNextFreeFd(void);
  void ReleaseFd(t_device_fd);
  std::vector <t_device_fd> freeFdList;

  t_device_fd nextFree;
};

typedef struct s_BusWriteCallBack t_BusWriteCallBack;

typedef void (t_BusWriteCbFunc)(t_BusWriteCallBack *);

struct s_BusWriteCallBack
{
  t_device_fd fdBus;
  t_device_fd fdDevice;
  void *dataPtr;
  t_BusWriteCbFunc *cbFunc;
};

typedef enum
{
  e_DeviceBaseSetWriteCallback,
  e_DeviceBaseSetSystemFd,
  e_DeviceBaseGetGlobalFd,
  e_DeviceBaseLast,
}e_DeviceBaseIoctl;

extern "C" int LibDeviceRegisterWriteCallback(t_device_fd fd, t_BusWriteCbFunc *cbFuncPtr, void *dataPtr);

class DeviceBusBase
{
public:
  DeviceBusBase(const char *name, int fd)
  {
    this->name = name;
    this->fd = fd;
    connectionCtr = 1;
  }
  int attach()
  {
    connectionCtr++;
    return fd;
  }
  int detach()
  {
    assert(connectionCtr > 0);
    connectionCtr--;
    return connectionCtr;
  }
  const std::string getName() const {return name;}
protected:
  int fd;
  std::string name;
  int connectionCtr;
};

class DeviceBusDouble : public DeviceBusBase
{
public:
  DeviceBusDouble(const char *name, int fd) : DeviceBusBase(name, fd)
  {
    val = 0;
  }
  double val;
};

class DeviceBusChar : public DeviceBusBase
{
public:
  DeviceBusChar(const char *name, int fd) : DeviceBusBase(name, fd)
  {
    val = 0;
  }
  char val;
};

class DeviceBusU32 : public DeviceBusBase
{
public:
  DeviceBusU32(const char *name, int fd) : DeviceBusBase(name, fd)
  {
    val = 0;
  }
  uint32_t val;
};

#endif /* __cplusplus */

#endif /* _LIBDEVICE_H_ */
