#ifndef MAILER_H
#define MAILER_H

#include <unistd.h>      // for close() function
#include <openssl/ssl.h> // for SSL_CTX and SSL

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

typedef struct mlr_connection
{
   int    socket;
   mlrSSL handle;
} mlrConn;

typedef int (*verbose_printer_t)(const char *format, ...);

verbose_printer_t mlr_set_verbose_reporting(verbose_printer_t printer);
const char *mlr_get_status_string(mlrStatus status);


// Forward declaration
typedef struct mlr_intercom mlrXCom;

void mlr_init_connection(mlrConn *conn);

mlrStatus mlr_get_connected_socket(int *socket, const char *host_url, int host_port);

mlrStatus mlr_open_connection(mlrConn *connection,
                              const char *host_url,
                              int host_port,
                              const char *user,
                              const char *password,
                              int ssl);

void mlr_close_xcom(mlrXCom *xcom);

typedef int (*mlrSender)(mlrXCom *com, const char *data, int data_len);
typedef int (*mlrRecver)(mlrXCom *com, const char *buffer, int buffer_len);

typedef struct mlr_intercom
{
   mlrSender sender;
   mlrRecver recver;
} mlrXCom;





#endif
