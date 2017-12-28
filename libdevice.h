#ifndef _LIBDEVICE_H_
#define _LIBDEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int t_device_fd;

extern t_device_fd device_open(const char *name, int flags);
extern int device_close(t_device_fd fd);
extern int device_ioctl(t_device_fd fd, unsigned long int request, void *data);
extern int device_read(t_device_fd fd, void *buf, unsigned int n);
extern int device_write(t_device_fd fd, const void *buf, unsigned int n);

#ifdef __cplusplus
}

#include <assert.h>

class LibDeviceBase
{
  public:
    LibDeviceBase(){};
    virtual ~LibDeviceBase();
    virtual const char *getName(void) = 0;
    virtual t_device_fd open(const char *name, int flags) = 0;
    virtual int close(t_device_fd fd) = 0;
    virtual int read(t_device_fd fd, void *buf, unsigned int n) = 0;
    virtual int write(t_device_fd fd, const void *buf, unsigned int n) = 0;
    virtual int ioctl(t_device_fd fd, unsigned long int request, void *data)
    { // not used function
      assert(0);
    }
};

#include <vector>

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

#include <string>
#include <map>

class LibDeviceDescr;

class LibDevice
{
  public:
    LibDevice()
    {
    }
    ~LibDevice();
    static void registerDevice(LibDeviceBase *device);
    static t_device_fd open(const std::string& name, int flags);
    static t_device_fd open(const char *name, int flags)
    {
      std::string str(name);
      return open(str, flags);
    }
    static int close(t_device_fd fd);
    static int read(t_device_fd fd, void *buf, unsigned int n);
    static int write(t_device_fd fd, const void *buf, unsigned int n);
    static int ioctl(t_device_fd fd, unsigned long int request, void *data);
    static int deviceIsValid(t_device_fd fd) /*const*/;
  protected:
    static int deviceIdxGetNextFree(void);
    static void deviceIdxRelease(int fd);
    static std::map <std::string, LibDeviceBase*> deviceList;
    static std::map <int, LibDeviceDescr*> deviceDescr;
    static DeviceFd fdHandler;
};

class DeviceBusBase
{
public:
  DeviceBusBase(const char *name, int fd)
  {
    this->name = name;
    this->fd = fd;
    connectionCtr = 1;
  }
  int fd;
  int connectionCtr;
  std::string name;
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
