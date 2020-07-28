#ifndef MAILER_H
#define MAILER_H

#include <unistd.h>      // for close() function
#include <openssl/ssl.h> // for SSL_CTX and SSL

#include <ctt.h>         // for ctt_oline_reader

typedef enum mlr_status
{
   MLR_SUCCESS = 0,
   MLR_FAILURE,
   MLR_ADDRESS_BAD_FLAGS,
   MLR_ADDRESS_TEMPORARY_FAILURE,
   MLR_ADDRESS_PERMANENT_FAILURE,
   MLR_HOST_REJECTS_REQUEST,
   MLR_SYSTEM_ERROR,              // system error: look at errno
   MLR_URL_UNRESOLVED,
   MLR_NO_SUITABLE_SOCKET,
   MLR_CONNECT_FAILED,
   MLR_CONNECT_TIMED_OUT,
   MLR_SSL_NO_METHOD,
   MLR_SSL_NO_CONTEXT,
   MLR_SSL_NO_SSL,
   MLR_SSL_CONNECTION_FAILED,
   MLR_SSL_CONNECTION_SHUTDOWN
} mlrStatus;

typedef enum {
   mlr_ci_start_tls =  0,               // STARTTLS
   mlr_ci_7_bit_mime,                   // 7BITMIME
   mlr_ci_8_bit_mime,                   // 8BITMIME
   mlr_ci_authenticated_turn,           // ATRN
   mlr_ci_binary_mime,                  // BINARYMIME
   mlr_ci_chunking,                     // CHUNKING
   mlr_ci_deliver_by,                   // DELIVERYBY
   mlr_ci_delivery_status_notification, // DSN
   mlr_ci_enhanced_status_codes,        // ENHANCEDSTATUSCODES
   mlr_ci_expand,                       // EXPN
   mlr_ci_extended_turn,                // ETRN
   mlr_ci_help,                         // HELP
   mlr_ci_one_message,                  // ONEX
   mlr_ci_pipelining,                   // PIPELINING
   mlr_ci_require_tls,                  // REQUIRETLS
   mlr_ci_smtp_utf8,                    // SMTPUTF8
   mlr_ci_verbose,                      // VERB

   mlr_ci_last                          // limit value for iterating enums
} mlrCapsIndex;

typedef struct mlr_connection_request
{
   const char *host_url;
   int         host_port;
   const char *user;
   const char *password;
   int ssl;
   int smtp;
} mlrConnReq;

typedef struct mlr_ssl_handle
{
   SSL_CTX *context;
   SSL     *ssl;
} mlrSSL;

/* Bit-mapped SMTP capability flags along with message size limit. */
typedef struct mlr_smtp_caps
{
   int      size;
   uint64_t flags;
} mlrSmtpCaps;

// Foreward declaration to provide for function pointer types:
typedef struct mlr_connection mlrConn;

typedef int (*mlrSender)(mlrConn *conn, const char *data, int data_len);
typedef int (*mlrReader)(mlrConn *conn, char *buffer, int buffer_len);

typedef struct mlr_connection
{
   int       socket;
   mlrSSL    handle;
   mlrSender sender;
   mlrReader reader;
} mlrConn;

int mlr_connection_send(mlrConn *conn, const char *data, int data_len);
int mlr_connection_read(mlrConn *conn, char *buffer, int buffer_len);

int mlr_connection_send_string(mlrConn *conn, const char *str);
int mlr_connection_send_concat_line(mlrConn *conn, ...);

/** Developer can provide a function pointer for printing
 *  progress or failure messages while progressing through
 *  this process.
 */

/** function pointer type for use with mlr_set_verbose_reporting() */
typedef int (*verbose_printer_t)(const char *format, ...);
/** Function by which a verbose printer is installed. */
verbose_printer_t mlr_set_verbose_reporting(verbose_printer_t printer);

const char *mlr_get_status_string(mlrStatus status);

void mlr_init_connection(mlrConn *conn);
mlrStatus mlr_get_connected_socket(int *socket, const char *host_url, int host_port);

mlrStatus mlr_open_connection(mlrConn *connection, const mlrConnReq *req);

int mlr_connection_line_reader(void *source, char *buffer, int bytes_to_read);


int mlr_get_smtp_line(LRScope *scope,
                      int *code,
                      const char **line,
                      const char **line_end);

int mlr_request_smtp_caps(mlrSmtpCaps *caps, LRScope *scope, mlrConn *comm, const char *url);
const char *mlr_cap_name_from_index(int index);
int mlr_seek_cap_index(const char *name, int name_len);
int mlr_smtp_cap_get(mlrSmtpCaps *caps, int index);
int mlr_smtp_cap_set(mlrSmtpCaps *caps, int index);

void mlr_close_connection(mlrConn *connection);

typedef int (*mlrSender)(mlrConn *conn, const char *data, int data_len);
typedef int (*mlrRecver)(mlrConn *conn, const char *buffer, int buffer_len);

typedef struct mlr_intercom
{
   mlrSender sender;
   mlrRecver recver;
} mlrXCom;





#endif
