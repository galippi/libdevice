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
    virtual const char *getName(void) = 0;
    virtual t_device_fd open(const char *name, int flags) = 0;
    virtual int close(t_device_fd fd) = 0;
    virtual int read(t_device_fd fd, void *buf, unsigned int n) = 0;
    virtual int write(t_device_fd fd, const void *buf, unsigned int n) = 0;
    virtual int ioctl(t_device_fd fd, unsigned long int request, void *data)
    { // not used function
      assert(0);
    }
    virtual void step(double dT)
    { /* do nothing by default */
    }
};

#include <string>
#include <map>

class LibDevice
{
  public:
    LibDevice()
    {
      reset();
    }
    static void registerDevice(LibDeviceBase *device);
    static t_device_fd open(const std::string& name, int flags);
    static t_device_fd open(const char *name, int flags)
    {
      //std::string str = name;
      std::string str(name);
      return open(str, flags);
    }
    static int close(t_device_fd fd);
    static int read(t_device_fd fd, void *buf, unsigned int n);
    static int write(t_device_fd fd, const void *buf, unsigned int n);
    static int ioctl(t_device_fd fd, unsigned long int request, void *data);
    static int deviceIsValid(t_device_fd fd) /*const*/;
    static void step(double dT);
    static void step_us(uint32_t dT);
    static void step_ns(uint32_t dT);
    static void reset(){};
    static void registerInterrupt(uint32_t dt);
  protected:
    static int deviceIdxGetNextFree(void);
    static void deviceIdxRelease(int fd);
    static std::map <std::string, LibDeviceBase*> deviceList;
    static std::map <int, LibDeviceBase*> deviceDescr;
    static std::map <int, int> deviceFd;
    static int deviceIdxNext;
    static int simTimer_us;
    static std::multimap <uint32_t, int> interruptCallback;
};

class DeviceBus
{
public:
  DeviceBus(const char *name, int fd)
  {
    this->name = name;
    this->fd = fd;
    val = 0;
    connectionCtr = 1;
  }
  double val;
  int fd;
  int connectionCtr;
protected:
  std::string name;
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
  int ReleaseFd(t_device_fd);
  std::vector <t_device_fd> freeFdList;

  t_device_fd nextFree;
};

#endif /* __cplusplus */

#endif /* _LIBDEVICE_H_ */
