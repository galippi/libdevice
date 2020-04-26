#ifndef _LIBDEVICE_VCAN_H_
#define _LIBDEVICE_VCAN_H_

#include "linux_can.h"

#ifdef __cplusplus
extern "C" {
#endif

void registerVCANDevice(const char *netName);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_VCAN_H_ */
