/*
 * Virtual can driver implementation.
 */

#include <string.h>
#include <assert.h>

#include <sys/socket.h>

#include "vcan.h"

#define VCAN_SOCKET_BUFFER_SIZE 1024
#define VCAN_MAX_SOCKET_COUNT   50
#define VCAN_DEVICE_NAME_LENGTH 16
#define VCAN_MAX_DEVICE_COUNT   10

/* virtual can socket with buffer */
typedef struct {
  struct can_frame buffer[VCAN_SOCKET_BUFFER_SIZE];
  int read_pointer;
  int write_pointer;
  int deviceID;
  char receive_own_msgs;
} vcan_socket_t;

/* virtual can device */
typedef struct
{
  char name[VCAN_DEVICE_NAME_LENGTH];
} vcan_device_t;

/* Array of available sockets */
static vcan_socket_t vcan_sockets[VCAN_MAX_SOCKET_COUNT];
static int vcan_sockets_used = 0;

/* Array of available devices */
static vcan_device_t vcan_devices[VCAN_MAX_DEVICE_COUNT];
static int vcan_device_created = 0;

static void vcan_inc_buffer_pointer(int *pointer)
{
  *pointer = (*pointer + 1) % VCAN_SOCKET_BUFFER_SIZE;
}

static int vcan_filedesc_is_valid(int fd)
{
  return ( (fd >= 0) && (fd < VCAN_MAX_SOCKET_COUNT) );
}

int vcan_read(int fd, void *buf, unsigned int n)
{
  if (   vcan_filedesc_is_valid(fd)
      && (n == sizeof(struct can_frame))
      && (vcan_sockets[fd].read_pointer != vcan_sockets[fd].write_pointer))
  {
    *((struct can_frame*)(buf)) =
      vcan_sockets[fd].buffer[vcan_sockets[fd].read_pointer];
    vcan_inc_buffer_pointer(&(vcan_sockets[fd].read_pointer));
    return(n);
  } else {
    return(-1);
  }
}

static int vcan_add_data_to_buffer(vcan_socket_t *socket, const void *buf, unsigned int n)
{
  if (n == sizeof(struct can_frame) ) {
    socket->buffer[socket->write_pointer] = *(const struct can_frame*)(buf);
    vcan_inc_buffer_pointer(&(socket->write_pointer));
    if (socket->read_pointer == socket->write_pointer) {
      vcan_inc_buffer_pointer(&(socket->read_pointer));
      //todo: buffer overflow
    }
    return(n);
  } else {
    return(-1);
  }
}

static int vcan_device_lookup(const char *name)
{
  int deviceID = -1;
  int i;
  for (i = 0; i < vcan_device_created; i++) {
    if(strcmp(vcan_devices[i].name, name) == 0) {
      deviceID = i;
      break;
    }
  }
  return(deviceID);
}

int vcan_write(int fd, const void *buf, unsigned int n)
{
  int socketID;
  if(!vcan_filedesc_is_valid(fd)) return -1;
  for(socketID = 0; socketID < vcan_sockets_used; socketID++) {
    if(  (   (socketID != fd)
          || (vcan_sockets[fd].receive_own_msgs == 1) )
       &&(vcan_sockets[socketID].deviceID == vcan_sockets[fd].deviceID))
    {
      unsigned int written_bytes = vcan_add_data_to_buffer(&(vcan_sockets[socketID]), buf, n);
      if (written_bytes != n) return (-1);
    }
  }
  return(n);
}

int vcan_device_create(const char *name)
{
  int deviceID = vcan_device_lookup(name);
  if ((deviceID < 0) && (vcan_device_created < VCAN_MAX_DEVICE_COUNT)) {
    deviceID = vcan_device_created;
    strncpy(vcan_devices[deviceID].name, name, sizeof(vcan_devices[deviceID].name));
    vcan_device_created++;
  }
  assert(deviceID >= 0);//todo: handle more device
  return(deviceID);
}

void vcan_lib_reset(void)
{
  vcan_sockets_used = 0;
  vcan_device_created = 0;
}

int vcan_ioctl(int fd, unsigned long int request, struct ifreq* ifr)
{
  if (!vcan_filedesc_is_valid(fd)) return -1;
  if (request == SIOCGIFINDEX) {
    ifr->ifr_ifindex = vcan_device_lookup(ifr->ifr_name);
    return(ifr->ifr_ifindex != -1 ? 0 : -1);
  }
  return(-1);
}

int vcan_bind(int fd, const struct sockaddr *addr_, socklen_t len)
{
  (void)len;
  if (!vcan_filedesc_is_valid(fd)) return -1;
  const struct sockaddr_can *addr = (const struct sockaddr_can*)addr_;
  vcan_sockets[fd].deviceID = addr->can_ifindex;
  return(0);
}

int vcan_socket(int domain, int type, int protocol)
{
  int socketid = -1;
  if(domain == PF_CAN && type == SOCK_RAW && protocol == CAN_RAW)  {
    if(vcan_sockets_used < VCAN_MAX_SOCKET_COUNT) {
      socketid = vcan_sockets_used;
      vcan_sockets[socketid].deviceID = -1;
      vcan_sockets[socketid].read_pointer = 0;
      vcan_sockets[socketid].write_pointer = 0;
      vcan_sockets[socketid].receive_own_msgs = 0;
      vcan_sockets_used++;
    } else {
      assert(0);//todo: handle more sockets
    }
  }
  return(socketid);
}

int vcan_fcntl(int fd, int cmd, int flag, int value)
{
  (void)cmd;
  (void)flag;
  (void)value;
  if (!vcan_filedesc_is_valid(fd)) return -1;
  return(0);
}

int vcan_setsockopt(int fd, int level, int optname,
                    const void *optval, socklen_t optlen)
{
  (void)level;
  if (    vcan_filedesc_is_valid(fd)
       && (optlen == sizeof(int))
       && (optname == CAN_RAW_RECV_OWN_MSGS)
       && (fd < vcan_sockets_used) )
  {
    vcan_sockets[fd].receive_own_msgs = *(const int*)optval;
    return(0);
  } else {
    return(-1);
  }
}

int vcan_open(const char *name, int mode)
{
  (void)mode;
  int deviceId = vcan_device_create(name);
  if (deviceId >= 0)
  {
    int fd = vcan_socket( PF_CAN, SOCK_RAW, CAN_RAW );
    if (fd >= 0)
    {
      struct sockaddr_can addr;
      addr.can_family = AF_CAN;
      addr.can_ifindex = deviceId;
      vcan_bind(fd, (const struct sockaddr*)&addr, sizeof(addr) );
      vcan_fcntl(fd, F_SETFL, O_NONBLOCK, 1);
    }
    return fd;
  }else
    return -1;
}

int vcan_close(int fd)
{
  if (!vcan_filedesc_is_valid(fd)) return -1;
  vcan_sockets[fd].deviceID = -1; /* mark it as free */
  while ((vcan_sockets_used > 0) && (vcan_sockets[vcan_sockets_used - 1].deviceID < 0))
  {
    vcan_sockets_used--; /* free up the marked descriptor */
  }
  return 0;
}
