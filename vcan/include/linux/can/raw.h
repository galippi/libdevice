
#ifndef RAW_H_
#define RAW_H_

#include "can.h"

#define SOL_CAN_RAW (SOL_CAN_BASE + CAN_RAW)

enum {
  CAN_RAW_RECV_OWN_MSGS
};

#endif /* RAW_H_ */
