#include "canlogger_libdevice.h"
#include "libdevice_timer.h"

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

void CanLogger::close(void)
{
  if (fout != NULL)
    fclose(fout);
  fout = NULL;
  for (int idx = 0; idx < nextDevIdx; idx++)
  {
    device_close(devices[idx]->fd);
    free(devices[idx]->cbData);
    delete devices[idx];
  }
  nextDevIdx = 0;
}

CanLogger::~CanLogger()
{
  close();
}

void CanLogger::canWriteCb(CanLoggerCbData *cbData, t_LibDeviceCAN msg)
{
  if (fout != NULL)
  {
    uint32_t t;
    assert(device_read(systemTimer, &t, sizeof(t)) == sizeof(t));
    fprintf(fout, "%4u.%06u %2d %c%08x %d", t/1000000, t % 1000000, cbData->devIdx, (msg.id & 0x80000000) ? 'x' : ' ', msg.id, msg.dlc);
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
