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
  t_device_fd fd2 = device_open("/dev/adc/Ub", 0);
  (void)device_read(fd2, &Ub2, sizeof(Ub2));

  t_device_fd fd3 = device_open("/dev/dio/Ub", 0);
  char Ub3 = 1, Ub4;
  (void)device_write(fd3, &Ub3, sizeof(Ub3));
  (void)device_read(fd3, &Ub4, sizeof(Ub4));
  t_device_fd fd4 = device_open("/dev/dio/Ub", 0);
  (void)device_read(fd4, &Ub4, sizeof(Ub4));
  device_close(fd1);
  device_close(fd2);
  device_close(fd3);
  device_close(fd4);
  return 0;
}
