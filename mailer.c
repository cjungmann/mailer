#include "mailer.h"
#include "invisible.h"

#include <string.h>    // for strlen()

verbose_printer_t verbose_printer = NULL;

EXPORT verbose_printer_t mlr_set_verbose_reporting(verbose_printer_t printer)
{
   verbose_printer_t old = verbose_printer;
   verbose_printer = printer;
   return old;
}

EXPORT void mlr_init_connection(mlrConn *conn)
{
   conn->socket         = -1;
   conn->handle.context = NULL;
   conn->handle.ssl     = NULL;
}

EXPORT mlrStatus mlr_open_connection(mlrConn *connection,
                                     const char *host_url,
                                     int host_port,
                                     const char *user,
                                     const char *password,
                                     int ssl)
{
   int socket;

   mlrStatus status = mlr_get_connected_socket(&socket, host_url, host_port);
   if (status)
   {
      if (verbose_printer)
         (*verbose_printer)("FAIL: getting connected socket, %s,\n", mlr_get_status_string(status));

      return status;
   }
   else
   {
      status = open_ssl_handle(&connection->handle, socket);

      if (status)
      {
         if (verbose_printer)
            (*verbose_printer)("FAIL: getting SSL handle, %s,\n", mlr_get_status_string(status));

         close(socket);
         return status;
      }
      else
      {
         if (verbose_printer)
            (*verbose_printer)("SUCCESS: got SSL handle.\n");
      }

      close(socket);
      return MLR_SUCCESS;
   }
}

EXPORT void mlr_close_xcom(mlrXCom *xcom)
{
}
