
#ifndef CAN_H
#define CAN_H

#include <stdint.h>

typedef uint32_t canid_t;

struct can_frame {
  canid_t can_id;
  uint8_t can_dlc; /** data length */
  uint8_t data[8] __attribute__((aligned(8)));
};

#define CAN_RAW  1
#define SOL_CAN_BASE 100

struct sockaddr_can {
  uint16_t   can_family;
  int        can_ifindex;
  union {
    struct { canid_t rx_id, tx_id; } tp;
  } can_addr;
};

/* socket.h defines */
#define SOCK_RAW 3
#define PF_CAN   29
#define AF_CAN   PF_CAN

#endif
