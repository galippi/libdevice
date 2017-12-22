#include "libdevice.h"
#include "libdevice_adc.h"

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  registerAdcDevice();
  t_device_fd fd = device_open("/dev/adc/Ub", 0);
  double Ub = 3.33, Ub2;
  (void)device_write(fd, &Ub, sizeof(Ub));
  (void)device_read(fd, &Ub2, sizeof(Ub2));
  t_device_fd fd2 = device_open("/dev/adc/Ub", 0);
  (void)device_read(fd2, &Ub2, sizeof(Ub2));
  device_close(fd);
  device_close(fd2);
  return 0;
}
