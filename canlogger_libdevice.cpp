#include "canlogger_libdevice.h"

class CanLoggerCbData
{
  public:
    CanLoggerCbData(t_device_fd fd, int devIdx, CanLogger *logger)
    {
      this->fd = fd;
      this->devIdx = devIdx;
      this->logger = logger;
    }
    t_device_fd fd;
    int devIdx;
    CanLogger *logger;
};

static void canWriteCbFunc(t_device_fd fd, void *data)
{
  t_LibDeviceCAN msg;
  while(device_read(fd, &msg, sizeof(msg)) == sizeof(msg))
  {
    CanLoggerCbData *cbData = (CanLoggerCbData*)data;
    cbData->logger->canWriteCb(cbData, msg);
  }
}

CanLogger::CanLogger(const char *filename)
{
  fout = fopen(filename, "wt");
  if (fout == NULL)
  {
    throw 1;
  }
  nextDevIdx = 0;
}

CanLogger::~CanLogger()
{
  if (fout != NULL)
    fclose(fout);
}

void CanLogger::canWriteCb(CanLoggerCbData *cbData, t_LibDeviceCAN msg)
{
  if (fout != NULL)
    fprintf(fout, " 1.123456 %2d %d %c%08x %02x %02x\n", cbData->devIdx, (msg.id & 0x80000000) ? 'x' : ' ', msg.id, msg.dlc, msg.data[0], msg.data[1]);
}

void CanLogger::addBus(t_network_id net, const char *busname)
{
  int devIdx = nextDevIdx++;
  devices[devIdx] = device_open(net, busname, 0);
  assert(devices[devIdx] >= 0);
  t_DeviceCanWriteCbData *cbData = (t_DeviceCanWriteCbData*)malloc(sizeof(t_DeviceCanWriteCbData));
  cbData->deviceCanWriteCbFunc = canWriteCbFunc;
  cbData->data = new CanLoggerCbData(devices[devIdx], devIdx, this);
  device_ioctl(devices[devIdx], e_DeviceCanSetWriteCb, (void*)cbData);
}
