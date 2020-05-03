#include <assert.h>
#include <math.h>
#include <memory.h>

#include "libdevice.h"
#include "libdevice_timer.h"
#include "libdevice_adc.h"
#include "libdevice_dio.h"
#include "libdevice_can.h"
#include "libdevice_vcan.h"
#include "canlogger_libdevice.h"

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
  printf("Executing CAN_Test\n");
  t_network_id net = network_open("base");
  t_device_fd fd_can1c = device_open(net, "/dev/can/J1939", 0);
  assert(fd_can1c >= 0);
  assert(device_close(fd_can1c) == 0);
  t_device_fd fd_can1 = device_open(net, "/dev/can/J1939", 0);
  assert(fd_can1 >= 0);
  assert(fd_can1 == fd_can1c); /* reopening channel shall be the same id */
  t_device_fd fd_can2 = device_open(net, "/dev/can/J1939", 0);
  assert(fd_can2 >= 0);
  t_device_fd fd_can3 = device_open(net, "/dev/can/sensor", 0);
  assert(fd_can3 >= 0);
  t_LibDeviceCAN msg;

  t_LibDeviceCAN msg0 = {0x1234, 8, {0, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can1, &msg0, sizeof(msg)) == sizeof(msg));
  t_LibDeviceCAN  msg1 = {0x5678, 8, {0xaa, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can1, &msg1, sizeof(msg)) == sizeof(msg));
  t_LibDeviceCAN msg2 = {0x8765, 8, {0xBB, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can2, &msg2, sizeof(msg)) == sizeof(msg));
  t_LibDeviceCAN msg3 = {0x1357, 8, {0xcc, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can3, &msg3, sizeof(msg)) == sizeof(msg));

  assert(device_read(fd_can1, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg0, sizeof(msg)) == 0);
  assert(device_read(fd_can1, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg1, sizeof(msg)) == 0);
  assert(device_read(fd_can1, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg2, sizeof(msg)) == 0);
  assert(device_read(fd_can1, &msg, sizeof(msg)) ==  0);

  assert(device_read(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg0, sizeof(msg)) == 0);
  assert(device_read(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg1, sizeof(msg)) == 0);
  assert(device_read(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg2, sizeof(msg)) == 0);
  assert(device_read(fd_can2, &msg, sizeof(msg)) ==  0);

  assert(device_read(fd_can3, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg3, sizeof(msg)) == 0);
  t_LibDeviceCAN  msg4 = {0x5678, 8, {0xee, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can3, &msg4, sizeof(msg)) == sizeof(msg));
  assert(device_read(fd_can3, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg4, sizeof(msg)) == 0);
  assert(device_read(fd_can3, &msg, sizeof(msg)) ==  0);

  assert(device_close(fd_can1) == 0);
  assert(device_close(fd_can2) == 0);
  assert(device_close(fd_can3) == 0);
}

static void vCAN_Test(void)
{
  printf("Executing vCAN_Test\n");
  t_network_id net = network_open("base");
  t_device_fd fd_can1 = device_open(net, "/dev/vcan/J1939", 0);
  assert(fd_can1 >= 0);
  t_device_fd fd_can2 = device_open(net, "/dev/vcan/J1939", 0);
  assert(fd_can2 >= 0);
  t_device_fd fd_can3 = device_open(net, "/dev/vcan/sensor", 0);
  assert(fd_can3 >= 0);
  typedef struct can_frame t_LibDevicevCAN;
  t_LibDevicevCAN msg;

  t_LibDevicevCAN msg0 = {0x1234, 8, {0, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can1, &msg0, sizeof(msg)) == sizeof(msg));
  t_LibDevicevCAN  msg1 = {0x5678, 8, {0xaa, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can1, &msg1, sizeof(msg)) == sizeof(msg));
  t_LibDevicevCAN msg2 = {0x8765, 8, {0xBB, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can2, &msg2, sizeof(msg)) == sizeof(msg));
  t_LibDevicevCAN msg3 = {0x1357, 8, {0xcc, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can3, &msg3, sizeof(msg)) == sizeof(msg));

  assert(device_read(fd_can1, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg2, sizeof(msg)) == 0);
  assert(device_read(fd_can1, &msg, sizeof(msg)) ==  -1);

  assert(device_read(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg0, sizeof(msg)) == 0);
  assert(device_read(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(memcmp(&msg, &msg1, sizeof(msg)) == 0);
  assert(device_read(fd_can2, &msg, sizeof(msg)) ==  -1);

  assert(device_read(fd_can3, &msg, sizeof(msg)) == -1);
  t_LibDevicevCAN  msg4 = {0x5678, 8, {0xee, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can3, &msg4, sizeof(msg)) == sizeof(msg));
  assert(device_read(fd_can3, &msg, sizeof(msg)) ==  -1);

  assert(device_close(fd_can1) == 0);
  assert(device_close(fd_can2) == 0);
  assert(device_close(fd_can3) == 0);
}

static void Timer_Test(void)
{
  printf("Executing Timer_Test\n");
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

static void ADC_DIO_Test()
{
  printf("Executing ADC_DIO_Test\n");
  t_network_id net = network_open("base");
  t_device_fd fd1 = device_open(net, "/dev/adc/Ub", 0);
  assert(fd1 >= 0);
  assert(LibDeviceRegisterWriteCallback(fd1, ubWriteCb, NULL) == 0);
  assert(device_adc_write_double(fd1, 3.33) == sizeof(double));
  assert(fabs(device_adc_read_double(fd1)-3.33) < 1e-9);
  t_device_fd fd11 = device_open(net, "/dev/adc/Uz", 0);
  assert(fd11 >= 0);
  assert(device_adc_write_double(fd11, 13.0) == sizeof(double));
  t_device_fd fd2 = device_open(net, "/dev/adc/Ub", 0);
  assert(fd2 >= 0);
  assert(fabs(device_adc_read_double(fd2)-3.33) < 1e-9);
  assert(fabs(device_adc_read_double(fd11)-13.0) < 1e-9);

  t_device_fd fd3 = device_open(net, "/dev/dio/Ub", 0);
  assert(fd3 >= 0);
  assert(device_dio_write_char(fd3, 1) == 1);
  assert(device_dio_read_char(fd3) == 1);
  t_device_fd fd4 = device_open(net, "/dev/dio/Ub", 0);
  assert(fd4 >= 0);
  t_device_fd fd_pcv = device_open(net, "/dev/dio/pcv", 0);
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
  assert((fd1 = device_open(net, "/dev/adc/Ub", 0)) >= 0);
  assert(device_close(fd1) == 0);
}

static void networkList_test()
{
  printf("Executing networkList_test\n");
  LibDeviceNetworkList list;
  LibDeviceNetwork *net0 = list.get("base");
  assert(net0->getId() == 0);
  LibDeviceNetwork *net1 = list.get("base");
  assert(net0 == net1);
  net1 = list.get("base2");
  assert(net0 != net1);
  assert(net1->getId() == 1);
  LibDeviceNetwork *net2 = list.get("base");
  assert(net0 == net2);
}

static void networkCrossTalk_test()
{
  printf("Executing networkCrossTalk_test\n");
  t_network_id net0 = network_open("ECU0");
  t_network_id net1 = network_open("ECU1");
  t_device_fd fd0 = device_open(net0, "/dev/adc/Ub", 0);
  assert(fd0 >= 0);
  t_device_fd fd1 = device_open(net1, "/dev/adc/Ub", 0);
  assert(fd1 >= 0);
  assert(fd0 != fd1);
  assert(device_adc_write_double(fd0, 3.33) == sizeof(double));
  assert(device_adc_write_double(fd1, 5.55) == sizeof(double));
  assert(fabs(device_adc_read_double(fd0)-3.33) < 1e-9);
  assert(fabs(device_adc_read_double(fd1)-5.55) < 1e-9);
  assert(device_close(fd0) == 0);
  assert(device_close(fd1) == 0);
}

static size_t fileSize(FILE *fin)
{
  fseek(fin, 0, SEEK_END);
  size_t s = ftell(fin);
  fseek(fin, 0, SEEK_SET);
  return s;
}

static uint8_t *fileLoad(FILE *fin, size_t *s)
{
  *s = fileSize(fin);
  uint8_t *mem = (uint8_t *)malloc(*s);
  if (mem != NULL)
    *s = fread(mem, 1, *s, fin);
  return mem;
}

static int fileCompareFile(FILE *file0, FILE *file1)
{
  int result = 0;
  size_t s0, s1;
  uint8_t *mem0 = fileLoad(file0, &s0);
  uint8_t *mem1 = fileLoad(file1, &s1);
  if ((mem0 == NULL) || (mem1 == NULL) || (s0 != s1))
    result = 1;
  else
  {
    result = memcmp(mem0, mem1, s0);
  }
  if (mem0 != 0)
    free(mem0);
  if (mem1 != 0)
    free(mem1);
  return result;
}

static int fileCompare(const char *file0, const char *file1)
{
  int result = 0;
  FILE *f0 = fopen(file0, "rb");
  FILE *f1 = fopen(file1, "rb");
  if ((f0 == NULL) || (f1 == NULL))
    result = 1;
  else
  {
    result = fileCompareFile(f0, f1);
  }
  if (f0 != NULL)
    fclose(f0);
  if (f1 != NULL)
    fclose(f1);
  return result;
}

static void canLogger_test()
{
  printf("Executing canLogger_test\n");
  t_network_id net0 = network_open("base");
  CanLogger logger("canlogger_test.asc");
  const char *busname0 = "/dev/can/J1939";
  logger.addBus(net0, busname0);
  t_device_fd fd_can1 = device_open(net0, busname0, 0);
  assert(fd_can1 >= 0);
  t_device_fd fd_can2 = device_open(net0, busname0, 0);
  assert(fd_can2 >= 0);
  t_LibDeviceCAN msg = {0x1234, 8, {0, 1, 2, 3, 4, 5, 6, 7}};
  assert(device_write(fd_can1, &msg, sizeof(msg)) == sizeof(msg));
  t_DeviceTimerStep timerStep = {200000};
  assert(device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep) == 0);
  msg.id = 0x80123456;
  msg.dlc = 5;
  msg.data[2] = 0x22;
  assert(device_write(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep) == 0);
  const char *busname1 = "/dev/can/sensor";
  logger.addBus(net0, busname1);
  t_device_fd fd_can3 = device_open(net0, busname1, 0);
  assert(fd_can3 >= 0);
  msg.id = 0x11;
  msg.dlc = 7;
  msg.data[1] = 0x11;
  assert(device_write(fd_can3, &msg, sizeof(msg)) == sizeof(msg));
  assert(device_ioctl(systemTimer, e_DeviceTimerStep, (void*)&timerStep) == 0);
  assert(device_write(fd_can2, &msg, sizeof(msg)) == sizeof(msg));
  assert(device_close(fd_can1) == 0);
  assert(device_close(fd_can2) == 0);
  assert(device_close(fd_can3) == 0);
  logger.close();
  assert(fileCompare("canlogger_test.asc", "canlogger_test_data.asc") == 0);
}

int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;
  networkList_test();
  registerTimerDevice("base");
  Timer_Test();
  registerAdcDevice("base");
  registerDioDevice("base");
  ADC_DIO_Test();
  registerCANDevice("base");
  CAN_Test();
  registerVCANDevice("base");
  vCAN_Test();
  networkCrossTalk_test();
  canLogger_test();
  assert(device_close(systemTimer) == 0);
  return 0;
}
