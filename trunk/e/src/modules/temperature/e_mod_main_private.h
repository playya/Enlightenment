#ifndef E_MOD_MAIN_PRIVATE_H
#define E_MOD_MAIN_PRIVATE_H

typedef enum _Sensor_Type
{
   SENSOR_TYPE_NONE,
   SENSOR_TYPE_FREEBSD,
   SENSOR_TYPE_OMNIBOOK,
   SENSOR_TYPE_LINUX_MACMINI,
   SENSOR_TYPE_LINUX_I2C,
   SENSOR_TYPE_LINUX_ACPI,
   SENSOR_TYPE_LINUX_PCI,
   SENSOR_TYPE_LINUX_PBOOK,
   SENSOR_TYPE_LINUX_INTELCORETEMP,
   SENSOR_TYPE_LINUX_THINKPAD,
   SENSOR_TYPE_LINUX_SYS
} Sensor_Type;

#endif
