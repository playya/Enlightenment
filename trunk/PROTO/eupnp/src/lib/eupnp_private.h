/* Eupnp - UPnP library
 *
 * Copyright (C) 2009 Andre Dieb Martins <andre.dieb@gmail.com>
 *
 * This file is part of Eupnp.
 *
 * Eupnp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Eupnp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Eupnp.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _EUPNP_PRIVATE_H
#define _EUPNP_PRIVATE_H

#include <Eina.h>
#include <Eupnp.h>

#define EUPNP_SSDP_ADDR             "239.255.255.250"
#define EUPNP_SSDP_PORT             1900
#define EUPNP_SSDP_LOCAL_IFACE      "0.0.0.0"
#define EUPNP_SSDP_MSEARCH_TEMPLATE "M-SEARCH * HTTP/1.1\r\n"     \
                                    "HOST: %s:%d\r\n"             \
                                    "MAN: \"ssdp:discover\"\r\n"  \
                                    "MX: %d\r\n"                  \
                                    "ST: %s\r\n\r\n"              \

#define STR(x) #x
#define XSTR(x) STR(x)

#define CHECK_NULL_RET(x)                                               \
    do {                                                                \
        if (x == NULL) {                                                \
            WARN("%s == NULL!", XSTR(x));                             \
            return;                                                     \
        }                                                               \
    } while (0)

#define CHECK_NULL_RET_VAL(x, val)                                      \
    do {                                                                \
        if (x == NULL) {                                                \
            WARN("%s == NULL!", XSTR(x));                             \
            return val;                                                 \
        }                                                               \
    } while (0)

extern int EUPNP_LOGGING_DOM_GLOBAL;

#define WARN(...) EINA_LOG_DOM_WARN(EUPNP_LOGGING_DOM_GLOBAL, __VA_ARGS__)
#define DEBUG(...) EINA_LOG_DOM_DBG(EUPNP_LOGGING_DOM_GLOBAL, __VA_ARGS__)
#define INFO(...) EINA_LOG_DOM_INFO(EUPNP_LOGGING_DOM_GLOBAL, __VA_ARGS__)
#define ERROR(...) EINA_LOG_DOM_ERR(EUPNP_LOGGING_DOM_GLOBAL, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(EUPNP_LOGGING_DOM_GLOBAL, __VA_ARGS__)


#define WARN_D(DOM, ...) EINA_LOG_DOM_WARN(DOM, __VA_ARGS__)
#define DEBUG_D(DOM, ...) EINA_LOG_DOM_DBG(DOM, __VA_ARGS__)
#define INFO_D(DOM, ...) EINA_LOG_DOM_INFO(DOM, __VA_ARGS__)
#define ERROR_D(DOM, ...) EINA_LOG_DOM_ERR(DOM, __VA_ARGS__)
#define CRIT_D(DOM, ...) EINA_LOG_DOM_CRIT(DOM, __VA_ARGS__)


Eupnp_Service_Info   *eupnp_service_info_new(const char *udn, const char *location, const char *service_type, void *resource, void (*resource_free)(void *resource)) EINA_ARG_NONNULL(1,2,3,4);
void                  eupnp_service_info_free(Eupnp_Service_Info *d) EINA_ARG_NONNULL(1);
void                  eupnp_service_info_dump(const Eupnp_Service_Info *service_info) EINA_ARG_NONNULL(1);

typedef struct _Eupnp_Service_Action Eupnp_Service_Action;
typedef struct _Eupnp_State_Variable_Allowed_Value Eupnp_State_Variable_Allowed_Value;
typedef struct _Eupnp_Device_Icon Eupnp_Device_Icon;

struct _Eupnp_Service_Proxy {
   int spec_version_major;
   int spec_version_minor;

   Eina_Inlist *actions;     // List of actions
   Eina_Inlist *state_table; // List of state variables

   /* Private */
   const char *control_url;
   const char *eventsub_url;
   const char *base_url;
   const char *service_type;
   const char *event_host;
   Eupnp_Server event_server;
   void *xml_parser;
   Eupnp_Service_Proxy_Ready_Cb ready_cb;
   void *ready_cb_data;
   int refcount;
};

struct _Eupnp_Service_Action {
   EINA_INLIST;
   const char *name;
   Eina_Inlist *arguments;
};


typedef enum {
   EUPNP_DATA_TYPE_UI1,
   EUPNP_DATA_TYPE_UI2,
   EUPNP_DATA_TYPE_UI4,
   EUPNP_DATA_TYPE_I1,
   EUPNP_DATA_TYPE_I2,
   EUPNP_DATA_TYPE_I4,
   EUPNP_DATA_TYPE_INT,
   EUPNP_DATA_TYPE_R4,
   EUPNP_DATA_TYPE_R8,
   EUPNP_DATA_TYPE_NUMBER,
   EUPNP_DATA_TYPE_FIXED14_4,
   EUPNP_DATA_TYPE_FLOAT,
   EUPNP_DATA_TYPE_CHAR,
   EUPNP_DATA_TYPE_STRING,
   EUPNP_DATA_TYPE_DATE,
   EUPNP_DATA_TYPE_DATETIME,
   EUPNP_DATA_TYPE_DATETIME_TZ,
   EUPNP_DATA_TYPE_TIME,
   EUPNP_DATA_TYPE_TIME_TZ,
   EUPNP_DATA_TYPE_BOOLEAN,
   EUPNP_DATA_TYPE_BIN_BASE64,
   EUPNP_DATA_TYPE_BIN_HEX,
   EUPNP_DATA_TYPE_URI,
   EUPNP_DATA_TYPE_UUID
} Eupnp_Data_Type;

struct _Eupnp_State_Variable {
   EINA_INLIST;
   Eina_Bool send_events;
   const char *name;
   Eupnp_Data_Type data_type;
   void *default_value;
   Eina_Inlist *allowed_value_list;
   char *range_min;
   char *range_max;
   char *range_step;
};

struct _Eupnp_State_Variable_Allowed_Value {
   EINA_INLIST;
   const char *value;
};

struct _Eupnp_Device_Icon {
   EINA_INLIST;

   int width;
   int height;
   int depth;
   const char *url;
   const char *mimetype;
};

Eupnp_Device_Info *eupnp_device_info_new(const char *udn, const char *location, void *resource, void (*resource_free)(void *resource));
void               eupnp_device_info_free(Eupnp_Device_Info *d);
void               eupnp_device_info_icon_add(Eupnp_Device_Info *device_info, Eupnp_Device_Icon *icon);
void               eupnp_device_info_service_add(Eupnp_Device_Info *device_info, Eupnp_Service_Info *service);
void               eupnp_device_info_device_add(Eupnp_Device_Info *device, Eupnp_Device_Info *new);
void               eupnp_device_info_dump(const Eupnp_Device_Info *device_info);



#endif /* _EUPNP_PRIVATE_H */
