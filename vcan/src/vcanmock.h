
#ifndef VCANMOCK_H_
#define VCANMOCK_H_

#ifndef USE_REAL_SOCKET_CAN_INTERFACE
#define ioctl(fd,request,pifr) vcan_ioctl(fd,request,pifr)
#define bind(fd,paddr,len) vcan_bind(fd,paddr,len)
#define socket(dom,type,prot) vcan_socket(dom,type,prot)
#define setsockopt(fd,level,optname,optval,optlen) \
  vcan_setsockopt(fd,level,optname,optval,optlen)
#define fcntl(fd,cmd,flag,value) vcan_fcntl(fd,cmd,flag,value)
#define read(fd,pbuf,n) vcan_read(fd,pbuf,n)
#define write(fd,pbuf,n) vcan_write(fd,pbuf,n)
#define close(fd) vcan_close(fd)
#endif

#endif
