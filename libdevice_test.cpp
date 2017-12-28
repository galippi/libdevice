#include <assert.h>
#include <math.h>

#include "libdevice.h"
#include "libdevice_timer.h"
#include "libdevice_adc.h"
#include "libdevice_dio.h"

void it(t_TimerCallBack *data)
{
#if 0
  printf("Timercb=%u\n", data->timer);
#else
  static unsigned idx = 0;
  if (data == NULL)
  {
    idx = 0;
    return;
  }
  static const uint32_t t[] = {300, 1125, 1500};
  assert(idx < (sizeof(t)/sizeof(t[0])));
  assert(data->timer == t[idx]);
  idx++;
#endif
}

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  registerTimerDevice();
  {
    t_DeviceTimerSetTimer timer = {1500, it};
    device_ioctl(systemTimer, e_DeviceTimerSetTimer, (void*)&timer);
    timer.timer = 300;
    device_ioctl(systemTimer, e_DeviceTimerSetTimerDelta, (void*)&timer);
    t_DeviceTimerStep timerStep = {1000};
    device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep);
    timer.timer = 125;
    device_ioctl(systemTimer, e_DeviceTimerSetTimerDelta, (void*)&timer);
    device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep);
  }
  registerAdcDevice();
  registerDioDevice();
  t_device_fd fd1 = device_open("/dev/adc/Ub", 0);
  (void)device_adc_write_double(fd1, 3.33);
  assert(fabs(device_adc_read_double(fd1)-3.33) < 1e-9);
  t_device_fd fd11 = device_open("/dev/adc/Uz", 0);
  (void)device_adc_write_double(fd11, 13.0);
  t_device_fd fd2 = device_open("/dev/adc/Ub", 0);
  assert(fabs(device_adc_read_double(fd2)-3.33) < 1e-9);
  assert(fabs(device_adc_read_double(fd11)-13.0) < 1e-9);

  t_device_fd fd3 = device_open("/dev/dio/Ub", 0);
  (void)device_dio_write_char(fd3, 1);
  assert(device_dio_read_char(fd3) == 1);
  t_device_fd fd4 = device_open("/dev/dio/Ub", 0);
  t_device_fd fd_pcv = device_open("/dev/dio/pcv", 0);
  (void)device_dio_write_char(fd_pcv, 0);
  assert(device_dio_read_char(fd4) == 1);
  assert(device_dio_read_char(fd_pcv) == 0);
  device_close(fd1);
  device_close(fd11);
  device_close(fd2);
  device_close(fd3);
  device_close(fd4);
  device_close(fd_pcv);
  return 0;
}
