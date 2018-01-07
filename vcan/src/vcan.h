
#ifndef VCAN_H_
#define VCAN_H_


#if defined(_MSC_VER) && (1400 == _MSC_VER)
  /* MSVS 2005 does not have sdtint.h */
  typedef signed char        int8_t;
  typedef signed short       int16_t;
  typedef signed int         int32_t;
  typedef signed long long   int64_t;
  typedef unsigned char      uint8_t;
  typedef unsigned short     uint16_t;
  typedef unsigned int       uint32_t;
  typedef unsigned long long uint64_t;
#else
#include "stdint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int vcan_ioctl(int fd, unsigned long int request, struct ifreq* ifr);

int vcan_bind(int fd, const struct sockaddr *addr, socklen_t len);

int vcan_socket(int domain, int type, int protocol);

int vcan_setsockopt(int fd, int level, int optname,
                    const void *optval, socklen_t optlen);

int vcan_fcntl(int fd, int cmd, int flag, int value);

int vcan_read(int fd, void *buf, unsigned int n);

int vcan_write(int fd, const void *buf, unsigned int n);

int vcan_device_create(const char *name);

void vcan_lib_reset(void);

int vcan_open(const char *name, int mode);

int vcan_close(int fd);

#ifdef __cplusplus
}
#endif


#endif
