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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <Eina.h>

#include "Eupnp.h"
#include "eupnp_ssdp.h"
#include "eupnp_core.h"
#include "eupnp_log.h"
#include "eupnp_udp_transport.h"
#include "eupnp_device_info.h"
#include "eupnp_service_info.h"
#include "eupnp_http_message.h"
#include "eupnp_event_bus.h"
#include "eupnp_private.h"


/**
 * @addtogroup Eupnp_SSDP_Client_Module SSDP Client
 *
 * This module implements the client-side of the SSDP specification. It is
 * capable of searching for services through the local network.
 *
 * @section eupnp_ssdp_initialize Initializing the SSDP module
 *
 * For using the SSDP module, it must be initialized by calling
 * eupnp_ssdp_init(). This call will allocate the required resources and setup
 * it for use.
 *
 * When finished using the SSDP module, eupnp_ssdp_shutdown() must be called so
 * that resources allocated are freed properly.
 *
 *
 * @section eupnp_ssdp_instance Creating a SSDP Client
 *
 * For creating and destroying an Eupnp_SSDP_Client, use the
 * eupnp_ssdp_client_new() and eupnp_ssdp_client_free() functions,
 * respectivelly.
 *
 * The SSDP client starts listening for announcements once
 * eupnp_ssdp_client_start() is called. For stopping listening, call
 * eupnp_ssdp_client_stop().
 *
 * @section eupnp_ssdp_events Monitoring service announcements
 *
 * The utility of the SSDP client is to receive service annoucements. This is
 * done through the Event_Bus module.
 *
 * For receiving announcements, one needs to subscribe on the bus for
 * EUPNP_EVENT_SERVICE_FOUND and EUPNP_EVENT_SERVICE_GONE events
 * (see eupnp_event_bus_subscribe() and eupnp_event_bus.h). Whenever a service announces it's presence
 * or it's absence, these events are triggered and forwarded to your subscribed
 * callback.
 *
 * @code
 * #include <Eupnp.h>
 *
 * static Eina_Bool
 * on_service_found(void *user_data, Eupnp_Event_Type event_type, void *event_data)
 * {
 *     // Service found
 * }
 *
 * static Eina_Bool
 * on_service_gone(void *user_data, Eupnp_Event_Type event_type, void *event_data)
 * {
 *     // Service is gone
 * }
 *
 * int
 * main(int argc, char *argv[])
 * {
 *     eupnp_event_bus_init();
 *     Eupnp_SSDP_Client *client = eupnp_ssdp_client_new();
 *
 *     eupnp_event_bus_subscribe(EUPNP_EVENT_SERVICE_FOUND, EUPNP_CALLBACK(on_service_found), NULL);
 *     eupnp_event_bus_subscribe(EUPNP_EVENT_SERVICE_GONE, EUPNP_CALLBACK(on_service_gone), NULL);
 *
 *     eupnp_ssdp_client_start(client);
 *
 *     // Application main procedure
 *
 *     main_loop();
 *
 *     // Shutdown procedure
 *     eupnp_ssdp_client_stop(client);
 *     eupnp_event_bus_shutdown();
 *     return 0;
 * }
 *
 * @endcode
 *
 */

/*
 * Private API
 */

static int _eupnp_ssdp_main_count = 0;
static int _log_dom = -1;


/**
 * Parses the USN and finds out if it is a device or service announcement.
 * USN can be in the following forms:
 *    (1) uuid:device-UUID::upnp:rootdevice
 *    (2) uuid:device-UUID
 *    (3) uuid:device-UUID::urn:schemas-upnp-org:device:deviceType:v
 *    (4) uuid:device-UUID::urn:schemas-upnp-org:service:serviceType:v
 *    (5) uuid:device-UUID::urn:domain-name:device:deviceType:v
 *    (6) uuid:device-UUID::urn:domain-name:service:serviceType:v
 *
 * Sets the udn, udn_len and service_type given pointers to the values parsed.
 *
 * @return EINA_TRUE if parsed successfully. EINA_FALSE otherwise.
 */
static Eina_Bool
parse_usn(const char *usn, const char **udn, int *udn_len, const char **service_type)
{
   const char *tmp;
   const char *tmp1;
   const char *tmp2;

   if (strncmp(usn, "uuid:", 5))
     {
	ERROR_D(_log_dom, "Failed to parse USN: missing \"uuid:\" (original: %s)", usn);
	return EINA_FALSE;
     }

   tmp = usn + 5;

   // Check if there are more items separated by '::'
   tmp1 = strchr(tmp, ':');

   if (!tmp1)
     {
	// Type (2) - Did not found ':', means there's only the device-UUID part
	*udn = usn;
	*udn_len = strlen(usn);
	return EINA_TRUE;
     }
   else
     {
	if (*(tmp1 + 1) != ':')
	  {
	     ERROR_D(_log_dom, "Invalid USN: single colon after device uuid: %s", usn);
	     return EINA_FALSE;
	  }
	*udn = usn;
	*udn_len = tmp1 - usn;

	// Skip two colons
	tmp1 += 2;

	if (!strncmp(tmp1, "upnp:rootdevice", 15))
	  {
	     // Type (1)
	     return EINA_TRUE;
	  }

	if (strncmp(tmp1, "urn:", 4))
	  {
	     ERROR_D(_log_dom, "Invalid USN: type string does not contain urn.");
	     return EINA_FALSE;
	  }

	// Skip "urn:" and domain
	tmp2 = tmp1 + 4;
	tmp2 = strchr(tmp2, ':');

	if (!tmp2)
	  {
	     ERROR_D(_log_dom, "Incomplete USN: %s", usn);
	     return EINA_FALSE;
	  }

	// Skip colon after domain
	tmp2 += 1;

	if (!strncmp(tmp2, "service", 7))
	  {
	     // Type (4) or (6)
	     *service_type = tmp1;
	     return EINA_TRUE;
	  }
	
	if (!*udn)
	  {
	     ERROR_D(_log_dom, "Failed to parse USN, type not recognized");
	     return EINA_FALSE;
	  }

	return EINA_TRUE;
     }
}

/**
 * Processes a HTTP response message.
 *
 * Publishes EUPNP_EVENT_DEVICE_FOUND, EUPNP_EVENT_SERVICE_FOUND,
 * EUPNP_EVENT_DEVICE_GONE and EUPNP_EVENT_SERVICE_GONE events. These events
 * carry only basic information that came along with the announcement, which
 * makes this module be independent from the higher UPnP layers (such as
 * control point).
 */
static void
process_response(Eupnp_HTTP_Response *r)
{
   // TODO handle cache control, response date
   // TODO create a filter based on the st
   Eupnp_Device_Info *device_info;
   Eupnp_Service_Info *service_info;
   const char *location;
   const char *usn;
   const char *udn = NULL;
   const char *service_type = NULL;
   int udn_len;

   location = eupnp_http_response_header_get(r, "location");
   usn = eupnp_http_response_header_get(r, "usn");

   if (!location)
     {
	ERROR_D(_log_dom, "Invalid response: missing location");
	goto on_resp_error;
     }

   if (!usn)
     {
	ERROR_D(_log_dom, "Invalid response: missing usn");
	goto on_resp_error;
     }

   if (!parse_usn(usn, &udn, &udn_len, &service_type))
     {
	ERROR_D(_log_dom, "Failed to parse resposne USN: %s", usn);
	goto on_resp_error;
     }

   if (!eupnp_http_response_header_add(r, "udn", 3, udn, udn_len))
     {
	ERROR_D(_log_dom, "Failed to add udn header.");
	goto on_resp_error;
     }

   if (service_type && udn)
     {
	service_info = eupnp_service_info_new(udn, location, service_type, r,
					      (void *)eupnp_http_response_free);

	if (!service_info)
	  {
	     ERROR_D(_log_dom, "Could not create service event.");
	     return;
	  }

	eupnp_event_bus_publish(EUPNP_EVENT_SERVICE_FOUND, service_info);

	// Unreference local, frees it if no reference was made
	eupnp_service_info_unref(service_info);
	return;
     }
   else if (udn)
     {
	// Device related event
	device_info = eupnp_device_info_new(udn, location, r,
					    (void *)eupnp_http_response_free);

	if (!device_info)
	  {
	     ERROR_D(_log_dom, "Could not create device event.");
	     return;
	  }

	eupnp_event_bus_publish(EUPNP_EVENT_DEVICE_FOUND, device_info);

	// Unreference local, frees it if no reference was made.
	eupnp_device_info_unref(device_info);
	return;
     }

   DEBUG_D(_log_dom, "SSDP response not identified");
   eupnp_http_response_dump(r);

   on_resp_error:
      eupnp_http_response_free(r);
}

/**
 * Processes a NOTIFY request message.
 */
static void
process_notify_request(Eupnp_HTTP_Request *m)
{
   // TODO provide a option to ignore notify requests
   Eupnp_Device_Info *device_info;
   Eupnp_Service_Info *service_info;
   const char *nts;
   const char *location;
   const char *usn;
   const char *udn = NULL;
   const char *service_type = NULL;
   int type;
   int udn_len;

   nts = eupnp_http_request_header_get(m, "nts");
   location = eupnp_http_request_header_get(m, "location");
   usn = eupnp_http_request_header_get(m, "usn");

   if (!nts)
     {
	ERROR_D(_log_dom, "Invalid NOTIFY: nts header not found");
	goto on_req_error;
     }

   if (!usn)
     {
	ERROR_D(_log_dom, "Invalid NOTIFY: missing usn");
	eupnp_http_request_dump(m);
	goto on_req_error;
     }

   if (!parse_usn(usn, &udn, &udn_len, &service_type))
     {
	DEBUG_D(_log_dom, "Failed to parse usn %s", usn);
	eupnp_http_request_dump(m);
	goto on_req_error;
     }

   // Add the udn as a header so that it gets collected automatically
   if (!eupnp_http_request_header_add(m, "udn", 3, udn, udn_len))
     {
	DEBUG_D(_log_dom, "Failed to add udn header.");
	goto on_req_error;
     }

   // Guess event type
   if (!strcmp(nts, EUPNP_SSDP_NOTIFY_ALIVE))
     {
	if (!location)
	  {
	     ERROR_D(_log_dom, "Invalid NOTIFY: alive and location missing");
	     goto on_req_error;
	  }
	if (service_type)
	  type = EUPNP_EVENT_SERVICE_FOUND;
	else
	  type = EUPNP_EVENT_DEVICE_FOUND;
     }
   else if (!strcmp(nts, EUPNP_SSDP_NOTIFY_BYEBYE))
     {
	if (service_type)
	  type = EUPNP_EVENT_SERVICE_GONE;
	else
	  type = EUPNP_EVENT_DEVICE_GONE;
     }
   else
     {
	ERROR_D(_log_dom, "Invalid NOTIFY: not alive, nor byebye");
	goto on_req_error;
     }

   if (service_type)
     {
	udn = eupnp_http_request_header_get(m, "udn");
	service_info = eupnp_service_info_new(udn, location, service_type, m,
					      (void *)eupnp_http_request_free);

	if (!service_info)
	  {
	     ERROR_D(_log_dom, "Could not create service event.");
	     return;
	  }

	eupnp_event_bus_publish(type, service_info);

	// Unreference local, frees it if no reference was made
	eupnp_service_info_unref(service_info);
	return;
     }
   else if (udn)
     {
	// Device related event
	udn = eupnp_http_request_header_get(m, "udn");
	device_info = eupnp_device_info_new(udn, location, m,
					    (void *)eupnp_http_request_free);

	if (!device_info)
	  {
	     ERROR_D(_log_dom, "Could not create device event.");
	     return;
	  }

	eupnp_event_bus_publish(type, device_info);

	// Unreference local, frees it if no reference was made.
	eupnp_device_info_unref(device_info);
	return;
     }

   DEBUG_D(_log_dom, "SSDP request not identified");
   eupnp_http_request_dump(m);

   on_req_error:
      eupnp_http_request_free(m);
}

/**
 * Called when a datagram is ready to be read from the socket. Parses it and
 * takes the appropriate actions, considering the method of the request.
 *
 * TODO auto-register me on the event loop.
 */
static void
_eupnp_ssdp_on_datagram_available(Eupnp_SSDP_Client *ssdp)
{
   Eupnp_UDP_Datagram *d;

   d = eupnp_udp_transport_recvfrom(ssdp->udp_transport);

   if (!d)
     {
	ERROR_D(_log_dom, "Could not retrieve a valid datagram");
	return;
     }

   if (eupnp_http_message_is_response(d->data))
     {
	DEBUG_D(_log_dom, "Response message from %s:%d", d->host, d->port);

	Eupnp_HTTP_Response *r;
	r = eupnp_http_response_parse(d->data, d, (void *)eupnp_udp_transport_datagram_free);

	if (!r)
	  {
	     ERROR_D(_log_dom, "Failed parsing response datagram");
	     eupnp_udp_transport_datagram_free(d);
	     return;
	  }

	process_response(r);
     }
   else
     {
	Eupnp_HTTP_Request *m;
	m = eupnp_http_request_parse(d->data, d, (void *)eupnp_udp_transport_datagram_free);

	if (!m)
	  {
	     ERROR_D(_log_dom, "Failed parsing request datagram sent by %s:%d", d->host, d->port);
	     eupnp_udp_transport_datagram_free(d);
	     return;
	  }

	if (m->method == _eupnp_ssdp_notify)
	  {
	     DEBUG_D(_log_dom, "NOTIFY message from %s:%d", d->host, d->port);
	     process_notify_request(m);
	  }
	  else
	     eupnp_http_request_free(m);
     }
}

static Eina_Bool
_eupnp_ssdp_forward_sock_event(void *data, Eupnp_Fd_Handler fd_handler)
{
   _eupnp_ssdp_on_datagram_available(data);
   return EINA_TRUE;
}

/*
 * Public API
 */

/**
 * Initializes the ssdp module.
 *
 * @return On error, returns 0. Otherwise, returns the number of times it's been
 * called.
 */
EAPI int
eupnp_ssdp_init(void)
{
if (_eupnp_ssdp_main_count) return ++_eupnp_ssdp_main_count;

   if (!eina_init())
     {
	fprintf(stderr, "Failed to initialize eina module.\n");
	return 0;
     }

   if (!eupnp_log_init())
     {
	fprintf(stderr, "Failed to initialize eupnp log module.\n");
	goto log_init_err;
     }

   if ((_log_dom = eina_log_domain_register("Eupnp.SSDP", EINA_COLOR_BLUE)) < 0)
     {
	ERROR("Failed to create logging domain for ssdp module.");
	goto log_dom_error;
     }

   if (!eupnp_event_bus_init())
     {
	ERROR("Failed to initialize eupnp event bus module");
	goto event_bus_init_error;
     }

   if (!eupnp_device_info_init())
     {
	ERROR("Failed to initialize eupnp device info module");
	goto device_info_init_error;
     }

   if (!eupnp_service_info_init())
     {
	ERROR("Failed to initialize eupnp service info module");
	goto service_info_init_error;
     }

   _eupnp_ssdp_notify = (char *) eina_stringshare_add("NOTIFY");
   _eupnp_ssdp_msearch = (char *) eina_stringshare_add("M-SEARCH");
   _eupnp_ssdp_http_version = (char *) eina_stringshare_add(EUPNP_HTTP_VERSION);

   INFO_D(_log_dom, "Initializing ssdp module.");

   return ++_eupnp_ssdp_main_count;

   /* Init error handling */
   service_info_init_error:
      eupnp_device_info_shutdown();
   device_info_init_error:
      eupnp_event_bus_shutdown();
   event_bus_init_error:
      eina_log_domain_unregister(_log_dom);
   log_dom_error:
      eupnp_log_shutdown();
   log_init_err:
      eina_shutdown();

   return 0;
}

/**
 * Shuts down the ssdp module.
 *
 * @return 0 if completely shutted down the module.
 */
EAPI int
eupnp_ssdp_shutdown(void)
{
   if (_eupnp_ssdp_main_count != 1) return --_eupnp_ssdp_main_count;

   INFO_D(_log_dom, "Shutting down ssdp module.");

   eupnp_service_info_shutdown();
   eupnp_device_info_shutdown();
   eupnp_event_bus_shutdown();
   eina_log_domain_unregister(_log_dom);
   eupnp_log_shutdown();
   eina_shutdown();

   return --_eupnp_ssdp_main_count;
}

/**
 * Constructor for the Eupnp_SSDP_Client class.
 *
 * @return On success, a new Eupnp_SSDP_Client instance. Otherwise, NULL.
 */
EAPI Eupnp_SSDP_Client *
eupnp_ssdp_client_new(void)
{
   Eupnp_SSDP_Client *ssdp;

   ssdp = malloc(sizeof(Eupnp_SSDP_Client));

   if (!ssdp)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	ERROR_D(_log_dom, "Could not create SSDP client instance.");
	return NULL;
     }

   ssdp->udp_transport = eupnp_udp_transport_new(EUPNP_SSDP_ADDR,
						 EUPNP_SSDP_PORT,
						 EUPNP_SSDP_LOCAL_IFACE);

   if (!ssdp->udp_transport)
     {
	ERROR_D(_log_dom, "Could not create SSDP client instance.");
	free(ssdp);
	return NULL;
     }

   ssdp->socket_handler = NULL;

   return ssdp;
}

/**
 * Starts a SSDP client.
 *
 * Makes the SSDP client start listening for services announcements.
 *
 * @param c SSDP client to start
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool
eupnp_ssdp_client_start(Eupnp_SSDP_Client *c)
{
   CHECK_NULL_RET_VAL(c, EINA_FALSE);

   if (c->socket_handler)
     {
	WARN_D(_log_dom, "Tried to start SSDP client twice.");
	return EINA_FALSE;
     }

   c->socket_handler = eupnp_core_fd_handler_add(c->udp_transport->socket, EUPNP_FD_EVENT_READ, _eupnp_ssdp_forward_sock_event, c);

   return EINA_TRUE;
}

/**
 * Stops a SSDP client.
 *
 * Stops a SSDP client from listening for services announcements.
 */
EAPI Eina_Bool
eupnp_ssdp_client_stop(Eupnp_SSDP_Client *c)
{
   CHECK_NULL_RET_VAL(c, EINA_FALSE);

   if (!c->socket_handler)
     {
	WARN_D(_log_dom, "Tried to stop SSDP client when already stopped.");
	return EINA_FALSE;
     }

   if (eupnp_core_fd_handler_del(c->socket_handler))
     {
	DEBUG_D(_log_dom, "Removed SSDP socket handler successfully.");
	return EINA_TRUE;
     }

   ERROR_D(_log_dom, "Failed to remove SSDP socket handler.");
   return EINA_FALSE;
}

/**
 * Destructor for the Eupnp_SSDP_Client class.
 *
 * @param c Eupnp_SSDP_Client instance to be destroyed
 */
EAPI void
eupnp_ssdp_client_free(Eupnp_SSDP_Client *c)
{
   CHECK_NULL_RET(c);
   eupnp_udp_transport_free(c->udp_transport);
   free(c);
}

/**
 * Sends a search message for devices to the network (a.k.a. M-Search)
 *
 * @param c Eupnp_SSDP_Client instance.
 * @param mx maximum wait time in seconds for devices to wait before answering
 *        the search message.
 * @param search_target target for the search. Common values are "ssdp:all",
 *        "upnp:rootdevice", and so on (refer to the UPnP device architecture
 *        document for more).
 * @return On success EINA_TRUE, EINA_FALSE on error.
 */
EAPI Eina_Bool
eupnp_ssdp_discovery_request_send(Eupnp_SSDP_Client *c, int mx, const char *search_target)
{
   char *msearch;

   if (asprintf(&msearch, EUPNP_SSDP_MSEARCH_TEMPLATE,
                EUPNP_SSDP_ADDR, EUPNP_SSDP_PORT, mx, search_target) < 0)
     {
	ERROR_D(_log_dom, "Could not allocate buffer for search message.");
	return EINA_FALSE;
     }

    /* Use UDP socket from SSDP */
   if (eupnp_udp_transport_sendto(c->udp_transport, msearch, EUPNP_SSDP_ADDR,
                               EUPNP_SSDP_PORT) < 0)
     {
	ERROR_D(_log_dom, "Could not send search message.");
	return EINA_FALSE;
     }

   free(msearch);
   return EINA_TRUE;
}

