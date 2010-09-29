#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_GNUTLS
# include <gnutls/gnutls.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#elif USE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/dh.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include "Ecore.h"
#include "ecore_con_private.h"
#include <sys/mman.h>
#include <errno.h>

static int _init_con_ssl_init_count = 0;

#if USE_GNUTLS
# ifdef EFL_HAVE_PTHREAD
#include <pthread.h>
GCRY_THREAD_OPTION_PTHREAD_IMPL;
# endif

static int _client_connected = 0;
# define SSL_SUFFIX(ssl_func) ssl_func ## _gnutls
# define _ECORE_CON_SSL_AVAILABLE 1

#elif USE_OPENSSL

# define SSL_SUFFIX(ssl_func) ssl_func ## _openssl
# define _ECORE_CON_SSL_AVAILABLE 2

#else
# define SSL_SUFFIX(ssl_func) ssl_func ## _none
# define _ECORE_CON_SSL_AVAILABLE 0

#endif

#if USE_GNUTLS
static void
_gnutls_print_errors(int ret)
{
  if (ret)
    ERR("gnutls returned with error: %s - %s", gnutls_strerror_name(ret), gnutls_strerror(ret));
}


static const char*
SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_description_t status)
{
   switch (status)
     {
      case GNUTLS_HANDSHAKE_HELLO_REQUEST:
        return "Hello request";
      case GNUTLS_HANDSHAKE_CLIENT_HELLO:
        return "Client hello";
      case GNUTLS_HANDSHAKE_SERVER_HELLO:
        return "Server hello";
#ifdef USE_GNUTLS2_10
      case GNUTLS_HANDSHAKE_NEW_SESSION_TICKET:
        return "New session ticket";
#endif
      case GNUTLS_HANDSHAKE_CERTIFICATE_PKT:
        return "Certificate packet";
      case GNUTLS_HANDSHAKE_SERVER_KEY_EXCHANGE:
        return "Server key exchange";
      case GNUTLS_HANDSHAKE_CERTIFICATE_REQUEST:
        return "Certificate request";
      case GNUTLS_HANDSHAKE_SERVER_HELLO_DONE:
        return "Server hello done";
      case GNUTLS_HANDSHAKE_CERTIFICATE_VERIFY:
        return "Certificate verify";
      case GNUTLS_HANDSHAKE_CLIENT_KEY_EXCHANGE:
        return "Client key exchange";
      case GNUTLS_HANDSHAKE_FINISHED:
        return "Finished";
      case GNUTLS_HANDSHAKE_SUPPLEMENTAL:
        return "Supplemental";
     }
   return NULL;
}

#elif USE_OPENSSL

static void
_openssl_print_errors(void)
{
   do
     {
        unsigned long err;

        err = ERR_get_error();
        if (!err) break;
        ERR("openssl error: %s", ERR_reason_error_string(err));
     } while (1);
}
#endif

#define SSL_ERROR_CHECK_GOTO_ERROR(X) \
do \
  { \
     if ((X)) \
       { \
          ERR("Error at %s:%s:%d!", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
          goto error; \
       } \
  } \
while (0)


static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_init) (void);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_shutdown) (void);

static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_server_cafile_add) (Ecore_Con_Server *svr, const char *ca_file);
static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_server_crl_add) (Ecore_Con_Server *svr, const char *crl_file);
static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_server_cert_add) (Ecore_Con_Server *svr, const char *cert);
static Eina_Bool SSL_SUFFIX(_ecore_con_ssl_server_privkey_add) (Ecore_Con_Server *svr, const char *key_file);

static Ecore_Con_Ssl_Error  SSL_SUFFIX(_ecore_con_ssl_server_prepare)(Ecore_Con_Server *svr, int ssl_type);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_server_init) (Ecore_Con_Server * svr);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (Ecore_Con_Server *
                                            svr);
static Ecore_Con_State
                 SSL_SUFFIX(_ecore_con_ssl_server_try) (Ecore_Con_Server * svr);
static int
                 SSL_SUFFIX(_ecore_con_ssl_server_read) (Ecore_Con_Server * svr,
                                        unsigned char *buf, int size);
static int
                 SSL_SUFFIX(_ecore_con_ssl_server_write) (Ecore_Con_Server *
                                         svr,
                                         unsigned char *buf, int size);

static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_client_init) (Ecore_Con_Client * cl);
static Ecore_Con_Ssl_Error
                 SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (Ecore_Con_Client *
                                            cl);
static int
                 SSL_SUFFIX(_ecore_con_ssl_client_read) (Ecore_Con_Client * cl,
                                        unsigned char *buf, int size);
static int
                 SSL_SUFFIX(_ecore_con_ssl_client_write) (Ecore_Con_Client * cl,
                                         unsigned char *buf, int size);

/*
 * General SSL API
 */

Ecore_Con_Ssl_Error
ecore_con_ssl_init(void)
{
   if (!_init_con_ssl_init_count++)
        SSL_SUFFIX(_ecore_con_ssl_init) ();

   return _init_con_ssl_init_count;
}

Ecore_Con_Ssl_Error
ecore_con_ssl_shutdown(void)
{
   if (!--_init_con_ssl_init_count)
     SSL_SUFFIX(_ecore_con_ssl_shutdown) ();

   return _init_con_ssl_init_count;
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_prepare(Ecore_Con_Server *svr, int ssl_type)
{
   if (!ssl_type)
     return ECORE_CON_SSL_ERROR_NONE;
   return SSL_SUFFIX(_ecore_con_ssl_server_prepare) (svr, ssl_type);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_init(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_init) (svr);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_server_shutdown(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_shutdown) (svr);
}

Ecore_Con_State
ecore_con_ssl_server_try(Ecore_Con_Server *svr)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_try) (svr);
}

int
ecore_con_ssl_server_read(Ecore_Con_Server *svr, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_read) (svr, buf, size);
}

int
ecore_con_ssl_server_write(Ecore_Con_Server *svr, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_server_write) (svr, buf, size);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_init(Ecore_Con_Client *cl)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_init) (cl);
}

Ecore_Con_Ssl_Error
ecore_con_ssl_client_shutdown(Ecore_Con_Client *cl)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_shutdown) (cl);
}

int
ecore_con_ssl_client_read(Ecore_Con_Client *cl, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_read) (cl, buf, size);
}

int
ecore_con_ssl_client_write(Ecore_Con_Client *cl, unsigned char *buf, int size)
{
   return SSL_SUFFIX(_ecore_con_ssl_client_write) (cl, buf, size);
}

/**
 * @addtogroup Ecore_Con_SSL_Group Ecore Connection SSL Functions
 *
 * Functions that operate on Ecore connection objects pertaining to SSL.
 *
 * @{
 */

/**
 * Returns if SSL support is available
 * @return 1 if SSL is available and provided by gnutls, 2 if provided by openssl,
 * 0 if it is not available.
 * @ingroup Ecore_Con_Client_Group
 */
EAPI int
ecore_con_ssl_available_get(void)
{
   return _ECORE_CON_SSL_AVAILABLE;
}

/**
 * @brief Enable certificate verification on a server object
 *
 * Call this function on a server object before main loop has started
 * to enable verification of certificates against loaded certificates.
 * @param svr The server object
 */
EAPI void
ecore_con_ssl_server_verify(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_ssl_server_verify");
        return;
     }
   svr->verify = EINA_TRUE;
}
 
/**
 * @brief Add an ssl certificate for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM certificate.
 * Simply specify the cert here to use it in the server object for connecting or listening.
 * If there is an error loading the certificate, an error will automatically be logged.
 * @param cert The path to the certificate.
 * @return EINA_FALSE if the file cannot be loaded, otherwise EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_cert_add(Ecore_Con_Server *svr,
                              const char *cert)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_ssl_server_cert_add");
        return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_cert_add)(svr, cert);
}

/**
 * @brief Add an ssl CA file for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM CA file.
 * Simply specify the file here to use it in the server object for connecting or listening.
 * If there is an error loading the CAs, an error will automatically be logged.
 * @param ca_file The path to the CA file.
 * @return EINA_FALSE if the file cannot be loaded, otherwise EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_cafile_add(Ecore_Con_Server *svr,
                              const char *ca_file)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_ssl_server_cafile_add");
        return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_cafile_add)(svr, ca_file);
}

/**
 * @brief Add an ssl private key for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM private key
 * Simply specify the key file here to use it in the server object for connecting or listening.
 * If there is an error loading the key, an error will automatically be logged.
 * @param key_file The path to the key file.
 * @return EINA_FALSE if the file cannot be loaded,
 * otherwise EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_privkey_add(Ecore_Con_Server *svr,
                              const char *key_file)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_ssl_server_privkey_add");
        return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_privkey_add)(svr, key_file);
}

/**
 * @brief Add an ssl CRL for use in ecore_con functions.
 *
 * Use this function to add a SSL PEM CRL file
 * Simply specify the CRL file here to use it in the server object for connecting or listening.
 * If there is an error loading the CRL, an error will automatically be logged.
 * @param crl_file The path to the CRL file.
 * @return EINA_FALSE if the file cannot be loaded,
 * otherwise EINA_TRUE.
 */

EAPI Eina_Bool
ecore_con_ssl_server_crl_add(Ecore_Con_Server *svr,
                              const char *crl_file)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_ssl_server_crl_add");
        return EINA_FALSE;
     }

   return SSL_SUFFIX(_ecore_con_ssl_server_crl_add)(svr, crl_file);
}

/** }@ */

#if USE_GNUTLS

/*
 * GnuTLS
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_gnutls(void)
{
#ifdef EFL_HAVE_PTHREAD
   if (gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
      WRN(
         "YOU ARE USING PTHREADS, BUT I CANNOT INITIALIZE THREADSAFE GCRYPT OPERATIONS!");

#endif
   if (gnutls_global_init())
      return ECORE_CON_SSL_ERROR_INIT_FAILED;

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_gnutls(void)
{
   gnutls_global_deinit();

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_gnutls(Ecore_Con_Server *svr, int ssl_type __UNUSED__)
{
   int ret;

   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_allocate_credentials(&svr->cert));

   if (svr->created)
     {
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_init(&svr->dh_params));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_dh_params_generate2(svr->dh_params, 1024));

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_server_credentials(&svr->anoncred_s));
        /* TODO: implement PSK */
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_server_credentials(&svr->pskcred_s));

        gnutls_anon_set_server_dh_params(svr->anoncred_s, svr->dh_params);
        gnutls_certificate_set_dh_params(svr->cert, svr->dh_params);
        gnutls_psk_set_server_dh_params(svr->pskcred_s, svr->dh_params);
     }
   else
     {
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_psk_allocate_client_credentials(&svr->pskcred_c));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_anon_allocate_client_credentials(&svr->anoncred_c));
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(ret);
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_gnutls(Ecore_Con_Server *svr)
{
   const gnutls_datum_t *cert_list;
   unsigned int iter, cert_list_size;
   gnutls_x509_crt_t cert = NULL;
#ifdef USE_GNUTLS2_10
   const char *priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:+VERS-TLS1.2:+VERS-TLS1.1:+VERS-TLS1.0:+VERS-SSL3.0";
#else
   const char *priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:+VERS-TLS1.1:+VERS-TLS1.0:+VERS-SSL3.0";
#endif
   int ret = 0;

   switch (svr->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;
      case ECORE_CON_SSL_STATE_INIT:
        if (svr->type & ECORE_CON_USE_SSL2) /* not supported because of security issues */
          return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

        switch (svr->type & ECORE_CON_SSL)
          {
           case ECORE_CON_USE_SSL3:
           case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
              priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:!VERS-TLS1.0:!VERS-TLS1.1";
              break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
              priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:!VERS-SSL3.0";
              break;

           case ECORE_CON_USE_MIXED:
           case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
              break;

           default:
              return ECORE_CON_SSL_ERROR_NONE;
          }

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&svr->session, GNUTLS_CLIENT));
#ifdef USE_GNUTLS2_10
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_client(svr->session));
#endif
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_server_name_set(svr->session, GNUTLS_NAME_DNS, svr->name, strlen(svr->name)));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_priority_set_direct(svr->session, priority, NULL));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_CERTIFICATE, svr->cert));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_PSK, svr->pskcred_c));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_credentials_set(svr->session, GNUTLS_CRD_ANON, svr->anoncred_c));

        gnutls_dh_set_prime_bits(svr->session, 512);
        gnutls_transport_set_ptr(svr->session, (gnutls_transport_ptr_t)svr->fd);
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        ret = gnutls_handshake(svr->session);
        DBG("calling gnutls_handshake(): returned with '%s'", gnutls_strerror_name(ret));
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));
        if (!ret)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (gnutls_record_get_direction(svr->session))
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
             else
               ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
          }
      default:
        break;
     }

   if (!svr->verify)
     /* not verifying certificates, so we're done! */
     return ECORE_CON_SSL_ERROR_NONE;
   ret = 0;
   /* use CRL/CA lists to verify */
   SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_certificate_verify_peers2(svr->session, &iter));
   if (iter & GNUTLS_CERT_INVALID)
     ERR("The certificate is not trusted.");
   else if (iter & GNUTLS_CERT_SIGNER_NOT_FOUND)
     ERR ("The certificate hasn't got a known issuer.");
   else if (iter & GNUTLS_CERT_REVOKED)
     ERR ("The certificate has been revoked.");
   else if (iter & GNUTLS_CERT_EXPIRED)
     ERR ("The certificate has expired");
   else if (iter & GNUTLS_CERT_NOT_ACTIVATED)
     ERR ("The certificate is not yet activated");

   if (iter)
     goto error;

   if (gnutls_certificate_type_get(svr->session) != GNUTLS_CRT_X509)
     {
        ERR("Warning: PGP certificates are not yet supported!");
        goto error;
     }


   SSL_ERROR_CHECK_GOTO_ERROR(!(cert_list = gnutls_certificate_get_peers(svr->session, &cert_list_size)));
   SSL_ERROR_CHECK_GOTO_ERROR(!cert_list_size);

   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_init(&cert));
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER));

   SSL_ERROR_CHECK_GOTO_ERROR(!gnutls_x509_crt_check_hostname(cert, svr->name));
   gnutls_x509_crt_deinit(cert);
   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(ret);
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(svr->session)));
   if (svr->ssl_state != ECORE_CON_SSL_STATE_DONE)
     {
       ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(svr->session)));
       ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(svr->session)));
     }
   if (cert)
     gnutls_x509_crt_deinit(cert);
   _ecore_con_ssl_server_shutdown_gnutls(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_gnutls(Ecore_Con_Server *svr, const char *ca_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_trust_file(svr->cert, ca_file,
                                              GNUTLS_X509_FMT_PEM) < 1);

   return EINA_TRUE;
error:
   ERR("Could not load CA file!");
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_gnutls(Ecore_Con_Server *svr, const char *crl_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_crl_file(svr->cert, crl_file,
                                            GNUTLS_X509_FMT_PEM) < 1);

   return EINA_TRUE;
error:
   ERR("Could not load CRL file!");
   return EINA_FALSE;
}


static Eina_Bool
_ecore_con_ssl_server_privkey_add_gnutls(Ecore_Con_Server *svr, const char *key_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(gnutls_certificate_set_x509_key_file(svr->cert, svr->cert_file, key_file,
                                            GNUTLS_X509_FMT_PEM));

   return EINA_TRUE;
error:
   ERR("Could not load certificate/key file!");
   return EINA_FALSE;
}


static Eina_Bool
_ecore_con_ssl_server_cert_add_gnutls(Ecore_Con_Server *svr, const char *cert_file)
{
   if (!(svr->cert_file = strdup(cert_file)))
     return EINA_FALSE;

   return EINA_TRUE;
}


static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_gnutls(Ecore_Con_Server *svr)
{
   if (svr->session)
     {
        gnutls_bye(svr->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(svr->session);
     }

   if (svr->cert_file)
     free(svr->cert_file);
   svr->cert_file = NULL;
   if (svr->cert)
     gnutls_certificate_free_credentials(svr->cert);
   svr->cert = NULL;

   if ((svr->type & ECORE_CON_SSL) && svr->created)
     {
        if (svr->dh_params)
          {
             gnutls_dh_params_deinit(svr->dh_params);
             svr->dh_params = NULL;
          }
        if (svr->anoncred_s)
          gnutls_anon_free_server_credentials(svr->anoncred_s);
        if (svr->pskcred_s)
          gnutls_psk_free_server_credentials(svr->pskcred_s);

        svr->anoncred_s = NULL;
        svr->pskcred_s = NULL;
     }
   else if (svr->type & ECORE_CON_SSL)
     {
        if (svr->anoncred_c)
          gnutls_anon_free_client_credentials(svr->anoncred_c);
        if (svr->pskcred_c)
          gnutls_psk_free_client_credentials(svr->pskcred_c);

        svr->anoncred_c = NULL;
        svr->pskcred_c = NULL;
     }

   svr->session = NULL;

   return ECORE_CON_SSL_ERROR_NONE;
}

/* this is a stub function, the handshake is done in _init_gnutls */
static Ecore_Con_State
_ecore_con_ssl_server_try_gnutls(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_CONNECTED;
}

static int
_ecore_con_ssl_server_read_gnutls(Ecore_Con_Server *svr, unsigned char *buf,
                                  int size)
{
   int num;

   if (svr->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        DBG("Continuing gnutls handshake");
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
        return -1;
     }
     
   num = gnutls_record_recv(svr->session, buf, size);
   if (num > 0)
      return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;

        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
     }
   else if ((num == GNUTLS_E_AGAIN) || (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

static int
_ecore_con_ssl_server_write_gnutls(Ecore_Con_Server *svr, unsigned char *buf,
                                   int size)
{
   int num;


   if (svr->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        DBG("Continuing gnutls handshake");
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
        return -1;
     }

   num = gnutls_record_send(svr->session, buf, size);
   if (num > 0)
      return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
         WRN("Rehandshake request ignored");
         return 0;
/* this is only partly functional I think? */
        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_server_init_gnutls(svr))
          return 0;
     }
   else if ((num == GNUTLS_E_AGAIN) || (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}


static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_gnutls(Ecore_Con_Client *cl)
{
#ifdef USE_GNUTLS2_10
   const char *priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:+VERS-TLS1.2:+VERS-TLS1.1:+VERS-TLS1.0:+VERS-SSL3.0";
#else
   const char *priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:+VERS-TLS1.1:+VERS-TLS1.0:+VERS-SSL3.0";
#endif
   int ret = 0;

   switch (cl->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;
      case ECORE_CON_SSL_STATE_INIT:
        if (cl->host_server->type & ECORE_CON_USE_SSL2) /* not supported because of security issues */
          return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

        switch (cl->host_server->type & ECORE_CON_SSL)
          {
           case ECORE_CON_USE_SSL3:
           case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
              priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:!VERS-TLS1.0:!VERS-TLS1.1";
              break;

           case ECORE_CON_USE_TLS:
           case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
              priority = "NONE:%VERIFY_ALLOW_X509_V1_CA_CRT:+RSA:+DHE-RSA:+DHE-DSS:+ANON-DH:+COMP-DEFLATE:+COMP-NULL:+CTYPE-X509:+SHA1:+SHA256:+SHA384:+SHA512:+AES-256-CBC:+AES-128-CBC:+3DES-CBC:!VERS-SSL3.0";
              break;

           default:
              return ECORE_CON_SSL_ERROR_NONE;
          }

        _client_connected++;

        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_init(&cl->session, GNUTLS_SERVER));
#ifdef USE_GNUTLS_2_10
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_key_generate(&cl->session_ticket));
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_session_ticket_enable_server(cl->session, &cl->session_ticket));
#endif
        SSL_ERROR_CHECK_GOTO_ERROR(ret = gnutls_priority_set_direct(cl->session, priority, NULL));

        gnutls_certificate_server_set_request(cl->session, GNUTLS_CERT_REQUEST);

        gnutls_dh_set_prime_bits(cl->session, 2048);
        gnutls_transport_set_ptr(cl->session, (gnutls_transport_ptr_t)cl->fd);
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        DBG("calling gnutls_handshake()");
        ret = gnutls_handshake(cl->session);
        SSL_ERROR_CHECK_GOTO_ERROR(gnutls_error_is_fatal(ret));

        if (!ret)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
        else
          {
             if (gnutls_record_get_direction(cl->session))
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
             else
               ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
          }
      default:
        break;
     }

   /* TODO: add cert verification support */
   return ECORE_CON_SSL_ERROR_NONE;

error:
   _gnutls_print_errors(ret);
   if ((ret == GNUTLS_E_WARNING_ALERT_RECEIVED) || (ret == GNUTLS_E_FATAL_ALERT_RECEIVED))
     ERR("Also received alert: %s", gnutls_alert_get_name(gnutls_alert_get(cl->session)));
   ERR("last out: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_out(cl->session)));
   ERR("last in: %s", SSL_GNUTLS_PRINT_HANDSHAKE_STATUS(gnutls_handshake_get_last_in(cl->session)));
   _ecore_con_ssl_client_shutdown_gnutls(cl);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_gnutls(Ecore_Con_Client *cl)
{
   if (cl->session)
     {
        gnutls_bye(cl->session, GNUTLS_SHUT_RDWR);
        gnutls_deinit(cl->session);
        gnutls_free(cl->session_ticket.data);
        cl->session_ticket.data = NULL;
     }

   cl->session = NULL;

   return ECORE_CON_SSL_ERROR_NONE;
}


static int
_ecore_con_ssl_client_read_gnutls(Ecore_Con_Client *cl, unsigned char *buf,
                                  int size)
{
   int num;

   if (cl->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
        return -1;
     }

   num = gnutls_record_recv(cl->session, buf, size);
   if (num > 0)
      return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
       WRN("Rehandshake request ignored");
       return 0;
     }
   else if ((num == GNUTLS_E_AGAIN) || (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

static int
_ecore_con_ssl_client_write_gnutls(Ecore_Con_Client *cl, unsigned char *buf,
                                   int size)
{
   int num;


   if (cl->ssl_state == ECORE_CON_SSL_STATE_HANDSHAKING)
     {
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
        return -1;
     }

   num = gnutls_record_send(cl->session, buf, size);
   if (num > 0)
      return num;

   if (num == GNUTLS_E_REHANDSHAKE)
     {
        WRN("Rehandshake request ignored");
        return 0;
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
        if (!_ecore_con_ssl_client_init_gnutls(cl))
          return 0;
     }
   else if ((num == GNUTLS_E_AGAIN) || (num == GNUTLS_E_INTERRUPTED))
      return 0;

   return -1;
}

#elif USE_OPENSSL && !USE_GNUTLS

/*
 * OpenSSL
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_openssl(void)
{
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();

   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_openssl(void)
{
   ERR_free_strings();
   EVP_cleanup();
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_openssl(Ecore_Con_Server *svr, int ssl_type)
{
   long options;
   int dh = 0;

   if (ssl_type & ECORE_CON_USE_SSL2)
     return ECORE_CON_SSL_ERROR_SSL2_NOT_SUPPORTED;

   switch (ssl_type)
     {
      case ECORE_CON_USE_SSL3:
      case ECORE_CON_USE_SSL3 | ECORE_CON_LOAD_CERT:
         if (!svr->created)
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv3_client_method())));
         else
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv3_server_method())));
         break;

      case ECORE_CON_USE_TLS:
      case ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT:
         if (!svr->created)
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(TLSv1_client_method())));
         else
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(TLSv1_server_method())));
         break;

      case ECORE_CON_USE_MIXED:
      case ECORE_CON_USE_MIXED | ECORE_CON_LOAD_CERT:
         if (!svr->created)
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv23_client_method())));
         else
           SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl_ctx = SSL_CTX_new(SSLv23_server_method())));
         options = SSL_CTX_get_options(svr->ssl_ctx);
         SSL_CTX_set_options(svr->ssl_ctx, options | SSL_OP_NO_SSLv2 | SSL_OP_SINGLE_DH_USE);
         break;

      default:
         break;
     }

   if ((!svr->use_cert) && svr->created)
     {
        DH *dh_params;
        SSL_ERROR_CHECK_GOTO_ERROR(!(dh_params = DH_new()));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_parameters_ex(dh_params, 1024, DH_GENERATOR_5, NULL));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_check(dh_params, &dh));
        SSL_ERROR_CHECK_GOTO_ERROR((dh & DH_CHECK_P_NOT_PRIME) || (dh & DH_CHECK_P_NOT_SAFE_PRIME));
        SSL_ERROR_CHECK_GOTO_ERROR(!DH_generate_key(dh_params));
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_tmp_dh(svr->ssl_ctx, dh_params));
        DH_free(dh_params);
        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:@STRENGTH"));
     }
   else if (!svr->use_cert)
     SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_set_cipher_list(svr->ssl_ctx, "aNULL:!eNULL:!LOW:!EXPORT:!ECDH:RSA:AES:!PSK:@STRENGTH"));

     return ECORE_CON_SSL_ERROR_NONE;

error:
   if (dh)
     {
        if (dh & DH_CHECK_P_NOT_PRIME)
          ERR("openssl error: dh_params could not generate a prime!");
        else
          ERR("openssl error: dh_params could not generate a safe prime!");
     }
   else
     _openssl_print_errors();
   _ecore_con_ssl_server_shutdown_openssl(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}




static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_openssl(Ecore_Con_Server *svr)
{
   int err, ret = -1;

   switch (svr->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;
      case ECORE_CON_SSL_STATE_INIT:
        SSL_ERROR_CHECK_GOTO_ERROR(!(svr->ssl = SSL_new(svr->ssl_ctx)));

        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(svr->ssl, svr->fd));
        SSL_set_connect_state(svr->ssl);
        svr->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:        
        ret = SSL_do_handshake(svr->ssl);
        err = SSL_get_error(svr->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL));

        if (ret == 1)
          {
             svr->handshaking = EINA_FALSE;
             svr->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   _openssl_print_errors();
   _ecore_con_ssl_server_shutdown_openssl(svr);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_openssl(Ecore_Con_Server *svr, const char *ca_file)
{
   SSL_ERROR_CHECK_GOTO_ERROR(!SSL_CTX_load_verify_locations(svr->ssl_ctx, ca_file, NULL));
   return EINA_TRUE;

error:
   _openssl_print_errors();
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_openssl(Ecore_Con_Server *svr, const char *crl_file)
{
   FILE *fp = NULL;
   X509_CRL *crl = NULL;

   if (!(fp = fopen(crl_file, "r")))
      goto error;
#warning IMPLEMENT FIXME!
   SSL_ERROR_CHECK_GOTO_ERROR(!(crl = PEM_read_X509_CRL(fp, NULL, NULL, NULL)));

   fclose(fp);

   return EINA_TRUE;

error:
   if (fp)
      fclose(fp);
   _openssl_print_errors();
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_privkey_add_openssl(Ecore_Con_Server *svr, const char *key_file)
{
   FILE *fp = NULL;
   EVP_PKEY *privkey = NULL;

   if (!(fp = fopen(key_file, "r")))
      goto error;

   SSL_ERROR_CHECK_GOTO_ERROR(!(privkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL)));

   fclose(fp);
   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_PrivateKey(svr->ssl_ctx, privkey) < 1);
   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_check_private_key(svr->ssl_ctx) < 1);

   return EINA_TRUE;

error:
   if (fp)
      fclose(fp);
   _openssl_print_errors();
   return EINA_FALSE;
}


static Eina_Bool
_ecore_con_ssl_server_cert_add_openssl(Ecore_Con_Server *svr, const char *cert_file)
{
   FILE *fp = NULL;
   X509 *cert = NULL;

   if (!(fp = fopen(cert_file, "r")))
      goto error;

   SSL_ERROR_CHECK_GOTO_ERROR(!(cert = PEM_read_X509(fp, NULL, NULL, NULL)));

   fclose(fp);

   SSL_ERROR_CHECK_GOTO_ERROR(SSL_CTX_use_certificate(svr->ssl_ctx, cert) < 1);

   return EINA_TRUE;

error:
   if (fp)
      fclose(fp);
   _openssl_print_errors();
   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_openssl(Ecore_Con_Server *svr)
{
   if (svr->ssl)
     {
        if (!SSL_shutdown(svr->ssl))
           SSL_shutdown(svr->ssl);

        SSL_free(svr->ssl);
     }

   if (svr->ssl_ctx)
      SSL_CTX_free(svr->ssl_ctx);

   svr->ssl = NULL;
   svr->ssl_ctx = NULL;
   svr->ssl_err = SSL_ERROR_NONE;

   return ECORE_CON_SSL_ERROR_NONE;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_openssl(Ecore_Con_Server *svr)
{
   int res, flag = 0;

   if ((res = SSL_connect(svr->ssl)) == 1)
      return ECORE_CON_CONNECTED;

   svr->ssl_err = SSL_get_error(svr->ssl, res);

   switch (svr->ssl_err)
     {
      case SSL_ERROR_NONE:
         return ECORE_CON_CONNECTED;

      case SSL_ERROR_WANT_READ:
         flag = ECORE_FD_READ;
         break;

      case SSL_ERROR_WANT_WRITE:
         flag = ECORE_FD_WRITE;
         break;

      default:
         return ECORE_CON_DISCONNECTED;
     }

   if (svr->fd_handler && flag)
           ecore_main_fd_handler_active_set(svr->fd_handler,
                                       flag);

   return ECORE_CON_INPROGRESS;
}

static int
_ecore_con_ssl_server_read_openssl(Ecore_Con_Server *svr, unsigned char *buf,
                                   int size)
{
   int num;

   num = SSL_read(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err == SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
     }

   if ((svr->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (svr->ssl_err == SSL_ERROR_SYSCALL) ||
       (svr->ssl_err == SSL_ERROR_SSL))
      return -1;

   if (num < 0)
      return 0;

   return num;
}

static int
_ecore_con_ssl_server_write_openssl(Ecore_Con_Server *svr, unsigned char *buf,
                                    int size)
{
   int num;

   num = SSL_write(svr->ssl, buf, size);
   svr->ssl_err = SSL_get_error(svr->ssl, num);

   if (svr->fd_handler)
     {
        if (svr->ssl && svr->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(svr->fd_handler,
                                            ECORE_FD_READ);
        else if (svr->ssl && svr->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              svr->fd_handler,
              ECORE_FD_WRITE);
     }

   if ((svr->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (svr->ssl_err == SSL_ERROR_SYSCALL) ||
       (svr->ssl_err == SSL_ERROR_SSL))
      return -1;

   if (num < 0)
      return 0;

   return num;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_openssl(Ecore_Con_Client *cl)
{
   int err, ret = -1;
   switch (cl->ssl_state)
     {
      case ECORE_CON_SSL_STATE_DONE:
        return ECORE_CON_SSL_ERROR_NONE;
      case ECORE_CON_SSL_STATE_INIT:
        SSL_ERROR_CHECK_GOTO_ERROR(!(cl->ssl = SSL_new(cl->host_server->ssl_ctx)));

        SSL_ERROR_CHECK_GOTO_ERROR(!SSL_set_fd(cl->ssl, cl->fd));
        SSL_set_accept_state(cl->ssl);
        cl->ssl_state = ECORE_CON_SSL_STATE_HANDSHAKING;
      case ECORE_CON_SSL_STATE_HANDSHAKING:
        ret = SSL_do_handshake(cl->ssl);
        err = SSL_get_error(cl->ssl, ret);
        SSL_ERROR_CHECK_GOTO_ERROR((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_SSL));
        if (ret == 1)
          {
             cl->handshaking = EINA_FALSE;
             cl->ssl_state = ECORE_CON_SSL_STATE_DONE;
          }
      default:
        break;
     }

   return ECORE_CON_SSL_ERROR_NONE;

error:
   _openssl_print_errors();
   _ecore_con_ssl_client_shutdown_openssl(cl);
   return ECORE_CON_SSL_ERROR_SERVER_INIT_FAILED;
}


static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_openssl(Ecore_Con_Client *cl)
{
   if (cl->ssl)
     {
        if (!SSL_shutdown(cl->ssl))
           SSL_shutdown(cl->ssl);

        SSL_free(cl->ssl);
     }

   cl->ssl = NULL;
   cl->ssl_err = SSL_ERROR_NONE;

   return ECORE_CON_SSL_ERROR_NONE;
}

static int
_ecore_con_ssl_client_read_openssl(Ecore_Con_Client *cl, unsigned char *buf,
                                   int size)
{
   int num;

   num = SSL_read(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(cl->fd_handler,
                                            ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              cl->fd_handler,
              ECORE_FD_WRITE);
     }

   if ((cl->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (cl->ssl_err == SSL_ERROR_SYSCALL) ||
       (cl->ssl_err == SSL_ERROR_SSL))
      return -1;

   if (num < 0)
      return 0;

   return num;
}

static int
_ecore_con_ssl_client_write_openssl(Ecore_Con_Client *cl, unsigned char *buf,
                                    int size)
{
   int num;

   num = SSL_write(cl->ssl, buf, size);
   cl->ssl_err = SSL_get_error(cl->ssl, num);

   if (cl->fd_handler)
     {
        if (cl->ssl && cl->ssl_err ==
            SSL_ERROR_WANT_READ)
           ecore_main_fd_handler_active_set(cl->fd_handler,
                                            ECORE_FD_READ);
        else if (cl->ssl && cl->ssl_err ==
                 SSL_ERROR_WANT_WRITE)
           ecore_main_fd_handler_active_set(
              cl->fd_handler,
              ECORE_FD_WRITE);
     }

   if ((cl->ssl_err == SSL_ERROR_ZERO_RETURN) ||
       (cl->ssl_err == SSL_ERROR_SYSCALL) ||
       (cl->ssl_err == SSL_ERROR_SSL))
      return -1;

   if (num < 0)
      return 0;

   return num;
}

#else

/*
 * No Ssl
 */

static Ecore_Con_Ssl_Error
_ecore_con_ssl_init_none(void)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_shutdown_none(void)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_prepare_none(Ecore_Con_Server *svr __UNUSED__, int ssl_type __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NONE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_init_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Eina_Bool
_ecore_con_ssl_server_cafile_add_none(Ecore_Con_Server *svr __UNUSED__, const char *ca_file __UNUSED__)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_cert_add_none(Ecore_Con_Server *svr __UNUSED__, const char *cert_file __UNUSED__)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_privkey_add_none(Ecore_Con_Server *svr __UNUSED__, const char *key_file __UNUSED__)
{
   return EINA_FALSE;
}

static Eina_Bool
_ecore_con_ssl_server_crl_add_none(Ecore_Con_Server *svr __UNUSED__, const char *crl_file __UNUSED__)
{
   return EINA_FALSE;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_server_shutdown_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

/* Tries to connect an Ecore_Con_Server to an SSL host.
 * Returns 1 on success, -1 on fatal errors and 0 if the caller
 * should try again later.
 */
static Ecore_Con_State
_ecore_con_ssl_server_try_none(Ecore_Con_Server *svr __UNUSED__)
{
   return ECORE_CON_DISCONNECTED;
}

static int
_ecore_con_ssl_server_read_none(Ecore_Con_Server *svr __UNUSED__, unsigned char *buf __UNUSED__,
                                int size __UNUSED__)
{
   return -1;
}

static int
_ecore_con_ssl_server_write_none(Ecore_Con_Server *svr __UNUSED__, unsigned char *buf __UNUSED__,
                                 int size __UNUSED__)
{
   return -1;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_init_none(Ecore_Con_Client *cl __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static Ecore_Con_Ssl_Error
_ecore_con_ssl_client_shutdown_none(Ecore_Con_Client *cl __UNUSED__)
{
   return ECORE_CON_SSL_ERROR_NOT_SUPPORTED;
}

static int
_ecore_con_ssl_client_read_none(Ecore_Con_Client *cl __UNUSED__, unsigned char *buf __UNUSED__,
                                int size __UNUSED__)
{
   return -1;
}

static int
_ecore_con_ssl_client_write_none(Ecore_Con_Client *cl __UNUSED__, unsigned char *buf __UNUSED__,
                                 int size __UNUSED__)
{
   return -1;
}

#endif
