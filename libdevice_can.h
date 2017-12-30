#ifndef _LIBDEVICE_CAN_H_
#define _LIBDEVICE_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

void registerCANDevice(void);

typedef struct
{
  uint32_t id;
  uint8_t  dlc;
  uint8_t  data[8];
}t_LibDeviceCAN;

void device_can_send(t_device_fd fd, t_LibDeviceCAN msg);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_CAN_H_ */
