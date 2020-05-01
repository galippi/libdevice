#ifndef _CANLOGGER_LIBDEVICE_H_
#define _CANLOGGER_LIBDEVICE_H_

#include <stdio.h>

#include "libdevice.h"
#include "libdevice_can.h"

#define CANLOGGER_DEVICES_MAX 20

class CanLoggerCbData;

class CanLogger
{
  public:
    CanLogger(const char *filename);
    ~CanLogger();
    void addBus(t_network_id id, const char *busname);
    void canWriteCb(CanLoggerCbData *cbData, t_LibDeviceCAN msg);
  private:
    FILE *fout;
    t_device_fd devices[CANLOGGER_DEVICES_MAX];
    int nextDevIdx;
};

#endif /* _CANLOGGER_LIBDEVICE_H_ */
