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

typedef struct mlr_ssl_handle
{
   SSL_CTX *context;
   SSL     *ssl;
} mlrSSL;

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

mlrStatus mlr_open_connection(mlrConn *connection,
                              const char *host_url,
                              int host_port,
                              const char *user,
                              const char *password,
                              int ssl,
                              int smtp);

int mlr_get_smtp_line(LRScope *scope,
                      int *code,
                      int *final_line,
                      const char **line,
                      const char **line_end);

void mlr_close_connection(mlrConn *connection);

typedef int (*mlrSender)(mlrConn *conn, const char *data, int data_len);
typedef int (*mlrRecver)(mlrConn *conn, const char *buffer, int buffer_len);

typedef struct mlr_intercom
{
   mlrSender sender;
   mlrRecver recver;
} mlrXCom;





#endif
