#ifndef __POWER_SENSOR
#define __POWER_SENSOR

#define PS_IOC_MAGIC 'P'
#define PSCTL_START	_IO(PS_IOC_MAGIC, 1)
#define PSCTL_STOP	_IOR(PS_IOC_MAGIC, 2, int)
#define PSCTL_READ_PWR  _IOR(PS_IOC_MAGIC, 3, int)
#endif
