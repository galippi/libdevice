#ifndef _LIBDEVICE_DIO_H_
#define _LIBDEVICE_DIO_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int device_dio_open(const char *name, int flags);
extern int device_dio_close(int fd);
extern int device_dio_ioctl(int fd, unsigned long int request, void *data);
extern int device_dio_read(int fd, void *buf, unsigned int n);
extern int device_dio_write(int fd, const void *buf, unsigned int n);

void registerDioDevice(void);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_DIO_H_ */
