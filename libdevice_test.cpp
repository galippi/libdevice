#include <assert.h>
#include <math.h>

#include "libdevice.h"
#include "libdevice_timer.h"
#include "libdevice_adc.h"
#include "libdevice_dio.h"
#include "libdevice_can.h"

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
  static const uint32_t t[] = {300, 1125, 1500, 2222, 2700};
  assert(idx < (sizeof(t)/sizeof(t[0])));
  assert(data->timer == t[idx]);
  idx++;
#endif
}

void ubWriteCb(t_BusWriteCallBack * data)
{
  assert(fabs(device_adc_read_double(data->fdBus)-3.33) < 1e-9);
}

static void CAN_Test(void)
{
  registerCANDevice();
  t_device_fd fd_can1 = device_open("/dev/can/J1939", 0);
  assert(fd_can1 >= 0);
  t_device_fd fd_can2 = device_open("/dev/can/J1939", 0);
  assert(fd_can2 >= 0);
  t_device_fd fd_can3 = device_open("/dev/can/sensor", 0);
  assert(fd_can3 >= 0);
  assert(device_close(fd_can1) == 0);
  assert(device_close(fd_can2) == 0);
  assert(device_close(fd_can3) == 0);
}

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  registerTimerDevice();
  {
    t_DeviceTimerSetTimer timer = {1500, it};
    assert(device_ioctl(systemTimer, e_DeviceTimerSetTimer, (void*)&timer) == 0);
    timer.timer = 300;
    assert(device_ioctl(systemTimer, e_DeviceTimerSetTimerDelta, (void*)&timer) == 0);
    t_DeviceTimerStep timerStep = {1000};
    assert(device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep) == 0);
    timer.timer = 125;
    assert(device_ioctl(systemTimer, e_DeviceTimerSetTimerDelta, (void*)&timer) == 0);
    assert(device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep) == 0);
    assert(device_systimer_setCb(e_DeviceTimerSetTimer, 1700, it) < 0);
    assert(device_systimer_setCb(e_DeviceTimerSetTimer, 2700, it) == 0);
    assert(device_systimer_setCb(e_DeviceTimerSetTimerDelta, 222, it) == 0);
    assert(device_systimer_step(1000) == 0);
  }
  registerAdcDevice();
  registerDioDevice();
  t_device_fd fd1 = device_open("/dev/adc/Ub", 0);
  assert(fd1 >= 0);
  assert(LibDeviceRegisterWriteCallback(fd1, ubWriteCb, NULL) == 0);
  assert(device_adc_write_double(fd1, 3.33) == sizeof(double));
  assert(fabs(device_adc_read_double(fd1)-3.33) < 1e-9);
  t_device_fd fd11 = device_open("/dev/adc/Uz", 0);
  assert(fd11 >= 0);
  assert(device_adc_write_double(fd11, 13.0) == sizeof(double));
  t_device_fd fd2 = device_open("/dev/adc/Ub", 0);
  assert(fd2 >= 0);
  assert(fabs(device_adc_read_double(fd2)-3.33) < 1e-9);
  assert(fabs(device_adc_read_double(fd11)-13.0) < 1e-9);

  t_device_fd fd3 = device_open("/dev/dio/Ub", 0);
  assert(fd3 >= 0);
  assert(device_dio_write_char(fd3, 1) == 1);
  assert(device_dio_read_char(fd3) == 1);
  t_device_fd fd4 = device_open("/dev/dio/Ub", 0);
  assert(fd4 >= 0);
  t_device_fd fd_pcv = device_open("/dev/dio/pcv", 0);
  assert(fd_pcv >= 0);
  assert(device_dio_write_char(fd_pcv, 0) == 1);
  assert(device_dio_read_char(fd4) == 1);
  assert(device_dio_read_char(fd_pcv) == 0);
  assert(device_close(fd1) == 0);
  assert(device_close(fd11) == 0);
  assert(device_close(fd2) == 0);
  assert(device_close(fd3) == 0);
  assert(device_close(fd4) == 0);
  assert(device_close(fd_pcv) == 0);
  assert((fd1 = device_open("/dev/adc/Ub", 0)) >= 0);
  assert(device_close(fd1) == 0);
  CAN_Test();
  return 0;
}
