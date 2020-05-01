#ifndef _CANLOGGER_LIBDEVICE_H_
#define _CANLOGGER_LIBDEVICE_H_

#include <stdio.h>

#include "libdevice.h"
#include "libdevice_can.h"

class CanLogger;

class CanLoggerCbData
{
  public:
    CanLoggerCbData(t_device_fd fd, int devIdx, CanLogger *logger, t_DeviceCanWriteCbData *cbData)
    {
      this->fd = fd;
      this->devIdx = devIdx;
      this->logger = logger;
      this->cbData = cbData;
    }
    void callBack(t_LibDeviceCAN msg);
    t_device_fd fd;
    int devIdx;
    CanLogger *logger;
    t_DeviceCanWriteCbData *cbData;
};

#define CANLOGGER_DEVICES_MAX 20

class CanLogger
{
  public:
    CanLogger(const char *filename);
    ~CanLogger();
    void addBus(t_network_id id, const char *busname);
    void canWriteCb(CanLoggerCbData *cbData, t_LibDeviceCAN msg);
  private:
    FILE *fout;
    CanLoggerCbData *devices[CANLOGGER_DEVICES_MAX];
    int nextDevIdx;
};

#endif /* _CANLOGGER_LIBDEVICE_H_ */
