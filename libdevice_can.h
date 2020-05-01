#ifndef _LIBDEVICE_CAN_H_
#define _LIBDEVICE_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

void registerCANDevice(const char *networkName);

typedef struct
{
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
}t_LibDeviceCAN;

typedef enum
{
  e_DeviceCanStep = e_DeviceBaseLast,
  e_DeviceCanSetWriteCb,
}e_DeviceCanIoctl;

typedef struct
{
  void (*deviceCanWriteCbFunc)(t_device_fd fd, void *data);
  void *data;
}t_DeviceCanWriteCbData;

void device_can_send(t_device_fd fd, t_LibDeviceCAN msg);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_CAN_H_ */
