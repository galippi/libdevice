#include "canlogger_libdevice.h"

static void canWriteCbFunc(t_device_fd fd, void *data)
{
  t_LibDeviceCAN msg;
  while(device_read(fd, &msg, sizeof(msg)) == sizeof(msg))
  {
    CanLoggerCbData *cbData = (CanLoggerCbData*)data;
    CanLogger *logger = cbData->logger;
    logger->canWriteCb(cbData, msg);
  }
}

CanLogger::CanLogger(const char *filename)
{
  nextDevIdx = 0;
  fout = fopen(filename, "wt");
  if (fout == NULL)
  {
    throw 1;
  }
}

CanLogger::~CanLogger()
{
  if (fout != NULL)
    fclose(fout);
  for (int idx = 0; idx < nextDevIdx; idx++)
  {
    device_close(devices[idx]->fd);
    free(devices[idx]->cbData);
    delete devices[idx];
  }
}

void CanLogger::canWriteCb(CanLoggerCbData *cbData, t_LibDeviceCAN msg)
{
  if (fout != NULL)
  {
    fprintf(fout, " 1.123456 %2d %c%08x %d", cbData->devIdx, (msg.id & 0x80000000) ? 'x' : ' ', msg.id, msg.dlc);
    for (uint32_t i = 0; i < msg.dlc; i++)
        fprintf(fout, " %02x", msg.data[i]);
    fprintf(fout, "\n");
  }
}

void CanLogger::addBus(t_network_id net, const char *busname)
{
  t_DeviceCanWriteCbData *cbData = (t_DeviceCanWriteCbData*)malloc(sizeof(t_DeviceCanWriteCbData));
  int devIdx = nextDevIdx++;
  devices[devIdx] = new CanLoggerCbData(device_open(net, busname, 0), devIdx, this, cbData);
  assert(devices[devIdx] != NULL);
  cbData->deviceCanWriteCbFunc = canWriteCbFunc;
  cbData->data = devices[devIdx];
  device_ioctl(devices[devIdx]->fd, e_DeviceCanSetWriteCb, (void*)cbData);
}
