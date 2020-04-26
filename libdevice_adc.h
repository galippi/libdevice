#ifndef _LIBDEVICE_ADC_H_
#define _LIBDEVICE_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int device_adc_open(const char *name, int flags);
extern int device_adc_close(int fd);
extern int device_adc_ioctl(int fd, unsigned long int request, void *data);
extern int device_adc_read(int fd, void *buf, unsigned int n);
extern int device_adc_write(int fd, const void *buf, unsigned int n);

extern int device_adc_write_double(int fd, double val);
extern double device_adc_read_double(int fd);

void registerAdcDevice(const char *networkName);

#ifdef __cplusplus
}
#endif

#endif /* _LIBDEVICE_ADC_H_ */
