#include "libdevice.h"
#include "libdevice_adc.h"
#include "libdevice_dio.h"

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  registerAdcDevice();
  registerDioDevice();
  t_device_fd fd1 = device_open("/dev/adc/Ub", 0);
  double Ub = 3.33, Ub2;
  (void)device_write(fd1, &Ub, sizeof(Ub));
  (void)device_read(fd1, &Ub2, sizeof(Ub2));
  t_device_fd fd11 = device_open("/dev/adc/Uz", 0);
  double Uz = 13;
  (void)device_write(fd11, &Uz, sizeof(Uz));
  t_device_fd fd2 = device_open("/dev/adc/Ub", 0);
  (void)device_read(fd2, &Ub2, sizeof(Ub2));
  (void)device_read(fd11, &Uz, sizeof(Uz));

  t_device_fd fd3 = device_open("/dev/dio/Ub", 0);
  char Ub3 = 1, Ub4;
  (void)device_write(fd3, &Ub3, sizeof(Ub3));
  (void)device_read(fd3, &Ub4, sizeof(Ub4));
  t_device_fd fd4 = device_open("/dev/dio/Ub", 0);
  t_device_fd fd_pcv = device_open("/dev/dio/pcv", 0);
  char pcv = 0;
  (void)device_write(fd_pcv, &pcv, sizeof(pcv));
  (void)device_read(fd4, &Ub4, sizeof(Ub4));
  (void)device_read(fd_pcv, &pcv, sizeof(pcv));
  device_close(fd1);
  device_close(fd11);
  device_close(fd2);
  device_close(fd3);
  device_close(fd4);
  device_close(fd_pcv);
  return 0;
}
