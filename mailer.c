#include "mailer.h"
#include "invisible.h"

#include <string.h>    // for strlen()
#include <ctype.h>     // for isdigit()

#include <stdarg.h>    // for variable number of arguments


EXPORT int mlr_connection_send(mlrConn *conn, const char *data, int data_len)
{
   return (*conn->sender)(conn, data, data_len);
}

EXPORT int mlr_connection_send_string(mlrConn *conn, const char *str)
{
   return (*conn->sender)(conn, str, strlen(str));
}

EXPORT int mlr_connection_send_concat_line(mlrConn *conn, ...)
{
   int count = 0;
   va_list ap;
   const char *str;
   va_start(ap, conn);

   while ((str = va_arg(ap, const char *)))
      count += mlr_connection_send_string(conn, str);

   count += mlr_connection_send(conn, "\n", 1);

   va_end(ap);

   return count;
}

EXPORT int mlr_connection_read(mlrConn *conn, char *buffer, int buffer_len)
{
   return (*conn->reader)(conn, buffer, buffer_len);
}


/* Global variable that both flags and services verbose messaging. */
verbose_printer_t verbose_printer = NULL;

/* Convenience function for constructing a verbose output line. */
int verbose_print_concat_line(verbose_printer_t printer, ...)
{
   int count = 0;
   va_list ap;
   const char *str;
   va_start(ap, printer);

   while ((str = va_arg(ap, const char *)))
      count += (*printer)(str);

   count += (*printer)("\n");

   va_end(ap);

   return count;
}

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

EXPORT mlrStatus mlr_open_connection(mlrConn *connection, const mlrConnReq *req)
{
   int socket;

   mlrStatus status = mlr_get_connected_socket(&socket, req->host_url, req->host_port);
   if (status)
   {
      if (verbose_printer)
         (*verbose_printer)("FAIL: getting connected socket, %s,\n", mlr_get_status_string(status));

      return status;
   }
   else
   {
      connection->socket = socket;
      connection->sender = socket_writer;
      connection->reader = socket_reader;

      if (req->ssl)
      {
         if (req->smtp)
         {
            mlr_connection_send_concat_line(connection, "EHLO ", req->host_url, NULL);
         }

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
      }

      return status;
   }
}

int confirm_smtp_line_form(const char *line, const char *line_end)
{
   const char *ptr = line;
   const char*dash_or_space = &line[3];

   while (ptr < dash_or_space)
   {
      if (!isdigit(*ptr))
         return 0;
      ++ptr;
   }

   return *dash_or_space == ' ' || *dash_or_space == '-';
}

/* For function pointer argument of line_reader in ctt library. */
EXPORT int mlr_connection_line_reader(void *source, char *buffer, int bytes_to_read)
{
   mlrConn *conn = (mlrConn*)source;
   return mlr_connection_read(conn, buffer, bytes_to_read);
}

EXPORT int mlr_get_smtp_line(LRScope *scope,
                             int *code,
                             const char **line,
                             const char **line_end)
{
   const char *tline;
   const char *tline_end;

   if (!scope->eof)
   {
      if (ctt_get_line(scope, &tline, &tline_end))
      {
         if (confirm_smtp_line_form(tline, tline_end))
         {
            *code = atoi(tline);
            *line = &tline[4];
            *line_end = tline_end;

            if (tline[3] == ' ')
               scope->eof = 1;

            return 1;
         }
      }
   }

   return 0;
}

EXPORT void mlr_close_connection(mlrConn *connection)
{
   if (connection->handle.ssl)
   {
      SSL_free(connection->handle.ssl);
      connection->handle.ssl = NULL;
   }

   if (connection->handle.context)
   {
      SSL_CTX_free(connection->handle.context);
      connection->handle.context = NULL;
   }

   if (connection->socket >= 0)
   {
      close(connection->socket);
      connection->socket = -1;
   }
}

